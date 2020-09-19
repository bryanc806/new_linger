/*
Register        Designation             Function
--------        -----------             --------------------------------------
  0             ORA or IRB              Output or input register B
  1             ORA or IRA              Output or input register A
  2             DDRB                    Data direction register B
  3             DDRA                    Data direction register A
  4             T1C-L                   T1 low-byte latch or T1 low-byte
                                        counter
  5             T1C-H                   T1 high-byte counter
  6             T1L-L                   T1 low-byte latch
  7             T1L-H                   T1 high-byte latch
  8             T2C-L                   T2 low-byte latch or T2 low-byte
                                        counter
  9             T2C-H                   T2 high-byte counter
 10             SR                      Shift Register
 11             ACR                     Auxiliary Control Register
 12             PCR                     Peripheral Control Register
 13             IFR                     Interrupt Flag Register
 14             IER                     Interrupt Enable Register
 15             ORA or IRA              Identical to register 1 but no handshake
*/
#define	VIA_PORTB	0x4000
#define	VIA_PORTA	0x4001
#define	VIA_DDRB	0x4002
#define	VIA_DDRA	0x4003
#define	VIA_T1CL	0x4004
#define	VIA_T1CH	0x4005
#define	VIA_T1LL	0x4006
#define	VIA_T1LH	0x4007
#define	VIA_T2CL	0x4008
#define	VIA_T2CH	0x4009
#define	VIA_SR		0x400a
#define	VIA_ACR		0x400b
#define	VIA_PCR		0x400c
#define	VIA_IFR		0x400d
#define	VIA_IER		0x400e
#define	VIA_PORTA_NHS	0x400f

#define	VIA_WRITE(a,b)	*(unsigned char *)a = b
void via_init();
void via_setdivider(unsigned short);
extern unsigned char	via_nmi;
void	_delay_ms(unsigned short s);

