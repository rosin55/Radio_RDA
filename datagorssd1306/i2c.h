/* File i2c.h */
#ifndef I2C_H
#define I2C_H

#define SCLPORT                 PORTC 
#define SCLDDR                  DDRC  
#define SCLPIN                  PINC  

#define SDAPORT                 PORTC 
#define SDADDR                  DDRC  
#define SDAPIN                  PINC  

#define SCL                     PC5
#define SDA                     PC4


#define SOFT_I2C_SDA_LOW        SDADDR|=((1<<SDA))
#define SOFT_I2C_SDA_HIGH       SDADDR&=(~(1<<SDA))
#define SOFT_I2C_SCL_LOW        SCLDDR|=((1<<SCL))
#define SOFT_I2C_SCL_HIGH       SCLDDR&=(~(1<<SCL))
#define Q_DEL                   _delay_loop_2(3)
#define H_DEL                   _delay_loop_2(5)

#define SSD1306_ADDRESS         0x3C
#define DC_BIT                  6
#define CONTROL_BYTE_COMMAND    0 << DC_BIT  
#define CONTROL_BYTE_DATA       1 << DC_BIT 

#define WRITE                   0

#include <avr/io.h>

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t data);

#endif /* I2C_H */