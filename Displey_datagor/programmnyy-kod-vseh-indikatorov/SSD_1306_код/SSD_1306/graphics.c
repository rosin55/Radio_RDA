#include "graphics.h"

void setCursor(uint16_t x, uint16_t y) {
  currentX = x; 
  currentY = y;
}

void setColor(uint16_t color) {
  currentColor = color; 
}

void drawVerticalLine(uint16_t _hight) {
  while(_hight--) {
    draw_pixel(currentX, currentY, currentColor);  
    currentY++;
  }
}

void drawHorizontalLine(uint16_t _length) {
  while(_length--) {
    draw_pixel(currentX, currentY, currentColor);  
    currentX++;
  }
}

void drawRectangle(uint16_t _width, uint16_t _hight) {
  uint16_t startX = currentX, startY = currentY;   
  
  drawVerticalLine(_hight);
  currentY--;  
  drawHorizontalLine(_width);

  currentX = startX; currentY = startY; 
  
  drawHorizontalLine(_width); 
  currentX--;  
  drawVerticalLine(_hight);    
}

void fillRectangle(uint16_t _width, uint16_t _hight) {
  uint16_t startY = currentY;  

  while(_width--) {
    drawVerticalLine(_hight);    
    currentX++;    
    currentY = startY;
  }
}

void drawByte(uint8_t _byte) {
  uint8_t bitNumber = 8;
  
  while(bitNumber--) {
    if((_byte & MSbit) != 0) {
      draw_pixel(currentX, currentY, currentColor);   
    }
    
    _byte <<= 1;
    currentY++;
  }
}

void drawIcon(tImage structureName) {
  uint16_t startX = currentX, startY = currentY;

  uint16_t imageByteNum = 0;
  
  uint16_t bytesPerHight = (uint16_t)(structureName.hight / 8); 
  if ((bytesPerHight * 8) != structureName.hight) bytesPerHight += 1;
  
  for(uint16_t imageHigt = 0; imageHigt < bytesPerHight; imageHigt++) {
    for(uint16_t imageWidth = 0; imageWidth < structureName.width; imageWidth++) {
      drawByte(pgm_read_byte(&structureName.data[imageByteNum]));
      currentX++;
      currentY = startY;
      imageByteNum++;      
    }
      
      startY += 8;
      currentY = startY;  
      currentX = startX;
  }      
}

void setFont(tImage FontName)
{
  currentFont.data = FontName.data; 
  currentFont.width = FontName.width; 
  currentFont.hight = FontName.hight;  
}

void printChar(uint8_t BlockNum) 
{
  uint16_t startX = currentX, startY = currentY;
 
  uint16_t bytesPerHight = (uint16_t)(currentFont.hight / 8); 
  if ((bytesPerHight * 8) != currentFont.hight) bytesPerHight += 1;  

  uint16_t imageByteNum = BlockNum*bytesPerHight*currentFont.width;
  
  for(uint16_t imageHigt = 0; imageHigt < bytesPerHight; imageHigt++) {
    for(uint16_t imageWidth = 0; imageWidth < currentFont.width; imageWidth++) {
      drawByte(pgm_read_byte(&currentFont.data[imageByteNum]));
      currentX++;
      currentY = startY;
      imageByteNum++;      
    }
      
      startY += 8;
      currentY = startY;  
      currentX = startX;
  }          
}

void printString(char *myString)
{
  uint16_t startY = currentY, startX = currentX;
  uint16_t BlockNum = 0;
  
  while(*myString != '\0')
  	{
		if(*myString <= DELETE_CODE)
			{
				BlockNum = *myString;
			}
		else 
			{
				if(*myString == RUS_YO_CODE)
					{
						BlockNum = RUS_YO_NUMBER;
					}
				else if(*myString == RUS_yo_CODE)
					{
						BlockNum = RUS_yo_NUMBER;
					}
				else
					{
						BlockNum = *myString - SUBTRUCT;
					}	
			}
		
			
		printChar(BlockNum);  
      	currentY = startY;
      	startX += (currentFont.width + 1);
      	currentX = startX;
      	myString++;
	}	
}
