OPTFLAGS=-Osir
cc65 -t none $OPTFLAGS --cpu 65c02 main.c
ca65 --cpu 65sc02 main.s
cc65 -t none $OPTFLAGS --cpu 65c02 vtlc.c
ca65 --cpu 65sc02 vtlc.s
cc65 -t none $OPTFLAGS --cpu 65c02 via.c
ca65 --cpu 65sc02 via.s
cc65 -t none $OPTFLAGS --cpu 65c02 uart.c
ca65 --cpu 65sc02 uart.s
cc65 -t none $OPTFLAGS --cpu 65c02 vt100.c
ca65 --cpu 65sc02 vt100.s
cc65 -t none $OPTFLAGS --cpu 65c02 menu.c
ca65 --cpu 65sc02 menu.s
cc65 -t none $OPTFLAGS --cpu 65c02 keyboard_xt.c
ca65 --cpu 65sc02 keyboard_xt.s
ca65 crt0.s
#ca65 --cpu 65sc02 rs232_tx.s
ca65 --cpu 65sc02 interrupt.s
ca65 --cpu 65sc02 vectors.s
ld65 -C sbc.cfg -m main.map vectors.o main.o vtlc.o uart.o via.o vt100.o interrupt.o crt0.o keyboard_xt.o menu.o sbc.lib
