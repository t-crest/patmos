#ifndef FLIGHT_BMI055_H
#define FLIGHT_BMI055_H
#include "common.h"
#include "i2c.h"
#include <machine/rtc.h>

#define REG_CHIPID 0x00 //Chip ID Read only

#define REG_ACC_X_LSB 0x02 // X-axis LSB. Read only. Locks MSB when accessed
#define REG_ACC_X_MSB 0x03 // X-axis MSB. Read only.
#define REG_ACC_Y_LSB 0x04 // Y-axis LSB. Read only.
#define REG_ACC_Y_MSB 0x05 // Y-axis MSB
#define REG_ACC_Z_LSB 0x06
#define REG_ACC_Z_MSB 0x07

#define REG_TEMP 0x08

#define REG_INT_STATUS_0 0x09 //Interrupt status flags. Read only.
#define REG_INT_STATUS_1 0x0A //Interrupt status flags. Read only.
#define REG_INT_STATUS_2 0x0B //Interrupt status flags. Read only.
#define REG_INT_STATUS_3 0x0C //Interrupt status flags. Read only.

#define REG_FIFO_STATUS 0x0E //FIFO frame counter and overrun signal. Read only.

#define REG_PMU_RANGE 0x0F     //Accelerometer g-range selection. Read/Write.
#define REG_PMU_BW 0x10        //Acceleration data filter bandwidth selection. Read/Write.
#define REG_PMU_LPW 0x11       //Power mode selection. Read/Write.
#define REG_PMU_LPM 0x12       //Low power mode configuration settings. Read/Write.
#define REG_ACC_HBW 0x13       //Acceleration data format settings. Read/Write.
#define REG_BGW_SOFTRESET 0x14 //Reset trigger. Write only.
#define REG_INT_EN_0 0x16      //Interrupt group 0 enable. Read/Write.
#define REG_INT_EN_1 0x17      //Interrupt group 1 enable. Read/Write.
#define REG_INT_EN_2 0x18      //Interrupt group 2 enable. Read/Write.
#define REG_INT_MAP_0 0x19     //Map int signal to INT1 pin. Read/Write.
#define REG_INT_MAP_1 0x1A     //Map int signal to INT1 & INT2 pins. Read/Write.
#define REG_INT_MAP_2 0x1B     //Map int signal to INT2 pin. Read/Write.
#define REG_INT_SRC 0x1E       //Data source definition for interrupts with selectable data source. Read/Write
#define REG_INT_OUT_CTRL 0x20  //Electrical behaviour of INT pins. Read/Write.
#define REG_INT_RST_LATCH 0x21 //Interrupt reser bit & mode selection. Read/Write.
#define REG_INT_0 0x22         //Delay for the low-g interrupt.
#define REG_INT_1 0x23         //Threshold for low-g interrupt.
#define REG_INT_2 0x24         //Low-g interrupt mode, interrupt hysterisis, high-g hysterisis.
#define REG_INT_3 0x25         //Delay for the high-g interrupt.
#define REG_INT_4 0x26         //Threshold for high-g interrupt.
#define REG_INT_5 0x27         //Num of samlpe for slope interrupt.
#define REG_INT_6 0x28         //Threshold for motion detect intrrupt.
#define REG_INT_7 0x29         //Threshold for slow/no-motion intrrupt.
#define REG_INT_8 0x2A         //Timing for single and double tap intrerrupts.
#define REG_INT_9 0x2B         //Samples processed by single/double tap after wakeup. Threshold for single/double tap interrupts.
#define REG_INT_A 0x2C         //Definition of Hysterisis, blokcing and mode for orientation interrupt.
#define REG_INT_B 0x2D         //Definition of axis orientation, up/down masking and theta blocking for orientation interrupt.
#define REG_INT_C 0x2E         //Flat threshold angle  for flat interrupt.
#define REG_INT_D 0x2F         //Definition of flat interrupt hold time and interrupt hysterisis.
#define FIFO_CONFIG_0 0x30     //FIFO watermark level.
#define PMU_SELF_TEST 0x32     //Settings for sensor self-test config and trigger.
#define TRIM_NVM_CTRL 0x33     //Settings for few-time programmable NVM.
#define BGW_SPI3_WDT 0x34      //Settings for interfaces.
#define OFC_CTRL 0x36          //Control and config of fast and slow offset compensation.
#define OFC_SETTING 0x37       //Config of fast and slow offset compensation.
#define OFC_OFFSET_X 0x38      //Offset compensation value for x-axis.
#define OFC_OFFSET_Y 0x39      //Offset compensation value for y-axis.
#define OFC_OFFSET_Z 0x3A      //Offset compensation value for z-axis.
#define TRIM_GP0 0x3B          //GP data register with NVM backup.
#define TRIM_GP1 0x3C          //GP data register with NVM backup.
#define FIFO_CONFIG_1 0x3E     //FIFO buffer clear and FIFO-full flag is cleared when writing here.
#define FIFO_DATA 0x3F         //FIFO data readout register.

struct Report
{
    int timestamp;
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    float temp;
} report;

uint8_t read_reg(unsigned sensor, unsigned reg);
uint16_t read_reg16(unsigned sensor, unsigned reg_msb, unsigned reg_lsb);

void modify_reg(unsigned sensor, unsigned reg, uint8_t clearbits, uint8_t setbits);
void write_reg(unsigned sensor, unsigned reg, unsigned char value);
#endif