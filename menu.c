#include <6502.h>
#include <stdio.h>
#include <stdlib.h>
#include "vtlc.h"
#include "via.h"

#include "keyboard_xt.h"
#pragma bss-name (push,"EXTZP")
extern unsigned short clk_count;
#pragma zpsym("clk_count")
#pragma bss-name (pop)

//#define	F_CPU	4915200
#define	F_CPU	3686400
#define	F_BAUD	(F_CPU/16)
static const unsigned char *menu_bauds[] = { "300", "600", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "76800", "external", 0 };
static const unsigned short menu_dividers[] = {
	(F_BAUD / (300)),
	(F_BAUD / (600)),
	(F_BAUD / (1200)),
	(F_BAUD / (2400)),
	(F_BAUD / (4800)),
	(F_BAUD / (9600)),
	(F_BAUD / (19200)),
	(F_BAUD / (38400)),
	(F_BAUD / (57600)),
	(F_BAUD / (76800)),
	0
};
unsigned int menu_current_baud=0;

void menu_display(unsigned char cSel)
{
	unsigned char c;
	unsigned char menu_char='a';
	
	vtlc_clearscreen(0, 0, 2);
	vtlc_goto(0,0);

	vtlc_puts("Better Linger vt100!");

	for (c=0; menu_bauds[c]; c++)
	{
		vtlc_goto(0,1+c);
		vtlc_putchar(menu_char | (c == menu_current_baud ? 128 : 0));
		vtlc_putchar(' ');
		if (cSel==menu_char)
		{
			menu_current_baud=c;
			if (menu_dividers[c])
				via_setdivider(menu_dividers[c]);
		}
		menu_char++;
		vtlc_puts(menu_bauds[c]);
	}
}

unsigned char menu_active = 0;
void menu()
{
	const unsigned char	*p;
	unsigned char caText[20];
	if (!menu_active)
	{
		menu_active = 1;
		vtlc_beginupdate();
	}
	else
	{
		menu_active=0;
		return;
	}
	while (1)
	{
		menu_display(0);

		while (!(p = keyboard_get()))
		{
			vtlc_goto(40,1);
			utoa(clk_count, caText, 10);
			vtlc_puts(caText);
			if (!menu_active)
			{
				vtlc_clearscreen(0, 0, 2);
				vtlc_goto(0,0);
				vtlc_endupdate();
				return;
			}
		}

		menu_display(*p);


	}
}
