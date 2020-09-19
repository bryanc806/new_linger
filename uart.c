#include <6502.h>
#include "uart.h"

unsigned char	uart_rxqueue[256];
//unsigned char	uart_txqueue[256];

#define	UART_CONTROL	0x9000
#define	UART_REGTXRX	0x9001

#define	UART_SET(a)	*(char *)UART_CONTROL = a
#define	UART_GET()	*(char *)UART_CONTROL
#define	UART_TXRX(a)	*(char *)UART_REGTXRX = a
void uart_init()
{
	UART_SET(0x17);	// master reset, 8n1, rts low, txint disabled, rxint disabled
	UART_SET(0x95);	// /16, 8n1, rts low, txint enabled, rxint enabled /16 == 115200
	// 10110101
}

unsigned char uart_getc()
{
	unsigned char c = uart_rxqueue[uart_rx_dequeue];
	++uart_rx_dequeue;
	if (uart_flow && uart_rx_dequeue == uart_rx_enqueue)  // e.g. the queue is empty
	{
		uart_putc(0x11);    // xon
		uart_flow = 0;
	}
	return c;
}

void uart_putc(unsigned char c)
{
#if 0
	SEI();
	if (uart_tx_enqueue == uart_tx_dequeue && (UART_GET() & 0x02))	// nothing queued and TXDE
	{
		UART_TXRX(c);
	}
	else
	{
		uart_txqueue[uart_tx_enqueue]=c;
		uart_tx_enqueue++;
	}
	CLI();
#else
	while (!(UART_GET() & 0x02));	// nothing queued and TXDE

	UART_TXRX(c);
#endif
}



