AVRemu

Disassembler / Emulator on Linux for
- ATmega48PA, ATmega88PA, ATmega168PA, ATmega328P
- ATmega8A
- ATtiny24A, ATtiny44A, ATtiny84A
- ATtiny25, ATtiny45, ATtiny85
- ATxmega128A4U, ATxmega64A4U, ATxmega32A4U, ATxmega16A4U,

The disassembler shows the MCU specific I/O register and interrupt vector names. Only those AVR instructions are used which are in the MCU's instruction set. (Exception is the generic ATany which supports all instructions but does not have any MCU knowlege.)
The twopass disassembler shows direct jump/call targets.

<hr/>
                                    
Compile:
<pre>
cd source
make -k
</pre>

<hr/>

Usage: 
<pre>
usage: /ei/home/am/c/AVRemu/source/AVRemu [-d] [-e] [-m &lt;mcu&gt;] [-x &lt;xref&gt;] [-p &lt;eeProm&gt;] &lt;avr-bin&gt;
       /ei/home/am/c/AVRemu/source/AVRemu -h
parameter:
   -m &lt;mcu&gt;    MCU type, see below
   -d          disassemble file
   -e          execute file
   -ee &lt;macro&gt; run macro file &lt;macro&gt;.aem (implies -e)
   -x &lt;xref&gt;   read/write xref file
   -p &lt;eeProm&gt; binary file of EEPROM memory
   &lt;avr-bin&gt;   binary file to be disassembled / executed
   -h          this help
Supported MCU types: ATany ATmega168PA ATmega328P ATmega48PA ATmega88PA ATmega8A ATtiny24A ATtiny25 ATtiny44A ATtiny45 ATtiny84A ATtiny85 ATxmega128A4U ATxmega16A4U ATxmega32A4U ATxmega64A4U
</pre>

To extract the bin and xref files from an elf file use the script Elf.rb
<pre>
usage: Elf.rb &lt;elf-file(in)&gt; &lt;bin-file(out)&gt; &lt;xref-file(out)&gt;
</pre>

<hr/>

Example Disassembler:
<pre>
AVRemu/source &gt; ./AVRemu -d -m ATtiny85 -x attiny85.xref attiny85.bin

RESET
External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset
00000:   ..     c00e          RJMP   RESET		; 14 0x0000f Relative Jump
IRQ_INT0
External Interrupt Request 0
00001:   ..     c0b4          RJMP   ISR_INT0		; 180 0x000b6 Relative Jump
IRQ_PCINT0
Pin Change Interrupt Request 0
00002:   ..     9518          RETI  		; Return from Interrupt
IRQ_TIMER1_COMPA
Timer/Counter1 Compare Match A
00003:   ..     9518          RETI  		; Return from Interrupt
IRQ_TIMER1_OVF
Timer/Counter1 Overflow
00004:   ..     9518          RETI  		; Return from Interrupt
IRQ_TIMER0_OVF
Timer/Counter0 Overflow
00005:   ..     9518          RETI  		; Return from Interrupt
IRQ_EE_RDY
EEPROM Ready
00006:   ..     9518          RETI  		; Return from Interrupt
IRQ_ANA_COMP
Analog Comparator
00007:   ..     9518          RETI  		; Return from Interrupt
IRQ_ADC
ADC Conversion Complete
00008:   ..     9518          RETI  		; Return from Interrupt
IRQ_TIMER1_COMPB
Timer/Counter1 Compare Match B
00009:   ..     9518          RETI  		; Return from Interrupt
IRQ_TIMER0_COMPA
Timer/Counter0 Compare Match A
0000a:   B.     c142          RJMP   ISR_TIMER0		; 322 0x0014d Relative Jump
IRQ_TIMER0_COMPB
Timer/Counter0 Compare Match B
0000b:   ..     9518          RETI  		; Return from Interrupt
IRQ_WDT
Watchdog Time-out
0000c:   ..     9518          RETI  		; Return from Interrupt
IRQ_USI_START
USI START
0000d:   ..     9518          RETI  		; Return from Interrupt
IRQ_USI_OVF
USI Overflow
0000e:   ..     9518          RETI  		; Return from Interrupt
RESET: RESET
0000f:   ..     e000          LDI    r16, 0x00		; 0 Load Immediate
00010:   ..     b903          OUT    ADCSRB, r16		; 0x03 Store Register to I/O Location
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
00012:   ..     b905          OUT    ADCH, r16		; 0x05 Store Register to I/O Location
00013:   ..     b906          OUT    ADCSRA, r16		; 0x06 Store Register to I/O Location
00014:   ..     b907          OUT    ADMUX, r16		; 0x07 Store Register to I/O Location
00015:   ..     b90d          OUT    USICR, r16		; 0x0d Store Register to I/O Location
00016:   ..     b90e          OUT    USISR, r16		; 0x0e Store Register to I/O Location
00017:   ..     bb04          OUT    DIDR0, r16		; 0x14 Store Register to I/O Location
00018:   ..     bb07          OUT    DDRB, r16		; 0x17 Store Register to I/O Location
00019:   ..     bb08          OUT    PORTB, r16		; 0x18 Store Register to I/O Location
0001a:   ..     e01b          LDI    r17, 0x0b		; 11 Load Immediate
0001b:   ..     bd10          OUT    PRR, r17		; 0x20 Store Register to I/O Location
0001c:   ..     bd08          OUT    OCR0B, r16		; 0x28 Store Register to I/O Location
0001d:   ..     e313          LDI    r17, 0x33		; 51 Load Immediate
0001e:   ..     bd19          OUT    OCR0A, r17		; 0x29 Store Register to I/O Location
0001f:   ..     e012          LDI    r17, 0x02		; 2 Load Immediate
00020:   ..     bd1a          OUT    TCCR0A, r17		; 0x2a Store Register to I/O Location
00021:   ..     bd0f          OUT    TCNT1, r16		; 0x2f Store Register to I/O Location
00022:   ..     bf00          OUT    TCCR1, r16		; 0x30 Store Register to I/O Location
00023:   ..     bf02          OUT    TCNT0, r16		; 0x32 Store Register to I/O Location
00024:   ..     bf13          OUT    TCCR0B, r17		; 0x33 Store Register to I/O Location
00025:   ..     e211          LDI    r17, 0x21		; 33 Load Immediate
00026:   ..     bf15          OUT    MCUCR, r17		; 0x35 Store Register to I/O Location
00027:   ..     bf08          OUT    TIFR, r16		; 0x38 Store Register to I/O Location
</pre>

