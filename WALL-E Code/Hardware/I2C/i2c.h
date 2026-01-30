#ifndef __I2C_H 
#define __I2C_H 
#include "sys.h"

// GPIO引脚配置（PB6接模块的SCL脚、PB7接模块的SDA脚）
#define GS_IIC_SCL    PBout(6) // SCL（SCL_OUT）
#define GS_IIC_SDA    PBout(7) // SDA_OUT，用于发送SDA数据给IIC传感器模块
#define GS_READ_SDA   PBin(7)  // SDA_IN，用于读取IIC传感器模块的SDA数据

// I/O方向配置(寄存器操作)
#define GS_SDA_IN()  {GPIOB->CRL &= 0x0FFFFFFF; GPIOB->CRL |= 8<<28;} // PB7 浮空输入
#define GS_SDA_OUT() {GPIOB->CRL &= 0x0FFFFFFF; GPIOB->CRL |= 3<<28;} // PB7 推挽输出（50MHz）

// 函数声明
u8 GS_Write_Byte(u8 REG_Address, u8 REG_data);
u8 GS_Read_Byte(u8 REG_Address);
u8 GS_Read_nByte(u8 REG_Address, u16 len, u8 *buf);
void GS_i2c_init(void);
void GS_WakeUp(void);

#endif
