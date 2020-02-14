#include "softSPI.h"

void spiInit(void) {
  SSD_DDR = (1<<DO_pin) | (1<<DI_pin) | (1<<CS_pin) | (1<<DC_pin);
  sbi(SSD_PORT, CS_pin);
  
  #ifdef RESET_HW
    SSD_DDR |= 1 << RESET_HW_pin;
    sbi(SSD_PORT, RESET_HW_pin);
  #endif
}

void spiWriteByte(uint8_t _byte) {
  for(uint8_t counter = 8; counter; counter--) {
      if (_byte & MSBit)
        sbi(SSD_PORT, DI_pin);
      else
        cbi(SSD_PORT, DI_pin);
      _byte <<= 1;
      sbi(SSD_PORT, DO_pin); 
      cbi(SSD_PORT, DO_pin); 
    }
}