<hr/>

Example Emulator:

<pre>
AVRemu/source &gt; ./AVRemu -e -m ATtiny85 -x attiny85.xref -p ledLamp.attiny85.eeprom  ledLamp.attiny85.bin 

type "?" for help

RESET
External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset
00000:   ..     c00e          RJMP   RESET		; 14 0x0000f Relative Jump
&gt; ?

&lt;empty line&gt;                  repeat last command
s [&lt;count&gt;]                   step in count instructions
n [&lt;count&gt;]                   step over count instructions
r                             run
r &lt;label&gt;                     run to address
g &lt;label&gt;                     set PC to address
b + &lt;label&gt;                   add breakpoint
b - &lt;label&gt;                   remove breakpoint
b ?                           list breakpoints
r ?                           read registers / useful in macros
d &lt;addr&gt; ? [&lt;len&gt;]            read memory content
d @ &lt;X|Y|Z|SP|r&lt;d&gt;&gt; ? [&lt;len&gt;] read memory content
p [&lt;label&gt;] ? [&lt;len&gt;]         list source
p @ &lt;X|Y|Z|r&lt;d&gt;&gt; ? [&lt;len&gt;]    list source
r&lt;d&gt;     = &lt;bytes&gt;            set register
d &lt;addr&gt; = &lt;bytes&gt;            set data memory
p &lt;addr&gt; = &lt;words&gt;            set program memory
ls [&lt;pattern&gt;]                list symbols containing &lt;pattern&gt;
io &lt;name&gt; = &lt;bytes&gt;           set next io read values (num)
io &lt;name&gt; = "&lt;asc&gt;"           set next io read values (str)
io ?                          list io port names
m &lt;name&gt;                      run macro file &lt;name&gt;.aem
mq                            quit macro execution
t on &lt;name&gt; [&lt;addr&gt;]          log to trace file until addr is reached (default 0x00000)
t off                         close trace file
$ &lt;text&gt;                      write text to output / useful in macros
q                             quit
h                             help
?                             help
&lt;label&gt; symbol or hex or dec address
&lt;addr&gt;  hex or dec address
&lt;count&gt; hex or dec number
&lt;len&gt;   hex or dec number
&lt;d&gt;     dec number 0 to 31
&lt;bytes&gt; list of hex or dec bytes
&lt;words&gt; list of hex or dec words

RESET
External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset
00000:   ..     c00e          RJMP   RESET		; 14 0x0000f Relative Jump
&gt; s
       ________  00 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
RESET: RESET
0000f:   ..     e000          LDI    r16, 0x00		; 0 Load Immediate
&gt; 
       ________  00 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
00010:   ..     b903          OUT    ADCSRB, r16		; 0x03 Store Register to I/O Location
&gt; 
       ________  00 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
&gt; r0=0xab
       ________  ab 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
&gt; b + Main
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
&gt; r
       ________  ab 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 02 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
Main: 00035
00195:   ..     9ab8          SBI    DDRB, 0		; 0x17 Set Bit in I/O Register
&gt; p ? 10
Main: 00035
00195:   ..     9ab8          SBI    DDRB, 0		; 0x17 Set Bit in I/O Register
00196:   ..     9ab9          SBI    DDRB, 1		; 0x17 Set Bit in I/O Register
00197:   ..     98c1          CBI    PORTB, 1		; 0x18 Clear Bit in I/O Register
00198:   .$     2411          EOR    r1, r1		; Exclusive OR
00199:   ..     e6c9          LDI    r28, 0x69		; 105 Load Immediate
0019a:   ..     e0d0          LDI    r29, 0x00		; 0 Load Immediate
0019b:   ..     01ce          MOVW   r24, r28		; Copy Register Word
0019c:   ..     d116          RCALL  ParamLoad		; 278 0x002b3 Relative Call to Subroutine
0019d:   ..     01ce          MOVW   r24, r28		; Copy Register Word
0019e:   \.     d15c          RCALL  ParamFix		; 348 0x002fb Relative Call to Subroutine

Main: 00035
00195:   ..     9ab8          SBI    DDRB, 0		; 0x17 Set Bit in I/O Register
&gt; q
</pre>
