#pragma bss-name (push,"EXTZP")
extern unsigned char	uart_rx_enqueue;
extern unsigned char	uart_rx_dequeue;
extern unsigned char	uart_tx_enqueue;
extern unsigned char	uart_tx_dequeue;
extern unsigned char	uart_flow;
#pragma zpsym("uart_rx_enqueue");
#pragma zpsym("uart_rx_dequeue");
#pragma zpsym("uart_tx_enqueue");
#pragma zpsym("uart_tx_dequeue");
#pragma zpsym("uart_flow");
#pragma bss-name (pop)

void uart_init();
unsigned char uart_getc();
void uart_putc(unsigned char c);
#define uart_empty() (uart_rx_enqueue == uart_rx_dequeue)
#define uart_available() (uart_rx_enqueue != uart_rx_dequeue)

