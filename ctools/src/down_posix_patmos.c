#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

// maximum of 1MB
#define MAX_MEM	(1048576/4)

static int prog_cnt = 0;
static char prog_char[] = {'|','/','-','\\','|','/','-','\\'};
static char *exitString = "JVM exit!";

static int echo = 0;
static int usb = 0;
static int cont = 0;

int serial_open(char *fname) {
	struct termios opts;
	int fd;
	
	fd = open(fname, O_RDWR);
	if (fd < 0) {
		perror("Error opening serial port");
		exit(1);
	}
	
	// get current port options
	if (tcgetattr(fd, &opts)) {
		perror("Error getting serial options");
		exit(1);
	}
	
	// set baud rate
	cfsetispeed(&opts, B115200);
	cfsetospeed(&opts, B115200);
	
	// local port, enable receiver
	opts.c_cflag |= (CLOCAL | CREAD);
	
	// set to 8-bit mode
	opts.c_cflag &= ~CSIZE;
	opts.c_cflag |= CS8;
	
	// no parity, 1 stop bit
	opts.c_cflag &= ~(PARENB | CSTOPB);
	
	// disable hardware flow control if available
#ifdef CNEW_RTSCTS
	opts.c_cflag &= ~CNEW_RTSCTS;
#endif

	// disable any processing; full raw mode
	opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	opts.c_oflag &= ~OPOST;
	
	// no software flow control either
	opts.c_iflag &= ~(IXON | IXOFF | IXANY);
	
	if (tcsetattr(fd, TCSANOW, &opts)) {
		perror("Error setting serial options");
		exit(1);
	}
	
	return fd;
}

void write32_check(int fd, unsigned char data[4]) {
	int j, t;
	ssize_t c;
	unsigned char send[4];// = { 0x40, 0x41};//[4];
	unsigned char echo[4];
	for (j=0; j<4; j++) {
		send[j] = data[j];

	}
	c = 0;
   	write (fd, send, 4);
	printf("ok");
	read(fd, echo, 4);
	printf("echo");
	for(j=0; j<4; j++)
	{	
		fprintf(stdout, "%x ", echo[j]);
	}
	printf("test");


}

int getFileSize(FILE *file)
{
	int lCurPos, lEndPos;
	lCurPos = ftell(file);
	fseek(file, 0, 2);
	lEndPos = ftell(file);
	fseek(file, lCurPos, 0);
	return lEndPos;
}


int main(/*int argc, char *argv[]*/)
{
	unsigned char c;
	int i, j;
	int64_t l;
	
	int fdSerial;
	int fdFile;

	int32_t *ram;
	int64_t len;

	uint8_t *byt_buf;
	uint8_t *buf;
	
	char *ptr;
	struct stat statbuf;
//*******************************************************//
	const char *filePath = "test_shift.bin";	
	unsigned char *fileBuf;			// Pointer to our buffered data
	unsigned char read_buf[4];
	//read_buf = (unsigned char*) malloc (4);
	FILE *file = NULL;		// File pointer
	fdSerial = serial_open("/dev/ttyUSB0");
	if ((file = fopen(filePath, "rb")) == NULL)
		printf("Could not open specified file");
	else
		printf("File opened successfully");

	// Get the size of the file in bytes
	int fileSize = getFileSize(file);
	fprintf(stdout, "\nfileSize %i", fileSize);
	fprintf(stdout, "\n:", " " );
	for(i = 0; i < fileSize/4; i++)
		{
			fread(&read_buf, 1, 4, file);
			write32_check(fdSerial, read_buf);
		}

//******************************************************//
	close(fdSerial);
	

	return 0;
}
