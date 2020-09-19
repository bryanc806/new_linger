#include "vtlc.h"
#define	VTLCD	0xa000	// data register r/w
#define	VTLCA	0xa001	// address register r, w == status register

#define	VTLC_RESET	0x06
#define	VTLC_TOSADD	0x08
#define	VTLC_CURLO	0x09
#define	VTLC_CURHI	0x0a
#define	VTLC_FILADD	0x0b
#define	VTLC_ATTDAT	0x0c
#define	VTLC_CHAR	0x0d
#define	VTLC_MODE	0x0e

#define	NOQUEUE
#ifdef	NOQUEUE
// write to a register, but leave VTLCA at that register
#define	VTLC_REG_WRITE(a,b)	*(unsigned char *)VTLCA=a; *(unsigned char *)VTLCD = b;

// write to a register, reset VTLCA to VTLC_CHAR
#define	VTLC_REG_WRITE_FINAL(a,b)	*(unsigned char *)VTLCA=a; *(unsigned char *)VTLCD = b; *(unsigned char *)VTLCA=VTLC_CHAR;

// just write to the CHAR register assuming it as left that way.
#define	VTLC_REG_WRITE_CHAR(b)	*(unsigned char *)VTLCD = b;

#define	VTLC_WAIT()	while(!(*(unsigned char *)VTLCA & 0x80))
#else

unsigned short vtlc_enqueue = 0;
unsigned short vtlc_dequeue = 0;
#define	QUEUE_SIZE	2900
unsigned char	vtlc_reg_queue[QUEUE_SIZE];
unsigned char	vtlc_value_queue[QUEUE_SIZE];


void VTLC_REG_WRITE(unsigned char a,unsigned char b)
{
	vtlc_reg_queue[vtlc_enqueue] = a;
	vtlc_value_queue[vtlc_enqueue]=b;
	if (++vtlc_enqueue >= QUEUE_SIZE)
		vtlc_enqueue=0;
	vtlc_flush();	// try to flush out what we can
}
#define	VTLC_WAIT()
#endif
unsigned char	tosaddr = 0;

// cDir == 0, below cursor, cDir == 1, above cursor, cDir == 2, all
void vtlc_clearscreen(unsigned char col, unsigned char row, unsigned char cDir)
{
	switch (cDir)
	{
		case 0:	// cursor to bottom of screen
			vtlc_goto(col, row);
			VTLC_REG_WRITE(VTLC_FILADD, tosaddr);
			break;
		case 1:	// top of screen to row, col
			{
				unsigned char	filadd= row * 5;
				filadd+=(col>>4);
				filadd+=tosaddr;
				if (filadd > 120)
					filadd-=120;
				VTLC_WAIT();
				VTLC_REG_WRITE(VTLC_CURLO, 0);
				VTLC_REG_WRITE(VTLC_CURHI, 0);
				VTLC_REG_WRITE(VTLC_FILADD, filadd);
			}
			break;
		case 2:	// all
			vtlc_goto(0,0);
			VTLC_REG_WRITE(VTLC_FILADD, tosaddr);
			break;
	}
	VTLC_WAIT();
	VTLC_REG_WRITE(VTLC_CHAR, ' ');
}

void vtlc_init()
{
	VTLC_REG_WRITE(VTLC_RESET, 0);
	VTLC_REG_WRITE(VTLC_CHAR, ' ');	// write dummy char data register
	VTLC_WAIT();
	VTLC_REG_WRITE(VTLC_ATTDAT, 0x01);	// block cursor, reverse video for tag bytes
	VTLC_REG_WRITE(VTLC_TOSADD, 0);
	vtlc_clearscreen(0, 0, 2);
	VTLC_REG_WRITE_FINAL(VTLC_MODE, 0x80);
}

void	vtlc_beginupdate()
{
	VTLC_REG_WRITE_FINAL(VTLC_ATTDAT, 0x41);	// disable cursor
}

void	vtlc_endupdate()
{
	VTLC_REG_WRITE_FINAL(VTLC_ATTDAT, 0x01);	// block cursor
}

void vtlc_scroll(signed char c)
{
	tosaddr += 5 * c;
	if (tosaddr > 120)
		tosaddr = 0;
//	tosaddr &=0x7f;	// blank out the 50/60hz bit -- not needed above code will do that
	VTLC_REG_WRITE_FINAL(VTLC_TOSADD, tosaddr);
}

void vtlc_goto(unsigned char col, unsigned char row)
{
	unsigned short	sVal = tosaddr;
	sVal = sVal * 16;
	sVal += (row * 80) + col;
	if (sVal >= 2000)
		sVal-=2000;
	VTLC_WAIT();
	VTLC_REG_WRITE(VTLC_CURLO, sVal & 0x00ff);
	VTLC_REG_WRITE_FINAL(VTLC_CURHI, (sVal >> 8) & 0x07);
}

void vtlc_putchar(unsigned char c)
{
	VTLC_WAIT();
	VTLC_REG_WRITE_CHAR(c);
}

unsigned char vtlc_getchar()
{
	VTLC_WAIT();
	return *(unsigned char *)VTLCD;
}


void vtlc_puts(const unsigned char *p)
{
	while(*p)
	{
		vtlc_putchar(*(p++));
	}
}

void vtlc_flush()
{
#ifdef NOQUEUE
#else
	if (vtlc_dequeue==vtlc_enqueue-1)	// full
	{
		vtlc_flush_all();
		return;
	}
 	while ((*(unsigned char *)VTLCA & 0x80) && (vtlc_dequeue != vtlc_enqueue))
	{
		*(unsigned char *)VTLCA=vtlc_reg_queue[vtlc_dequeue];
		*(unsigned char *)VTLCD = vtlc_value_queue[vtlc_dequeue];
		if (++vtlc_dequeue >= QUEUE_SIZE)
			vtlc_dequeue=0;
	}
#endif
}

void vtlc_flush_all()
{
#ifdef NOQUEUE
#else
 	while (vtlc_dequeue != vtlc_enqueue)
	{
		while (!(*(unsigned char *)VTLCA & 0x80));	// busy

		*(unsigned char *)VTLCA=vtlc_reg_queue[vtlc_dequeue];
		*(unsigned char *)VTLCD = vtlc_value_queue[vtlc_dequeue];
		if (++vtlc_dequeue >= QUEUE_SIZE)
			vtlc_dequeue=0;
	}
#endif
}

