#include "mpu9250.h"
#include "common.h"

int getMPU9250WhoAmI(){
	return i2c_read(MPU9250_CHIP_ADDR, MPU9250_REG_WHO_AM_I);
}

void writeRegByteMPU9250(unsigned char reg,  unsigned char value) {
    i2c_write(MPU9250_CHIP_ADDR, reg, value);
}

unsigned char readRegByteMPU9250(unsigned char reg){
    int reply = i2c_read(MPU9250_CHIP_ADDR, reg);
    if(reply > 0){ 
	    return (unsigned char) reply;
    } else {
        return 0;
    }
}

int initMPU9250(){

    if(getMPU9250WhoAmI() == MPU9250_WHO_AM_I_ID)
    {
        writeRegByteMPU9250(MPU9250_REG_PWR_MGMT_1, MPU9250_RESET_MASK & 0xFF);//reset the whole module first
        DEAD = 50000000 / CPU_PERIOD; //wait for 50ms for the gyro to stable
        writeRegByteMPU9250(MPU9250_REG_PWR_MGMT_1, 0x1);//PLL with Z axis gyroscope reference
        writeRegByteMPU9250(MPU9250_REG_CONFIG, 0x01);        //DLPF_CFG = 1: Fs=1khz; bandwidth=42hz
        writeRegByteMPU9250(MPU9250_REG_SMPLRT_DIV, 0x01);    //1kHz / (1+SMPLRT_DIV) sample rate ~ 2.9ms
        writeRegByteMPU9250(MPU9250_REG_GYRO_CONFIG, MPU9250_GYRO_FS_2000);    //Gyro full scale setting
        writeRegByteMPU9250(MPU9250_REG_ACCEL_CONFIG, MPU9250_ACCEL_FS_2);    //Accel full scale setting
        //writeRegByteMPU9250(MPU9250_REG_INT_PIN_CFG, 0x30);        //interrupt status bits are cleared on any read operation
        writeRegByteMPU9250(MPU9250_REG_SIGNAL_PATH_RESET, 0x07);//reset gyro and accel sensor
        return 1;
    }
    else
    {
        return 0;
    }
}

short getMPU9250RawAccelX(){
	return (short)(readRegByteMPU9250(MPU9250_REG_ACCEL_XOUT_H)<<8)+(readRegByteMPU9250(MPU9250_REG_ACCEL_XOUT_L));
}

short getMPU9250RawAccelY(){
    return (short)(readRegByteMPU9250(MPU9250_REG_ACCEL_YOUT_H)<<8)+(readRegByteMPU9250(MPU9250_REG_ACCEL_YOUT_L));
}

short getMPU9250RawAccelZ(){
	return (short)(readRegByteMPU9250(MPU9250_REG_ACCEL_ZOUT_H)<<8)+(readRegByteMPU9250(MPU9250_REG_ACCEL_ZOUT_L));
}

double getMPU9250AccelX(){
	return ((double)getMPU9250RawAccelX() / 16384.0f);
}
double getMPU9250AccelY(){
	return ((double)getMPU9250RawAccelY() / 16384.0f);
}
double getMPU9250AccelZ(){
	return ((double)getMPU9250RawAccelZ() / 16384.0f);
}

short getMPU9250RawGyroX(){
	return (short)(readRegByteMPU9250(MPU9250_REG_GYRO_XOUT_H)<<8)+(readRegByteMPU9250(MPU9250_REG_GYRO_XOUT_L));
}

short getMPU9250RawGyroY(){
    return (short)(readRegByteMPU9250(MPU9250_REG_GYRO_YOUT_H)<<8)+(readRegByteMPU9250(MPU9250_REG_GYRO_YOUT_L));
}

short getMPU9250RawGyroZ(){
	return (short)(readRegByteMPU9250(MPU9250_REG_GYRO_ZOUT_H)<<8)+(readRegByteMPU9250(MPU9250_REG_GYRO_ZOUT_L));
}