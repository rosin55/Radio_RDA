#include "ssd1306.h"

void ssd1306_command(uint8_t command) 
{
  #if defined I2C_INTERFACE
    i2c_start();
    i2c_write((SSD1306_ADDRESS << 1) | WRITE); 
    i2c_write(CONTROL_BYTE_COMMAND);
    i2c_write(command);
    i2c_stop();  
  
  #elif defined SPI_INTERFACE
    cbi(SSD_PORT, DC_pin); 
    cbi(SSD_PORT, CS_pin); 
    SPI_byte(command);
    sbi(SSD_PORT, CS_pin);   
  #endif  
}

void ssd1306_data(uint8_t data) 
{
  #if defined I2C_INTERFACE
    i2c_start();
    i2c_write((SSD1306_ADDRESS << 1) | WRITE); 
    i2c_write(CONTROL_BYTE_DATA);
    i2c_write(data);
    i2c_stop();  
  
  #elif defined SPI_INTERFACE
    sbi(SSD_PORT, DC_pin); 
    cbi(SSD_PORT, CS_pin); 
    SPI_byte(data);
    sbi(SSD_PORT, CS_pin);   
  #endif  
}

void ssd1306_reset_hw(void)
{
  #ifdef RESET_HW
    cbi(SSD_PORT, RESET_HW_pin); 
    _delay_us(RESET_HW_DELAY);
    sbi(SSD_PORT, RESET_HW_pin);     
  #endif  
}

void ssd1306_reset_sw(void)
{
  /*Fundamental settings*/
  ssd1306_command(SSD1306_SETCONTRAST);                   
    ssd1306_command(RESET_CONTRAST);  
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME);   
  ssd1306_command(SSD1306_NORMALDISPLAY);                       
  ssd1306_command(SSD1306_DISPLAYOFF);                       
  
  /*Scroll settings*/
  ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);                       
    ssd1306_command(RESET_TOP_ROW);                       
    ssd1306_command(RESET_ROWS_NUMBER);    
  
  /*Addressing settings*/                       
  ssd1306_command(SSD1306_SETLOWCOLUMN | RESET_LOWER_NIBBLE);                       
  ssd1306_command(SSD1306_SETHIGHCOLUMN | RESET_HIGHER_NIBBLE);                       
  ssd1306_command(SSD1306_MEMORYMODE);                       
    ssd1306_command(PAGE_ADDRESSING);                       
  ssd1306_command(SSD1306_COLUMNADDR);                       
    ssd1306_command(RESET_COLUMNADDR);                       
  ssd1306_command(SSD1306_PAGEADDR);                       
    ssd1306_command(RESET_PAGEADDR);                       
  ssd1306_command(SSD1306_PAGESTARTADDR | PAGE0);                           

  /*Hardware settings*/      
  ssd1306_command(SSD1306_SETSTARTLINE | RESET_STARTLINE);
  ssd1306_command(SSD1306_SEGREMAP | SEGREMAP_OFF);
  ssd1306_command(SSD1306_SETMULTIPLEX);                
    ssd1306_command(RESET_MUX_RATIO);  
  ssd1306_command(SSD1306_COMSCANINC);                  
  ssd1306_command(SSD1306_SETDISPLAYOFFSET);              
    ssd1306_command(RESET_OFFSET);  
  ssd1306_command(SSD1306_SETCOMPINS);                    
    ssd1306_command(RESET_SETCOMPINS);     
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            
    ssd1306_command(RESET_CLOCKDIV);       
  ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(RESET_PRECHARGE);  
  ssd1306_command(SSD1306_SETVCOMDETECT);                 
    ssd1306_command(RESET_SETVCOMDETECT);                          
  ssd1306_command(SSD1306_CHARGEPUMP);                    
    ssd1306_command(CHARGEPUMP_DISABLE); 
}

