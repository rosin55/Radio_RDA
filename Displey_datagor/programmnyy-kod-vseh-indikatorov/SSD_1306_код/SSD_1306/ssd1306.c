#include "ssd1306.h"

static uint8_t buffer[SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT/8];

void ssd1306_command(uint8_t command) {
  cbi(SSD_PORT, DC_pin); 
  cbi(SSD_PORT, CS_pin); 
    spiWriteByte(command);
  sbi(SSD_PORT, CS_pin);   
}

void ssd1306_data(uint8_t data) {
  sbi(SSD_PORT, DC_pin); 
  cbi(SSD_PORT, CS_pin); 
    spiWriteByte(data);
  sbi(SSD_PORT, CS_pin);   
}

void ssd1306_reset_sw(void) {
  #ifdef RESET_SW
    cbi(SSD_PORT, RESET_SW_pin); 
    _delay_us(RESET_SW_DELAY);
    sbi(SSD_PORT, RESET_SW_pin);     
    _delay_ms(120);
  #endif  
}

void ssd1306_init(void) {
  spiInit();
  
  #ifdef RESET_SW
    ssd1306_reset_sw();
  #endif  
  
  ssd1306_command(SSD1306_CHARGEPUMP);                    
    ssd1306_command(CHARGEPUMP_ENABLE); 
  ssd1306_command(SSD1306_MEMORYMODE);                    
    ssd1306_command(HORIZONTAL_ADDRESSING);    
  ssd1306_command(SSD1306_DISPLAYON);     
  _delay_ms(120);   

  ssd1306_set_rotation(1);    
  ssd1306_clear_display();
  ssd1306_draw_display();    
}

void ssd1306_clear_display(void) {
  memset(buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
}

void ssd1306_draw_display(void) {
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(0);   
  ssd1306_command(SSD1306_LCDWIDTH-1); 
  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(0); 
  ssd1306_command(7); 

  sbi(SSD_PORT, DC_pin); 
  cbi(SSD_PORT, CS_pin); 
  for (uint16_t i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8); i++) {
    spiWriteByte(buffer[i]);
  }
  sbi(SSD_PORT, CS_pin);     
}

void ssd1306_set_rotation(uint8_t rotation) {
  if(rotation == 1) {
    ssd1306_command(SSD1306_SEGREMAP | SEGREMAP_ON);      
    ssd1306_command(SSD1306_COMSCANDEC);   
  }
  if(rotation == 2) {
    ssd1306_command(SSD1306_SEGREMAP | SEGREMAP_OFF);      
    ssd1306_command(SSD1306_COMSCANINC);   
  }
}

void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
  switch (color) {
    case WHITE:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
    case BLACK:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break; 
  }
}
