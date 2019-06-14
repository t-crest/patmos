#ifndef XPLANE_H
#define XPLANE_H

#define FSIM_PORT 666
#define FSIM_PERIOD 2000 //us
#define FSIM_MSG_TIMEOUT 1000
#define FSIM_MSG_BUF_SIZE 100

typedef struct {
    unsigned char prologue[5];
	unsigned char index[1];
	unsigned char empty[3];
	unsigned char pitch[4];
	unsigned char roll[4];
	unsigned char headT[4];
	unsigned char headM[4];
	unsigned char blank[4*4];
} fsim_msg_t;

typedef union{
	unsigned char bytes[4];
	float val;
} fsim_data_t;

typedef struct{
	fsim_data_t pitch;
	fsim_data_t roll;
	fsim_data_t headT;
	fsim_data_t headM;
} acf_orientation_t;

typedef struct {
  int len;
  int rdId;
  int wrId;
  acf_orientation_t acf_orientation_buf[FSIM_MSG_BUF_SIZE];
} fsim_msg_circbuf_t;

#endif // !TTEPWM_H