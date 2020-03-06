
#include "Arduino.h"
#include "WProgram.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "main.h"
#include "i2c.c"
//#include "spi.c"
#include "ssd1306.c"

void void setup()
{
    ssd1306_init();
  	ssd1306_draw_display();
	
}

void void loop()
{
	
}