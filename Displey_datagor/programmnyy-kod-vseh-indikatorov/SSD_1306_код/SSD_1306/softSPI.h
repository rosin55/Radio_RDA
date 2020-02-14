#ifndef _SOFT_SPI_H_
#define _SOFT_SPI_H_

#include <avr/io.h>
#include <util/delay.h>

#define cbi(_byte, _bit)      _byte &= ~(1<<_bit)
#define sbi(_byte, _bit)      _byte |=  (1<<_bit)

#define RESET_SW

#define SSD_DDR               DDRB
#define SSD_PORT              PORTB
#define SSD_PIN               PINB
#define DO_pin                PB0
#define DI_pin                PB1
#define DC_pin                PB3
#define CS_pin                PB4
#ifdef RESET_SW
  #define RESET_SW_pin        PB2
  #define RESET_SW_DELAY      10
#endif
  
#define MSBit                 0x80
#define LSBit                 0x01

void spiInit(void);
void spiWriteByte(uint8_t _byte);

#endif /* _SOFT_SPI_H_ */                         
