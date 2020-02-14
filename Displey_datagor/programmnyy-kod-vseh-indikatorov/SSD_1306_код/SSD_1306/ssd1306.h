#ifndef _SSD_1306_H_
#define _SSD_1306_H_

#include <util/delay.h>
#include <string.h>
#include <stdint.h> 
#include "softSPI.h"

#define SSD1306_LCDWIDTH            128
#define SSD1306_LCDHEIGHT           64

#define BLACK                       0
#define WHITE                       1

#define SSD1306_CHARGEPUMP          0x8D
  #define CHARGEPUMP_DISABLE      0x10
  #define CHARGEPUMP_ENABLE       0x14
#define SSD1306_DISPLAYON           0xAF

#define SSD1306_MEMORYMODE          0x20
  #define HORIZONTAL_ADDRESSING   0x00
#define SSD1306_SEGREMAP            0xA0
  #define SEGREMAP_OFF            0x00
  #define SEGREMAP_ON             0x01
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
  
#define SSD1306_COLUMNADDR          0x21  
#define SSD1306_PAGEADDR            0x22

void ssd1306_command(uint8_t command);
void ssd1306_data(uint8_t data);
void ssd1306_reset_sw(void);
void ssd1306_init(void);
void ssd1306_clear_display(void); 
void ssd1306_draw_display(void); 
void ssd1306_set_rotation(uint8_t rotation); 
void draw_pixel(uint16_t x, uint16_t y, uint16_t color);   

#endif /* _SSD_1306_H_ */   
