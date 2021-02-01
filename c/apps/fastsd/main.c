#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sdc_debug.h"
#include "ff.h"

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    DEBUG_PRINT("res = %d", res);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            DEBUG_PRINT("res = %d", res);
            if (res != FR_OK || fno.fname[0] == 0){
                //DEBUG_PRINT("res = %d", res);
                break;  /* Break on error or end of dir */
            } 
            DEBUG_PRINT("res = %d", res);
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                DEBUG_PRINT("res = %d", res);
                if (res != FR_OK) {
                    //DEBUG_PRINT("res = %d", res);
                    break;
                }
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

int main()
{
    FATFS fs;
    FRESULT res;
	UINT a;
	
	// mount FAT
    res = f_mount(&fs, "", 1);
	
	// create file
	FIL fil_write;
	res = f_open(&fil_write,"/demo.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
	if(res != FR_OK) {
		printf("Could not create file 'demo.txt'. f_open: %d\n", res);
		return res;
	}
	
	// write to file
	char buffer_wr[4096];
	for(int i = 0, ascii = '!'; i < 4096; ++i) {
		buffer_wr[i] = ascii;
		ascii++;
		if (ascii > '~') {
			ascii = '!';
		}
	}
	res = f_write(&fil_write, buffer_wr, sizeof(buffer_wr), &a);
	if(res != FR_OK) {
		printf("Could not write to file 'demo.txt'. f_write: %d\n", res);
		return res;
	}
	printf("%u characters written to 'demo.txt'.\n", a);
	
	// close file
	res = f_close(&fil_write);
	if(res != FR_OK) {
		printf("Could not close file 'demo.txt'. f_close: %d\n", res);
		return res;
	}
	
	// read file
    FIL fil_read;
    res = f_open(&fil_read,"/demo.txt", FA_READ);
	if(res != FR_OK) {
		printf("Could not open file 'demo.txt'. f_open: %d\n", res);
		return res;
	}
    char buffer_read[4096];
    res = f_read(&fil_read, buffer_read, sizeof(buffer_read), &a);
	if(res != FR_OK) {
		printf("Could not read file 'demo.txt'. f_read: %d\n", res);
		return res;
	}
	if (a > sizeof(buffer_read)) {
		buffer_read[4095] = '\0';
	} else {
		buffer_read[a] = '\0';
	}
	printf("First %u characters of file 'demo.txt' are: %s\n", a, buffer_read);
	
	// close file
	res = f_close(&fil_read);
	if(res != FR_OK) {
		printf("Could not close file 'demo.txt'. f_close: %d\n", res);
		return res;
	}

	// create dirs
	res = f_mkdir("/demo_dir");
	if(res != FR_OK) {
		printf("Could not create dir '/demo_dir'. f_mkdir: %d\n", res);
		return res;
	}
	res = f_mkdir("/demo_dir/sub_dir");
	if(res != FR_OK) {
		printf("Could not create dir '/demo_dir/sub_dir'. f_mkdir: %d\n", res);
		return res;
	}
	
	// create files
	FIL fil_demo_dir_f;
	res = f_open(&fil_demo_dir_f,"/demo_dir/demo_dir.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
	if(res != FR_OK) {
		printf("Could not create file '/demo_dir/demo_dir.txt'. f_open: %d\n", res);
		return res;
	}
	res = f_close(&fil_demo_dir_f);
	if(res != FR_OK) {
		printf("Could not close file '/demo_dir/demo_dir.txt'. f_close: %d\n", res);
		return res;
	}
	FIL fil_demo_sub_dir_f;
	res = f_open(&fil_demo_sub_dir_f,"/demo_dir/sub_dir/demo_sub_dir.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
	if(res != FR_OK) {
		printf("Could not create file '/demo_dir/sub_dir/demo_sub_dir.txt'. f_open: %d\n", res);
		return res;
	}
	res = f_close(&fil_demo_sub_dir_f);
	if(res != FR_OK) {
		printf("Could not close file '/demo_dir/sub_dir/demo_sub_dir.txt'. f_close: %d\n", res);
		return res;
	}
	
	// list files in demo_dir
	printf("Files in '/demo_dir'.\n");
    res = scan_files("/demo_dir");
	if(res != FR_OK) {
		printf("Could not list all files in  '/demo_dir'. scan_files: %d\n", res);
		return res;
	}
	
    // f_mount(0, "", 0); // unmount

    return res;
}
