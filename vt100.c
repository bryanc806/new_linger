#include "vt100.h"
#include "vtlc.h"
#include "uart.h"
#include <string.h>

//#define	DEBUG_SEQ
//#define	VTMEM
#define	SCREEN_ROWS	24
#define	SCREEN_COLS	80
#pragma data-name (push,"ZEROPAGE")
signed char vt100_cur_row=0;
signed char vt100_cur_col=0;
unsigned char	vt100_attributes=0;
unsigned char	vt100_wrap_pending=0;
#ifdef VTMEM
unsigned char	vt100_screen_firstrow=0;
unsigned char	vt100_firstrow_affected=0;
unsigned char	vt100_lastrow_affected=0;
#endif
#pragma data-name (pop)


unsigned char	vt100_tab_stops[SCREEN_COLS];
#ifdef DEBUG_SEQ
#define	LAST_FEW_SIZE 30
unsigned char	vt100_last_few[LAST_FEW_SIZE];
unsigned char	vt100_last_few_start;
#endif

#ifdef VTMEM
unsigned char	vt100_screen[SCREEN_ROWS][SCREEN_COLS];
#endif

void	vt100_scroll()
{
	unsigned char	c1 = 0;
#ifdef VTMEM
	unsigned char	c2 = 0;
	vt100_screen_firstrow++;
	c2 = vt100_screen_firstrow;
	if (vt100_screen_firstrow >= SCREEN_ROWS)
		vt100_screen_firstrow = 0;
	for (c1 = 0; c1 < SCREEN_COLS; c1++)
		vt100_screen[c2][c1] = ' ';
	vt100_firstrow_affected = 0;
	vt100_lastrow_affected = SCREEN_ROWS-1;
#else
	vtlc_scroll(1);
	vtlc_goto(0, SCREEN_ROWS);	// note+1
	for (c1 = 0; c1 < SCREEN_COLS; c1++)
		vtlc_putchar(' ');
	
#endif
}

void	vt100_next_tab(signed char n)
{
	unsigned char c = 0;
	for (c = vt100_cur_col; c < SCREEN_COLS; c++)
	{
		if (vt100_tab_stops[c])
		{
			vt100_cur_col = c;
			n--;
			if (!n)
				break;
		}
	}
#ifndef VTMEM
	vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
}

void	vt100_next_row(signed char n, unsigned char scroll)
{
	vt100_cur_row+=n;
	if (vt100_cur_row >= SCREEN_ROWS)
	{
		if (scroll)
			vt100_scroll();
		vt100_cur_row = SCREEN_ROWS-1;
	}
	if (vt100_cur_row < 0)
		vt100_cur_row = 0;
#ifndef VTMEM
	vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
}

void	vt100_next_col(signed char n)
{
	vt100_cur_col+=n;
	if (vt100_cur_col >= SCREEN_COLS)
	{
		vt100_cur_col=SCREEN_COLS-1;
#ifndef VTMEM
		vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
		return;
	}
	if (vt100_cur_col < 0)
	{
		vt100_cur_col=0;
#ifndef VTMEM
		vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
		return;
	}
	if (n < 0 || n > 1)	// for all non-trivial cases, explicilty set the cursor location
		vtlc_goto(vt100_cur_col, vt100_cur_row);
}


void	vt100_insert_chars(char cCount)
{
	unsigned char cBuf[SCREEN_COLS];
	unsigned char	c;
	vtlc_getchar();	// preseed
	for (c=vt100_cur_col; c < SCREEN_COLS;c++)
		cBuf[c] = vtlc_getchar();
	vtlc_goto(vt100_cur_col, vt100_cur_row);
	for (c = 0; c < cCount; c++)
		vtlc_putchar(' ');
	for (c = vt100_cur_col; c < SCREEN_COLS-cCount; c++)
	{
		vtlc_putchar(cBuf[c]);
	}
 	vtlc_goto(vt100_cur_col, vt100_cur_row);
}

