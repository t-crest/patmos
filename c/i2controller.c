#include <stdio.h>
#include <stdint.h>
#include <machine/patmos.h>
#define IO_PTR_I2C_OFFSET   0xf00e0000
#define OCP_AVAIL_OFFSET    0b000100
#define OCP_READFLAG_OFFSET 0b010000
#define OCP_READDATA_OFFSET 0b001000

volatile _IODEV int *io_ptr = (volatile _IODEV int*) IO_PTR_I2C_OFFSET;
volatile _IODEV int *ocp_avail_ptr = (volatile _IODEV int*) (IO_PTR_I2C_OFFSET | OCP_AVAIL_OFFSET);
volatile _IODEV int *ocp_readflag_ptr = (volatile _IODEV int*) (IO_PTR_I2C_OFFSET | OCP_READFLAG_OFFSET);
volatile _IODEV int *ocp_readdata_ptr = (volatile _IODEV int*) (IO_PTR_I2C_OFFSET | OCP_READDATA_OFFSET);

/**
	Retrieves the value of the ocpAvailable flag
	@return True if OCP is available and can receive a new command, false otherwise
*/
uint8_t ocp_available() {
	return *ocp_avail_ptr;
}

/**
	Writes a byte to an I2C slave device
	@param slave_addr the I2C-address of the slave device (7 bits)
	@param data The byte to be sent to the slave device
*/
void write_byte(uint8_t slave_addr, uint8_t data) {
	int cmd = (slave_addr << 9) | (0 << 8) | data; //0 is the write bit
	*io_ptr = cmd;
}

/**
	Writes several bytes to the same i2c slave device
	@param slave_addr the I2C-address of the slave-device (7 bits)
	@param data Byte array of the values to be sent to the slave
	@param num_bytes the number of bytes in the array
*/
void write_bytes(uint8_t slave_addr, uint8_t* data, uint8_t num_bytes) {
	uint8_t i;
	for(i=0; i<num_bytes; i++) {
		while(!ocp_available()) { //Wait until available

		}
		write_byte(slave_addr, data[i]);
	}
}


/**
	Send a read prompt to the slave, requesting data that can be retrieved later
	@warning If a read prompt is issued, the data MUST be retrieved with get_read_data before another prompt can be made.
	If this is not done, the resulting data from read number 2 may be gibberish
	@param slave_addr The address of the slave device to be read
	@param num_bytes The number of bytes to be read. Must be in the range [1;4]
	@return 0 if successfully prompted, -1 if unsuccesful
*/
uint8_t read_prompt(uint8_t slave_addr, uint8_t num_bytes){
	if(num_bytes < 1 || num_bytes > 4 || !ocp_available()) {
		return -1;
	}
	int cmd = (num_bytes << 16) | (slave_addr << 9) | (1 << 8);
	*io_ptr = cmd;
	return 0;
}

/**
	Checks if read data is available
	@return 1 if read data can be retrieved, 0 otherwise
*/
uint8_t get_read_flag(){
	return *ocp_readflag_ptr;
}

/**
	Retrieves the data read after issuing a command with read_prompt. After reading, a new prompt may be issued
	@param data, a pointer to where data will be stored if read is succesful 
	@return The corresponding value of get_read_flag for this operation
	
*/
uint8_t get_read_data(int *data){
	if(get_read_flag()){
		*data = *ocp_readdata_ptr;
		return 1;
	} else {
		return 0;
	}
}

/**
	A small example showing how to write several bytes to a slave device
*/
void write_ex() {
	uint8_t bytes[]= {0x0e, 0x0e, 0x0e, 0x0e};
	uint8_t slave_addr =  0b1010101;
	write_bytes(slave_addr, bytes, 4);
}

/**
	A small example showing how to read from a slave device
*/
void read_ex() {
	int rd_data;
	uint8_t slave_addr = 0b1001000;
	uint8_t wr_data = 0x01;	

	//Writing a byte to the slave device to setup the read opeartion
	write_byte(slave_addr, wr_data);

	//Issue read prompt, starting data transfer from slave
	read_prompt(slave_addr, 1);

	//Get read data from slave once finished
	while(!get_read_data(&rd_data)){
		//Wait until data is available
	}
	printf("Got: %#010x\n", rd_data);
}

int main() {	
	write_ex();
	return 0;
}