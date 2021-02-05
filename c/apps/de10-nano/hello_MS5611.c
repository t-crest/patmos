#include <stdio.h>
#include <machine/patmos.h>
#include <stdint.h>
#include <machine/spm.h>

//I2C controller
#define I2C ( *( ( volatile _IODEV unsigned * )	PATMOS_IO_I2C ) )

// Barometer macros
#define MS5611_WRITE	0x000000EE	// Write mode
#define MS5611_READ		0x000000EF	// Read mode
#define MS5611_ADDRESS	0x00000077	// Default barometer addresss


volatile _SPM int *i2cctrl  = (volatile _SPM int *) PATMOS_IO_I2C+0x00;
volatile _SPM int *i2cstatus= (volatile _SPM int *) PATMOS_IO_I2C+0x04;
volatile _SPM int *i2caddr  = (volatile _SPM int *) PATMOS_IO_I2C+0x08;
volatile _SPM int *i2cdata  = (volatile _SPM int *) PATMOS_IO_I2C+0x0C;


int i2c_busy(){
	// returns 1 if busy, 0 if free
	if ((*i2cstatus & 0x00000001) != 0) return 1;
	else return 0;
}

int i2c_connected(){
	// Returns 1 if connected, 0 if not
	if ((*i2cstatus & 0x00000002) != 0) return 1;
	else return 0;
}

int i2c_getMode(){
	// returns 1 for read mode, 0 for write mode
	if ((*i2cstatus & 0x00000004) != 0) return 1;
	else return 0;
}

int i2c_lastACK(){
	// Returns 0 for ACK, 1 for NACK
	if ((*i2cctrl & 0x00000008) != 0) return 1;
	else return 0;
}

void i2c_enableClockStretching(){
	*i2cctrl= *i2cctrl | 0x00000004;
}

int i2c_gotNACK(){
	// reads bus's response - 0 for ACK, 1 for NACK 
	if ((*i2cctrl & 0x00000001) != 0) return 1;		// Got NACK
	else return 0;									// Got ACK
}

void i2c_stop(){
	// Sends a stop condition to end the transmission
	*i2cctrl = *i2cctrl | 0x00000002;
}

void i2c_printStatus(){
	printf("\n------ S T A T U S ------\n");
	if (i2c_busy()) printf("-- Device is busy!\n");
	else
	{
		printf("-- Device is free!\n");
		if (i2c_lastACK()) printf("-- Last a NACK was sent!\n");
		else printf("-- Last an ACK was sent!\n");
	}

	if (i2c_connected()) printf("-- Device connected!\n");
	else printf("-- No device connected!\n");

	if (i2c_getMode()) printf("-- Device is in read mode!\n");
	else printf("-- Device is in write mode!\n");

	printf("------------------------\n\n");
}

int i2c_write(unsigned int data){
	// Print i2c status
	i2c_printStatus();
	//i2c_enableClockStretching();
	printf("LOG: Trying to read the i2cstatus gives : 0x%0X \n", *i2cstatus);
	// Wait until device is available
	while (i2c_busy());

	// Send the write request
	printf("LOG: Sending Write request!\n");
	printf("LOG: Trying to read the i2caddr gives : 0x%0X \n", *i2caddr);
	*i2caddr = MS5611_WRITE;
	printf("LOG: Trying to read the i2caddr gives : 0x%0X \n", *i2caddr);
	// Wait for ACK
	while (i2c_busy());
	if (i2c_gotNACK()) return 0;
	i2c_printStatus();

	// Send command byte
	printf("LOG: Request received!\n");
	printf("LOG: Sending command!\n");
	*i2cdata = data;
	i2c_printStatus();

	// Wait for ACK
	while (i2c_busy());
	if (i2c_gotNACK()) return 0;

	// Send stop condition
	printf("LOG: Command received!\n");
	printf("LOG: Stopping transmission!\n");
	i2c_stop();
	return 0;
}


int main(int argc, char **argv)
{
  printf("Hello PREDICT!\n");

  i2c_write(0x0000001E);

  return 0;
}
