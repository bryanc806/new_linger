void vtlc_clearscreen(unsigned char col, unsigned char row, unsigned char cDir);
void vtlc_init();
void vtlc_goto(unsigned char col, unsigned char row);
void vtlc_putchar(unsigned char c);
unsigned char vtlc_getchar();
void vtlc_puts(const unsigned char *p);
void vtlc_scroll(signed char c);

void vtlc_beginupdate();
void vtlc_endupdate();
void vtlc_flush();
void vtlc_flush_all();

