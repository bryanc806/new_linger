; ---------------------------------------------------------------------------
; interrupt.s
; ---------------------------------------------------------------------------
;
; Interrupt handler.
;
; Checks for a BRK instruction and returns from all valid interrupts.

.export   _irq_int, _nmi_int, _kbd_byte, _via_state, _uart_rx_enqueue, _uart_rx_dequeue, _uart_tx_enqueue, _uart_tx_dequeue, _uart_flow, _clk_count

.import	_uart_rxqueue
.import	_uart_txqueue
	UART_CONTROL	:=	$9000
	UART_TXRX	:=	$9001
	VIA_IFR	:= $400d
	VIA_PORTB := $4000
	VIA_PORTA := $4001

.segment  "ZEROPAGE"
_uart_rx_enqueue:
	.res	1,$00
_uart_rx_dequeue:
	.res	1,$00
_uart_tx_enqueue:
	.res	1,$00
_uart_tx_dequeue:
	.res	1,$00
_uart_flow:
	.res	1,$00
_via_state:
	.res	1,$00
_kbd_byte:
	.res	1,$00
_kbd_rxbyte:
	.res	1,$00
_clk_count:
	.res	2,$00
.segment  "CODE"

.PC02                             ; Force 65C02 assembly mode

; ---------------------------------------------------------------------------
; Non-maskable interrupt (NMI) service routine

_nmi_int:  PHX
           PHA
           LDA  VIA_IFR
           STA  VIA_IFR			; reset flags
           PHA
           AND 	#$02
           BEQ _no_kbd
           LDA VIA_PORTA
           BPL _zero_bit
           LDA _kbd_rxbyte
           LSR A
           ORA #$80
		   JMP	_store_kbd
_zero_bit: LDA _kbd_rxbyte
           LSR A
_store_kbd:STA _kbd_rxbyte

           DEC _via_state
           BNE _no_full_byte

           STA _kbd_byte	; a still has the kbd_rxbyte, copy to _kbd_byte
           LDX #$0a	; prepare for next case
           STX _via_state

_no_full_byte:

_no_kbd:
           PLA
           AND 	#$10			; 30 hz timer
           BEQ _no_clock
           INC  _clk_count
           BNE	_clock_done
           INC  _clk_count+1

_clock_done:
_no_clock:
           PLA
           PLX
           RTI

; ---------------------------------------------------------------------------
; Maskable interrupt (IRQ) service routine

_irq_int:  PHX                    ; Save X register contents to stack
           PHA                    ; Save accumulator contents to stack
           LDA	UART_CONTROL
           AND	#$01
           BEQ	_no_rx
           LDX	_uart_rx_enqueue
           LDA	UART_TXRX
           STA	_uart_rxqueue,X
           INX
           STX	_uart_rx_enqueue
           LDA  _uart_flow
           BNE  _rx_restart			; already sent xoff
           TXA
           CLC
           ADC	#10					; 10 bytes from full
           CMP	_uart_rx_dequeue
           BPL  _rx_done
           LDA  #$13				; send xoff
           STA  UART_TXRX
           LDA	#1
           STA  _uart_flow
           JMP _rx_done

_rx_restart:
;           TXA                      ; X has _uart_rx_enqueue
;           CLC
;           ADC  #10				; 10 characters
;           CMP  _uart_rx_dequeue
;           BMI  _rx_done
;           LDA  #$11				; send xon
;           STA  UART_TXRX
;           LDA	#0
;           STA  _uart_flow
           
_rx_done:
_no_rx:
;           LDA	UART_CONTROL
;           AND	#$02			; txde
;           BEQ  _txfull
;           LDA  _uart_tx_dequeue
;           TXA
;           CMP  _uart_tx_enqueue
;           BEQ  _no_tx			; nothing to do
;           LDA  _uart_txqueue,X
;           STA  UART_TXRX
;           INX
;           STX  _uart_tx_dequeue
           
_no_tx:
_txfull:


; ---------------------------------------------------------------------------
; IRQ detected, return

irq:       PLA                    ; Restore accumulator contents
           PLX                    ; Restore X register contents
           RTI                    ; Return from all IRQ interrupts
