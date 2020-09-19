#include "keyboard_xt.h"
#include "menu.h"

#pragma bss-name (push,"EXTZP")
extern unsigned char kbd_byte;
#pragma zpsym("kbd_byte")
#pragma bss-name (pop)
unsigned char last_kbd_byte = 0xff;

static const unsigned char *kbd_returns[] =
{
	"", "\x1b",	"1",	"2",	"3",	"4",	"5",	"6",	"7",	"8",	"9",	"0",	"-",	"=",	"\x7f",	// 00 thru 0E
	"\x09",	"q",	"w",	"e",	"r",	"t",	"y",	"u",	"i",	"o",	"p",	"[",	"]",	"\x0d", // 0F thru 1C
	"\x80",	"a",	"s",	"d",	"f",	"g",	"h",	"j",	"k",	"l",	";",	"'",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"\\",	"z",	"x",	"c",	"v",	"b",	"n",	"m",	",",	".",	"/",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};
static const unsigned char *kbd_returns_shift[] =
{
	"", "\x1b",	"!",	"@",	"#",	"$",	"%",	"^",	"&",	"*",	"(",	")",	"_",	"+",	"\x7f",	// 00 thru 0E
	"\x09",	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\x0d", // 0F thru 1C
	"\x80",	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"|",	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char *kbd_returns_ctrl[] =	//tbd
{
	"", "\x1b",	"1",	"\x00",	"\x1b",	"\x1c",	"\x1d",	"\x1e", "\x1f",	"\x7f",	"(",	"",	"",	"=",	"\x7f",	// 00 thru 0E
	"\x09",	"\x11",	"\x17",	"\x05",	"\x12",	"\x14",	"\x19",	"\x15",	"\x09",	"\x0f",	"\x10",	"\x1b",	"\x1e",	"\x0d", // 0F thru 1C
	"\x80",	"\x01",	"\x13",	"\x04",	"\x06",	"\x07",	"\x08",	"\x0a",	"\x0b",	"\x0c",	";",	"'",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"\x1c",	"\x1a",	"\x18",	"\x03",	"\x16",	"\x02",	"\x0e",	"\x0d",	",",	".",	"\x1f",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	"\x00",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char *kbd_returns_shift_ctrl[] =	//tbd
{
	"", "\x1b",	"!",	"@",	"#",	"$",	"%",	"^",	"&",	"*",	"(",	")",	"_",	"+",	"\x7f",	// 00 thru 0E
	"\x09",	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\x0d", // 0F thru 1C
	"\x80",	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"|",	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char *kbd_returns_alt[] =
{
	"", "\x1b",	"\x87",	"@",	"#",	"$",	"%",	"^",	"&",	"*",	"(",	")",	"_",	"+",	"\x7f",	// 00 thru 0E
	"\x09",	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\x0d", // 0F thru 1C
	"\x80",	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"|",	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char *kbd_returns_shift_alt [] =
{
	"", "\x1b",	"!",	"@",	"#",	"$",	"%",	"^",	"&",	"*",	"(",	")",	"_",	"+",	"\x7f",	// 00 thru 0E
	"\x09",	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\x0d", // 0F thru 1C
	"\x80",	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"|",	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char *kbd_returns_ctrl_alt[] =
{
	"", "\x1b",	"!",	"@",	"#",	"$",	"%",	"^",	"&",	"*",	"(",	")",	"_",	"+",	"\x7f",	// 00 thru 0E
	"\x09",	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\x0d", // 0F thru 1C
	"\x80",	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"|",	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char *kbd_returns_shift_ctrl_alt[] =
{
	"", "\x1b",	"!",	"@",	"#",	"$",	"%",	"^",	"&",	"*",	"(",	")",	"_",	"+",	"\x7f",	// 00 thru 0E
	"\x09",	"Q",	"W",	"E",	"R",	"T",	"Y",	"U",	"I",	"O",	"P",	"{",	"}",	"\x0d", // 0F thru 1C
	"\x80",	"A",	"S",	"D",	"F",	"G",	"H",	"J",	"K",	"L",	":",	"\"",	"#",	// 1d thru 29 (ctrl)
	"\x81",	"|",	"Z",	"X",	"C",	"V",	"B",	"N",	"M",	"<",	">",	"?",	"\x81",	"\x82",	// 2a thru 37 (shift, shift, prnt screen)
	"\x83",	" ",	"\x84",	"\x1b[P",	"\x1b[Q",	"\x1b[R",	"\x1b[S",	"\x1b[15~",	"\x1b[17~",	"\x1b[18~",	"\x1b[19~",	"\x1b[20~",	"\x1b[21~",	// 38 thru 44 (alt, caps, f1-10)
	"\x85",	"\x86",	"\x1b[H",	"\x1b[A",	"\x1b[5~",	"-",	"\x1b[D",	"\x1b[E",	"\x1b[C",	"+",	"\x1b[F",	"\x1b[B",	"\x1b[6~",	// 45 thru 51
	"\x1b[2~",	"\x1b[3~",	"",		"",		"",		"\x1b[23~",		"\x1b[24~",		// 52 thru 58 (F11, F12)
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 59 thru 68
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,		// 69 thru 78
	0,	0,	0,	0,	0,	0,	0,	0		// 78 thru 7f
};

static const unsigned char **kbd_returns_modifiers[] = {kbd_returns, kbd_returns_shift, kbd_returns_ctrl, kbd_returns_shift_ctrl, kbd_returns_alt, kbd_returns_shift_alt, kbd_returns_ctrl_alt, kbd_returns_shift_ctrl_alt };

#define	KEY_CTRL	0x80
#define	KEY_SHIFT	0x81
#define	KEY_PRNTSCRN	0x82
#define	KEY_ALT		0x83
#define	KEY_CAPS	0x84
#define	KEY_NUMLOCK	0x85
#define	KEY_SCROLLLOCK	0x86
#define	KEY_MENU	0x87


unsigned char	kbd_modifiers = 0;

const unsigned char *keyboard_get()
{
	const unsigned char *p = 0;
	if (kbd_byte == last_kbd_byte)
		return 0;
	last_kbd_byte = kbd_byte;
	p = kbd_returns_modifiers[kbd_modifiers][last_kbd_byte & 0x7f];
	if (!p)
		return 0;
	if (*p & 0x80)	// special function
	{
		if (last_kbd_byte & 0x80)	// clear
		{
			switch (*p)
			{
				case KEY_SHIFT:
					kbd_modifiers &= ~0x01;
					break;
				case KEY_CTRL:
					kbd_modifiers &= ~0x02;
					break;
				case KEY_ALT:
					kbd_modifiers &= ~0x04;
					break;
			}
		}
		else
		{
			switch (*p)
			{
				case KEY_SHIFT:
					kbd_modifiers |= 0x01;
					break;
				case KEY_CTRL:
					kbd_modifiers |= 0x02;
					break;
				case KEY_ALT:
					kbd_modifiers |= 0x04;
					break;
				case KEY_MENU:
					menu();
					break;
			}
		}
		return 0;
	}
	else
	{
		if (last_kbd_byte & 0x80)	// key release
			return 0;
		else
			return p;
	}
}
