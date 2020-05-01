#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <machine/patmos.h>

//I2C controller
#define I2C (*((volatile _IODEV unsigned *)PATMOS_IO_I2C))

//Define sensor adresses
#define BARO 0x77       //MS561101
#define MAGNETO 0x1E    //IIS2
#define IMU_1 0x68      //ICM20689
#define IMU_2 0x18      //MBMI055



int i2c_write(unsigned char chipaddress, unsigned char regaddress, unsigned char data);

int i2c_read(unsigned char chipaddress, unsigned char regaddress);