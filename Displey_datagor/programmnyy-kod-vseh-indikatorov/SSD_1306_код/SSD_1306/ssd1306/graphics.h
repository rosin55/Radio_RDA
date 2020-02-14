#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <stdint.h> 
#include <avr/pgmspace.h>

#define MSbit             0x80

#define DELETE_CODE       0x7F
#define RUS_YO_CODE       0xA8
#define RUS_YO_NUMBER     0xC0
#define RUS_yo_CODE       0xB8
#define RUS_yo_NUMBER     0xC1
#define SUBTRUCT          0x40

typedef struct {
  const uint8_t *data;
  uint16_t width;
  uint16_t hight;
} tImage ;

tImage currentFont;

void setCursor(uint16_t x, uint16_t y);
void setColor(uint16_t color);
void drawVerticalLine(uint16_t _hight);
void drawHorizontalLine(uint16_t _length);
void drawRectangle(uint16_t _width, uint16_t _hight);
void fillRectangle(uint16_t _width, uint16_t _hight);
void drawByte(uint8_t _byte);
void drawIcon(tImage structureName);
void setFont(tImage Name);
void printChar(uint8_t BlockNum);
void printString(char *myString);

uint16_t currentX, currentY, currentColor;

#endif /*_GRAPHICS_H_ */ 
