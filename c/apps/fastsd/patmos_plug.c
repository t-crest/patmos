/// Interface between newlibc and ElmChan FatFS
/// Loosely based on RIOT-OS/RIOT
/// Copyright (c) 2017 HAW-Hamburg
/// LGPL v2.1

#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>

#include "sdc_debug.h"
#include "ff.h"

#define INIT_GUARD                \
	do                              \
	{                               \
		error_t init_res;             \
		init_res = patmosplug_init(); \
		if (init_res != 0)            \
		{                             \
			return init_res;            \
		}                             \
	} while (0);

typedef struct
{
	FIL file;
	char fname[FF_LFN_BUF + 1];
	int is_open;
} file_desc_t;

#define FD_OFFSET (4) // STDIN, OUT, ERR
#define FD_BUFFER_SIZE (128)
file_desc_t file_desc_buffer[FD_BUFFER_SIZE];

static int is_init = 0;
static FATFS fs;

static error_t patmosplug_init();
static error_t fatfs_result_to_posix(FRESULT result);

int patmosplug_open(const char *name, int flags, int mode)
{
	BYTE fatfs_flags = 0;
	FIL fd;
	FRESULT open_result;
	INIT_GUARD;

	if ((flags & O_ACCMODE) == O_RDONLY)
	{
		fatfs_flags |= FA_READ;
	}
	if ((flags & O_ACCMODE) == O_WRONLY)
	{
		fatfs_flags |= FA_WRITE;
	}
	if ((flags & O_ACCMODE) == O_RDWR)
	{
		fatfs_flags |= (FA_READ | FA_WRITE);
	}
	if ((flags & O_APPEND) == O_APPEND)
	{
		fatfs_flags |= FA_OPEN_APPEND;
	}
	if ((flags & O_TRUNC) == O_TRUNC)
	{
		fatfs_flags |= FA_CREATE_ALWAYS;
	}
	if ((flags & O_CREAT) == O_CREAT)
	{
		if ((flags & O_EXCL) == O_EXCL)
		{
			fatfs_flags |= FA_CREATE_NEW;
		}
		else
		{
			fatfs_flags |= FA_OPEN_ALWAYS;
		}
	}
	else
	{
		fatfs_flags |= FA_OPEN_EXISTING;
	}

	for (int i = 0; i < FD_BUFFER_SIZE; i++)
	{
		if (!file_desc_buffer[i].is_open)
		{
			open_result = f_open(&fd, name, fatfs_flags);
			if (open_result == FR_OK)
			{
				DEBUG_PRINT("Opening file '%s' with flags = '%x', mode = '%x' succeded", name, flags, mode);
				file_desc_buffer[i].is_open = 1;
				file_desc_buffer[i].file = fd;
				strncpy(file_desc_buffer[i].fname, name, FF_LFN_BUF);

				return i + FD_OFFSET;
			}
			else
			{
				DEBUG_PRINT("Opening file '%s' with flags = '%x', mode = '%x' failed, error %d", name, flags, mode, open_result);

				return fatfs_result_to_posix(open_result);
			}
		}
	}

	// no free fileslot found
	return -ENOBUFS;
}

int patmosplug_close(int file)
{
	INIT_GUARD;

	if (file < FD_OFFSET)
	{
		return -EBADF;
	}

	file_desc_t fd = file_desc_buffer[file - FD_OFFSET];
	if (fd.is_open == 0)
	{
		return -EBADF;
	}

	FRESULT res = f_close(&(fd.file));
	if (res == FR_OK)
	{
		DEBUG_PRINT("Closing file '%s' succeded");
		file_desc_buffer[file - FD_OFFSET].is_open = 0;
	}
	else
	{
		DEBUG_PRINT("Closing file '%s' failed, error %d", fd.fname, res);
	}
	return fatfs_result_to_posix(res);
}

int patmosplug_read(int file, char *buf, int len)
{
	INIT_GUARD;
	UINT br;

	if (file < FD_OFFSET)
	{
		return -EBADF;
	}

	file_desc_t fd = file_desc_buffer[file - FD_OFFSET];

	if (fd.is_open == 0)
	{
		return -EBADF;
	}

	FRESULT res = f_read(&(fd.file), buf, len, &br);

	if (res != FR_OK)
	{
		return fatfs_err_to_errno(res);
	}

	return (ssize_t)br;
}

int patmosplug_write(int file, char *buf, int nbytes)
{
	INIT_GUARD;
	UINT bw;

	if (file < FD_OFFSET)
	{
		return -EBADF;
	}

	file_desc_t fd = file_desc_buffer[file - FD_OFFSET];

	if (fd.is_open == 0)
	{
		return -EBADF;
	}

	FRESULT res = f_write(&(fd.file), buf, nbytes, &bw);

	if (res != FR_OK)
	{
		return fatfs_err_to_errno(res);
	}

	return (ssize_t)bw;
}

static error_t patmosplug_init()
{
	if (is_init)
	{
		return 0;
	}

	error_t error;
	FRESULT result = f_mount(&fs, "", 1);

	error = fatfs_result_to_posix(result);

	if (result == FR_OK)
	{
		is_init = 1;
		DEBUG_PRINT("Initializing patmos_plug succeded");
	}
	else
	{
		is_init = 0;
		DEBUG_PRINT("Initializing patmos_plug failed FRESULT = %d (error_t = %d)",
								result,
								error);
	}

	return error;
}

/// Convert FatFs error code to newlib error code
static error_t fatfs_result_to_posix(FRESULT result)
{
	switch (result)
	{
	case FR_OK:
		return 0;
	case FR_DISK_ERR:
		return -EIO;
	case FR_INT_ERR:
		return -EIO;
	case FR_NOT_READY:
		return -ENODEV;
	case FR_NO_FILE:
		return -ENOENT;
	case FR_NO_PATH:
		return -ENOENT;
	case FR_INVALID_NAME:
		return -ENOENT;
	case FR_DENIED:
		return -EACCES;
	case FR_EXIST:
		return -EEXIST;
	case FR_INVALID_OBJECT:
		return -EBADF;
	case FR_WRITE_PROTECTED:
		return -EACCES;
	case FR_INVALID_DRIVE:
		return -ENXIO;
	case FR_NOT_ENABLED:
		return -ENODEV;
	case FR_NO_FILESYSTEM:
		return -ENODEV;
	case FR_MKFS_ABORTED:
		return -EINVAL;
	case FR_TIMEOUT:
		return -EBUSY;
	case FR_LOCKED:
		return -EACCES;
	case FR_NOT_ENOUGH_CORE:
		return -ENOMEM;
	case FR_TOO_MANY_OPEN_FILES:
		return -ENFILE;
	case FR_INVALID_PARAMETER:
		return -EINVAL;
	}

	return (int)result;
}
