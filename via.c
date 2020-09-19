#include "via.h"

void	_delay_ms(unsigned short s)
{
	unsigned short count = s*26;
	for (; count; count--);
}

#pragma bss-name (push,"EXTZP")
extern unsigned char via_state;

#pragma zpsym("via_state")
#pragma bss-name (pop)


// period = counter * 2 + 4 clocks
void via_setdivider(unsigned short c)
{
	c=c>>1;
	if (c)
		c--;
	if (c)
		c--;
	VIA_WRITE(VIA_T1CL, c & 0xff);
	VIA_WRITE(VIA_T1CH, c >> 8);
}

// pins:
// CA1 clock from keyboard
// PA6 /reset from keyboard
// PA7 Data from keyboard
// PB7 baud clock
// CB1 30hz input

void via_init()
{
	VIA_WRITE(VIA_DDRB, 0x81);	// PB7 baud clock output, PB0 test output

	VIA_WRITE(VIA_ACR, 0xc0);	// T1 output to PB7

	via_setdivider(10);
	// keyboard
	VIA_WRITE(VIA_DDRA, 0x40);	

	// reset
	VIA_WRITE(VIA_PORTA, 0x00);

	_delay_ms(100);
	VIA_WRITE(VIA_PORTA, 0x40);

	// configure clock on ca1
	VIA_WRITE(VIA_PCR, 0x01);	// lower nibble:INPUT NEG. ACTIVE EDGE, pos ACTIVE EDGE

	via_state = 0x0a;	// setup to receive 8 bits
	VIA_WRITE(VIA_IER, 0x7f);	// disable all
	VIA_WRITE(VIA_IER, 0x92);	// enable interrupt:  CA1, CB1
}
