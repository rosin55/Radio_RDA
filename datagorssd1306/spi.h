/* File spi.h */
#ifndef SPI_H
#define SPI_H

#define cbi(_byte, _bit)      _byte &= ~(1<<_bit)
#define sbi(_byte, _bit)      _byte |=  (1<<_bit)

#define SSD_DDR               DDRB
#define SSD_PORT              PORTB
#define SSD_PIN               PINB
#define DO_pin                PB0
#define DI_pin                PB1
#define CS_pin                PB2
#define DC_pin                PB3
#ifdef RESET_HW
  #define RESET_HW_pin        PB4
  #define RESET_HW_DELAY      10
#endif
  
#define MSBit                 0x80
#define LSBit                 0x01

void SPI_init(void);
uint8_t SPI_byte(uint8_t byte);

#endif /* SPI_H */