void	vt100_delete_chars(char cCount)
{
	unsigned char cBuf[SCREEN_COLS];
	unsigned char	c;
	vtlc_goto(vt100_cur_col + cCount, vt100_cur_row);
	vtlc_getchar();	// preseed
	for (c=vt100_cur_col + cCount; c < SCREEN_COLS;c++)
		cBuf[c] = vtlc_getchar();

	vtlc_goto(vt100_cur_col, vt100_cur_row);

	for (c = vt100_cur_col + cCount; c < SCREEN_COLS; c++)
	{
		vtlc_putchar(cBuf[c]);
	}
	for (c = 0; c < cCount; c++)
	{
		vtlc_putchar(' ');
		c++;
	}
 	vtlc_goto(vt100_cur_col, vt100_cur_row);
}

void vt100_refresh()
{
#ifdef VTMEM
	unsigned char c1, c2, c3;
	vtlc_goto(0,0);
	c3 = vt100_screen_firstrow + vt100_firstrow_affected;
	if (c3 >= SCREEN_ROWS)
		c3-=SCREEN_ROWS;
	for (c1 = vt100_firstrow_affected; c1 <= vt100_lastrow_affected; c1++)
	{
		for (c2 = 0; c2 < SCREEN_COLS; c2++)
		{
			vtlc_putchar(vt100_screen[c3][c2]);
		}
		if (++c3 >= SCREEN_ROWS)
			c3=0;
	}
	vtlc_goto(vt100_cur_col, vt100_cur_row);
#else

#ifdef DEBUG_SEQ
	unsigned char c = 0, d=0, e = 0;
	vtlc_goto(0, 0);
	for (c = vt100_last_few_start; ; c++)
	{
		if (c >= LAST_FEW_SIZE)
			c = 0;
		d = vt100_last_few[c];
		if (d < ' ')
		{
			switch (d)
			{
				case ESC:
					vtlc_puts("[ESC]");
					break;
				case BS:
					vtlc_puts("[BS]");
					break;
				case TAB:
					vtlc_puts("[TAB]");
					break;
				case CR:
					vtlc_puts("[CR]");
					break;
				case LF:
					vtlc_puts("[LF]");
					break;
				case FF:
					vtlc_puts("[FF]");
					break;
				default:
					vtlc_putchar('0');
					vtlc_putchar('x');
					break;
			}
		}
		else
			vtlc_putchar(d | 128);
		e++;
		if (e >= LAST_FEW_SIZE)
			break;
	}
	vtlc_putchar('|');
	vtlc_putchar('x');
	vtlc_putchar('x');
	vtlc_putchar('x');
	vtlc_putchar('x');
	vtlc_putchar('x');
	vtlc_putchar('x');
	vtlc_putchar('x');
#endif
	vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
}

void vt100_clear(unsigned char cDir)
{
#ifdef VTMEM
#else
	vtlc_clearscreen(vt100_cur_col, vt100_cur_row, cDir);
#endif
}

void vt100_set_character_attributes(unsigned char c)
{
	switch (c)
	{
		case 0: // normal
			vt100_attributes = 0;
			break;

			// ignore
		case 1:	// bold
		case 2: // faint
		case 3: // italics
		case 4: // underlined
		case 5: // blink
			break;

		case 7:	// inverse
			vt100_attributes = 0x80;
			break;
	}
}

void vt100_eraseinline(unsigned char cDir)
{
	unsigned char cStart = 0, cEnd = 0;
	unsigned char c=0, c2=0;
	switch (cDir)
	{
		case 0:	// to the right
			cStart = vt100_cur_col;
			cEnd = SCREEN_COLS;
			break;
		case 1:	// to the left
			cStart = 0;
			cEnd = vt100_cur_col + 1;
			break;
		case 2: // all
			cStart = 0;
			cEnd = SCREEN_COLS;
			break;
	}
#ifdef VTMEM
	c2 = vt100_cur_row+vt100_screen_firstrow;
	if (c2 > SCREEN_ROWS)
		c2-=SCREEN_ROWS;
	for (c = cStart; c < cEnd; c++)
	{
		vt100_screen[c2][c] = ' ';
	}
#else
	vtlc_goto(cStart, vt100_cur_row);
	for (c = cStart; c < cEnd; c++)
	{
		vtlc_putchar(' ');
	}
	vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
}

