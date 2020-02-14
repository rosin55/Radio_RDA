/*
 * ssd1306.c
 *
 * Created: 03.02.2020 16:41:00
 *  Author: user
 */ 


#include <avr/io.h>
#include "softSPI.h"
#include "ssd1306.h"
#include "graphics.h"
#include "charge.c"
#include "ArialNarrow14.c"
int main(void)
{  ssd1306_init();

	setColor(WHITE);

	setCursor(54, 35);
	drawIcon(charge);
	
	setCursor(2, 17);
	drawHorizontalLine(124);

	setCursor(2, 62);
	drawHorizontalLine(124);

	setCursor(40, 26);
	drawVerticalLine(30);

	setCursor(88, 26);
	drawVerticalLine(30);
	
	setCursor(10, 29);
	drawRectangle(20, 20);

	setCursor(98, 29);
	fillRectangle(20, 20);
	
	setFont(ArialNarrow14);
	setCursor(25, 0);
	printString("Журнал");
	
	ssd1306_draw_display();

    while(1)
    {
        //TODO:: Please write your application code 
    }
}