#include "spi.h"
#include <avr/io.h>

void SPI_init(void)
{
  SSD_DDR = (1<<DO_pin) | (1<<DI_pin) | (1<<CS_pin) | (1<<DC_pin);
  sbi(SSD_PORT, CS_pin);
  
  #ifdef RESET_HW
    SSD_DDR |= 1 << RESET_HW_pin;
    sbi(SSD_PORT, RESET_HW_pin);
  #endif
}

uint8_t SPI_byte(uint8_t byte)
{
  for(uint8_t counter = 8; counter; counter--)
    {
      if (byte & MSBit)
        sbi(SSD_PORT, DI_pin);
      else
        cbi(SSD_PORT, DI_pin);
      byte <<= 1;
      sbi(SSD_PORT, DO_pin); 
      cbi(SSD_PORT, DO_pin); 
    }
}