void vt100_mode2()	// e.g. recieved esc [
{
	signed char	csiParams[4] = {0,0,0,0};
	unsigned char	csiParamIndex=0;
	unsigned char c = 0;

	while (1)
	{
		while (uart_empty());
		c = uart_getc();
#ifdef DEBUG_SEQ
		vt100_last_few[vt100_last_few_start] = c;
		vt100_last_few_start++;
		if (vt100_last_few_start >= LAST_FEW_SIZE)
			vt100_last_few_start = 0;
#endif
		if (c >= '0' && c <= '9')	// parameter
		{
			csiParams[csiParamIndex] *=10;
			csiParams[csiParamIndex]+= (c - '0');
		}
		else if (c == ';')
		{
			csiParamIndex++;
		}
		else if (c == '?')	// dec specific, ignore, but process rest
		{
		}
		else	// final character
		{
			switch(c)
			{
				case 'A':	// cursor up
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_next_row(-csiParams[0], 0);
					break;
	
				case 'B':	// cursor down
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_next_row(csiParams[0], 0);
					break;
	
				case 'C':	// cursor forward
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_next_col(csiParams[0]);
					break;
	
				case 'D':	// cursor backward
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_next_col(-csiParams[0]);
					break;
	
				case 'E':	// cursor next line
					vt100_cur_col = 0;
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_next_row(csiParams[0], 0);
					break;
	
				case 'F':	// cursor preceding line
					vt100_cur_col = 0;
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_next_row(-csiParams[0], 0);
					break;
	
				case 'G':	// cursor absolute column
					if (csiParams[0] == 0)	// not specified
						vt100_cur_col = 0;
					else
						vt100_cur_col = csiParams[0] - 1;
#ifndef VTMEM
					vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
					break;
	
				case 'f':	// horizontal and vertical position
				case 'H':	// cursor position
					if (csiParams[0]==0)	// not specified, default to 1,1
						vt100_cur_row = 0;
					else
						vt100_cur_row = csiParams[0] - 1;
		
					if (csiParams[1]==0)
						vt100_cur_col = 0;
					else
						vt100_cur_col = csiParams[1] - 1;
#ifndef VTMEM
					vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
					break;
	
				case 'J':	//erase in display
					vt100_clear(csiParams[0]);
					break;
				case 'K':	// erase in line
					vt100_eraseinline(csiParams[0]);
					break;
				case 'm':	// character attributes
					vt100_set_character_attributes(csiParams[0]);
					break;
	
				case '@':	// insert chars
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_insert_chars(csiParams[0]);
					break;
	
				case 'P':	// delete char
					if (csiParams[0] == 0)
						csiParams[0] = 1;
					vt100_delete_chars(csiParams[0]);
					break;
	
				case 'd':	// line position absolute
					if (csiParams[0]==0)	// not specified, default to 1,1
						vt100_cur_row = 0;
					else
						vt100_cur_row = csiParams[0] - 1;
					break;
	
				case 'b':	// repeat previous char n times
					break;
#ifndef VTMEM
					vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
					break;
	
				case 'c':	// device attributes
					{
						unsigned char *p = "\x1b?1;0c";
						while (*p)
						{
							uart_putc(*p);
							p++;
						}
					}
					break;
				case 'h':	// set mode
					break;	// ignore

				case 'r':	// scrolling region
					break;

				default:
					vtlc_goto(0,0);
					vtlc_putchar(128 | '[');
					vtlc_putchar(128 | c);
					vtlc_goto(vt100_cur_col, vt100_cur_row);
					break;
/*
				case 'I':	//forward tab stops
				case 'L':	// insert lines
				case 'M':	// delete lines
				case 'N':	// noop
				case 'O':	// noop
				case 'S':	// scroll up
				case 'T':	// scroll down
				case 'X':	// erase chars
				case 'Z':	// cursor back tab stops
				case '`':	// cursor position absolute
				case 'a':	// cursor position absolute
				case 'e':	// line position relative
				case 'g':	// tab clear
				case 'i':	// media copy
				case 'l':	// reset mode
				case 'n':	// device status report
					break;*/
			}
			return;
		}
	}
}

void vt100_mode3()	// e.g. recieved esc P
{
	return;
}