void ssd1306_init(void)
{
  #if defined I2C_INTERFACE
    i2c_init();  
  #elif defined SPI_INTERFACE
    SPI_init();  
  #endif  

  #ifndef RESET_HW
    ssd1306_reset_sw();
  #else
    ssd1306_reset_hw();      
  #endif      
  
  ssd1306_command(SSD1306_CHARGEPUMP);                    
    ssd1306_command(CHARGEPUMP_ENABLE); 
  ssd1306_command(SSD1306_MEMORYMODE);                    
    ssd1306_command(HORIZONTAL_ADDRESSING);                                  
  ssd1306_command(SSD1306_SEGREMAP | SEGREMAP_ON);
  ssd1306_command(SSD1306_COMSCANDEC);
  ssd1306_command(SSD1306_SETCONTRAST);                   
    ssd1306_command(0xCF);
  ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1); 
  ssd1306_command(SSD1306_DISPLAYON);     
  _delay_ms(120);        
}

void ssd1306_clear_display(void) 
{
  memset(buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8));
}

void ssd1306_draw_display(void) 
{
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(0);   
  ssd1306_command(SSD1306_LCDWIDTH-1); 
  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(0); 
  ssd1306_command(7); 

  #if defined I2C_INTERFACE
    i2c_start();
    i2c_write((SSD1306_ADDRESS << 1) | WRITE); 
    i2c_write(CONTROL_BYTE_DATA);
    for (uint16_t i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8); i++) 
      {
        i2c_write(buffer[i]);
      }
    i2c_stop();

  #elif defined SPI_INTERFACE
    sbi(SSD_PORT, DC_pin); 
    cbi(SSD_PORT, CS_pin); 
    for (uint16_t i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8); i++) 
      {
        SPI_byte(buffer[i]);
      }
    sbi(SSD_PORT, CS_pin);     
  #endif  
}

void draw_pixel(int16_t x, int16_t y, uint16_t color) 
{
  switch (color) 
    {
      case WHITE:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] |=  (1 << (y&7)); break;
      case BLACK:   buffer[x+ (y/8)*SSD1306_LCDWIDTH] &= ~(1 << (y&7)); break; 
    }
}

void ssd1306_set_rotation(uint8_t rotation) 
{
  if(rotation == 1)
    {
      ssd1306_command(SSD1306_SEGREMAP | SEGREMAP_ON);      
      ssd1306_command(SSD1306_COMSCANDEC);   
    }

  if(rotation == 2)
    {
      ssd1306_command(SSD1306_SEGREMAP | SEGREMAP_OFF);      
      ssd1306_command(SSD1306_COMSCANINC);   
    }
}

void ssd1306_stop_scroll(void)
{
  ssd1306_command(SSD1306_DEACTIVATE_SCROLL);  
}

void ssd1306_scroll_right(uint8_t start_page, uint8_t stop_page)
{
  ssd1306_command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
  ssd1306_command(0x00);
  ssd1306_command(start_page);
  ssd1306_command(0x00);
  ssd1306_command(stop_page);
  ssd1306_command(0x00);
  ssd1306_command(0xFF);
  ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void ssd1306_scroll_left(uint8_t start_page, uint8_t stop_page)
{
  ssd1306_command(SSD1306_LEFT_HORIZONTAL_SCROLL);
  ssd1306_command(0x00);
  ssd1306_command(start_page);
  ssd1306_command(0x00);
  ssd1306_command(stop_page);
  ssd1306_command(0x00);
  ssd1306_command(0xFF);
  ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void ssd1306_scroll_diagonal_right(uint8_t start_page, uint8_t stop_page)
{
  ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);  
  ssd1306_command(0x00);
  ssd1306_command(SSD1306_LCDHEIGHT);
  ssd1306_command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  ssd1306_command(0x00);
  ssd1306_command(start_page);
  ssd1306_command(0x00);
  ssd1306_command(stop_page);
  ssd1306_command(0x01);
  ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void ssd1306_scroll_diagonal_left(uint8_t start_page, uint8_t stop_page)
{
  ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);  
  ssd1306_command(0x05);
  ssd1306_command(15);
  ssd1306_command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  ssd1306_command(0x00);
  ssd1306_command(start_page);
  ssd1306_command(0x00);
  ssd1306_command(stop_page);
  ssd1306_command(0x01);
  ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void ssd1306_set_contrast(uint8_t contrast)
{
  if((contrast > 0) && (contrast < 256))
    {
      ssd1306_command(SSD1306_SETCONTRAST);                   
      ssd1306_command(contrast);      
    }
}