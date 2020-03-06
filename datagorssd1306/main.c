#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "main.h"
#include "i2c.c"
//#include "spi.c"
#include "ssd1306.c"


int main(void)
  {
	
    ssd1306_init();
  	ssd1306_draw_display();

  	while(1)
	  {

	  }
	return 0;
  }