// C1 characters
void vt100_mode1()	// e.g. recieved esc
{
	unsigned char c = 0;
	while (uart_empty());
	c = uart_getc();
#ifdef DEBUG_SEQ
		vt100_last_few[vt100_last_few_start] = c;
		vt100_last_few_start++;
		if (vt100_last_few_start >= LAST_FEW_SIZE)
			vt100_last_few_start = 0;
#endif
	switch (c)	// most common to least common
	{
		case '[':
			vt100_mode2();
			break;
		case 'E':	// next line
			vt100_next_row(1,0);
			break;
		case 'P':	// device control string
			vt100_mode3();
			break;
		case 'M':	// reverse index
			vt100_next_row(-1,0);
			break;
		case 'D':	// index
			vt100_next_row(1,0);
			break;

		default:
			vtlc_goto(0,0);
			vtlc_putchar(128 | c);
			vtlc_goto(vt100_cur_col, vt100_cur_row);
			break;

/*		case ' ':
		case '#':
		case '%':
		case '(':
		case ')':
		case '*':
		case '+':
		case '7':
		case '8':
		case '=':	// application keypad
		case '>':
		case 'F':	// 
		case 'H':	// tab set
		case 'N':	// single shift select G2
		case 'O':	// G3
		case 'V':
		case 'W':
		case 'X':
		case 'Z':
		case 'c':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case '|':
		case '}':
		case '~':
		case '\\':
		case ']':
		case '^':
		case '_':*/
	}
}

void	vt100_putchars()
{
	unsigned char c;
#ifdef VTMEM
	vt100_firstrow_affected = vt100_cur_row;
	vt100_lastrow_affected = vt100_cur_row;
#endif
	while (uart_available())
	{
		c = uart_getc();
#ifdef DEBUG_SEQ
		vt100_last_few[vt100_last_few_start] = c;
		vt100_last_few_start++;
		if (vt100_last_few_start >= LAST_FEW_SIZE)
			vt100_last_few_start = 0;
#endif
		if (c < ' ')
		{
			vt100_wrap_pending = 0;
			switch (c)
			{
				case BEL:
					break;
				case BS:	// backspace
					vt100_next_col(-1);
					break;
				case ESC:
					vt100_mode1();
					break;
				case TAB:
					vt100_next_tab(1);
					break;
				case CR:
					vt100_cur_col = 0;
#ifndef VTMEM
					vtlc_goto(vt100_cur_col, vt100_cur_row);
#endif
					break;
				case LF:	// all treated as a linefeed
				case FF:
				case VT:
					vt100_next_row(1, 1);
					break;
				case SI:
					break;
				default:
					break;
			}
		}
		else
		{
			if (vt100_wrap_pending)
			{
				vt100_cur_col = 0;
				vt100_next_row(1,1);
				vt100_wrap_pending = 0;
			}
#ifdef VTMEM
			c2 = vt100_cur_row + vt100_screen_firstrow;
			if (c2 > SCREEN_ROWS)
				c2-=SCREEN_ROWS;					
			vt100_screen[c2][vt100_cur_col] = c;
#else
			vtlc_putchar(vt100_attributes | c);
#endif

			// same as vt100_next_col() ish,, but no function for efficiency
			vt100_cur_col++;
			if (vt100_cur_col >= SCREEN_COLS)
			{
				vt100_cur_col=SCREEN_COLS-1;
				vt100_wrap_pending = 1;
			}
		}
	}
#ifdef VTMEM
	if (vt100_cur_row > vt100_lastrow_affected)
		vt100_lastrow_affected = vt100_cur_row;
	if (vt100_cur_row < vt100_firstrow_affected)
		vt100_firstrow_affected = vt100_cur_row;
#endif
}

void	vt100_init()
{
	unsigned char c;
	vt100_cur_row = 0;
	vt100_cur_col = 0;
	vt100_attributes = 0;
#ifdef DEBUG_SEQ
	memset(vt100_last_few, ' ', sizeof(vt100_last_few));
	vt100_last_few_start = 0;
#endif
	memset(vt100_tab_stops, 0, sizeof(vt100_tab_stops));
	for (c = 7; c < SCREEN_COLS; c+=8)
	{
		vt100_tab_stops[c] = 1;
	}
#ifdef VTMEM
	vt100_screen_firstrow = 0;
	vt100_firstrow_affected = 0;
	vt100_lastrow_affected = 0;
#endif
	vt100_clear(2);
}
