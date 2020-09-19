#include <6502.h>
#include <stdio.h>
#include <stdlib.h>
#include "vtlc.h"
#include "uart.h"
#include "vt100.h"
#include "via.h"
#include "keyboard_xt.h"

#pragma bss-name (push,"EXTZP")
extern volatile unsigned short clk_count;
#pragma zpsym("clk_count")
#pragma bss-name (pop)
#define	TENMINUTES	60*30*10

int main (void)
{
	unsigned char c = 0;
	const unsigned char *p = 0;
	unsigned short	lastActivity=0;
	unsigned char	blankScreen=0;
//	char	caText[16];

	vtlc_init();
	vtlc_flush_all();
	via_init();


//	vtlc_puts("Better Linger VT100!");
//	vtlc_goto(0, 1);
//	for (c = ' '; c < 128; c++)
//	{
//		_delay_ms(20);
//		vtlc_putchar(c);
//		p = keyboard_get();
//		if (p)
//		{
//			c = ' ';
//		}
//	};

	vt100_init();
	
	uart_init();

	CLI();

//	for (loop=' '; loop < 'z'; loop++)
//		for (c = 0; c < 1920; c++)
//			vtlc_putchar(loop);

	while(1)
	{
		p = keyboard_get();
		if (p)
		{
			while (*p)
			{
				uart_putc(*(p++));
			}
			SEI();
			lastActivity=clk_count;
			CLI();
			blankScreen=0;
		}
		if (uart_available())
		{
			vtlc_beginupdate();
			vt100_putchars();
			vtlc_endupdate();
			SEI();
			lastActivity=clk_count;
			CLI();
			blankScreen=0;
			vt100_refresh();
		}
		SEI();
		if (clk_count-lastActivity > TENMINUTES && !blankScreen)
		{
			CLI();
			blankScreen=1;
			vtlc_beginupdate();
			vtlc_clearscreen(0,0,2);
		}
		CLI();
	}
	return 1;
}
