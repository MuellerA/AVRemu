# AVRemu

Disassembler / Emulator for
- ATmega48PA, ATmega88PA, ATmega168PA, ATmega328P
- ATmega8A
- ATtiny24A, ATtiny44A, ATtiny84A
- ATtiny25, ATtiny45, ATtiny85
- ATxmega128A4U, ATxmega64A4U, ATxmega32A4U, ATxmega16A4U,
- ATmega2560 (experimental)

The disassembler shows the MCU specific I/O register and interrupt vector names. Only those AVR instructions are used which are in the MCU's instruction set. (Exception is the generic ATany which supports all instructions but does not have any MCU knowlege.)
The twopass disassembler shows direct jump/call targets.
Modifications by Gilhad: LDS/STS show variable name if possible, ATmega2560

## Compile

```txt
cmake -B build.debug -G Ninja -DCMAKE_BUILD_TYPE=Debug .
cmake -B build.release -G Ninja -DCMAKE_BUILD_TYPE=Release .

cmake --build build.debug
cmake --build build.release
```

## Usage

```txt
usage: AVRemu [-d] [-e] [-m <mcu>] [-x <xref>] [-p <eeProm>] <avr-bin>
       AVRemu -h
parameter:
   -m <mcu>    MCU type, see below
   -d          disassemble file
   -e          execute file
   -ee <macro> run macro file <macro>.aem (implies -e)
   -x <xref>   xref file
   -p <eeProm> binary file of EEPROM memory
   <avr-bin>   binary file to be disassembled / executed
   -h          this help
Supported MCU types: ATany ATmega168PA ATmega328P ATmega48PA ATmega88PA ATmega8A ATtiny24A ATtiny25 ATtiny44A ATtiny45 ATtiny84A ATtiny85 ATxmega128A4U ATxmega16A4U ATxmega32A4U ATxmega64A4U
```

## XRef, Bin File

To extract the bin and xref files from an elf file use the script Elf.rb
```txt
usage: Elf.rb <elf-file(in)> <bin-file(out)> <xref-file(out)>
```

### XRef file format

```txt
X AAAA NNNN DDDD
- X     xref type: c: call; j: jump; d: data; r: ram
- AAAA  address (ram +0x00800000)
- NNNN  name
- DDDD  description
```

## Disassembler

### Example

```txt
AVRemu > ./build.release/AVRemu -d -m ATtiny85 -x ledLamp.attiny85.xref ledLamp.attiny85.bin

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
```

## Emulator

### Macros

Macros have the extension .aem (AvrEmuMacro). Macros are executed with the ```m <macro-file-name>``` command (without .aem extension). Macros are searched next to the binary file, in the directory ```~/.avremu/<mcu from -m parameter>``` and in the directory ```~/.avremu``` (later two linux only).
Commands in the macro file are executed in the same way as in the command line with two exceptions: the empty line does not repeat the previous command, and lines starting with a ```#``` are treated as comment.

### Filters (Linux only)

Filters are registered with the ```f + <mask> <os-command>``` command. A filter is a program that receives output lines like the ```v``` command. The filter then returns a line which will be displayed on the command line unless it is the empty line. The filter must respond with exactly one line for each received line.
A trivial filter would be ```/bin/cat```.

### IO input data

When setting input data for an io port, the next read operations will return the specified bytes. Corresponding ready status bits will be set accordingly.
The IO command is supported for ATxmega*::USART*_DATA and ATmegaXX8::UDRn ports.

### Example

```txt
AVRemu > ./build.release/AVRemu -e -m ATtiny85 -x ledLamp.attiny85.xref -p ledLamp.attiny85.eeprom  ledLamp.attiny85.bin

type "?" for help

RESET
External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset
00000:   ..     c00e          RJMP   RESET		; 14 0x0000f Relative Jump
> ?

<empty line>                  repeat last command
s [<count>]                   step in count instructions
n [<count>]                   step over count instructions
r                             run
r <label>                     run to address
rj                            run to next jump / branch
rc                            run to next call
rr                            run to next return
ra                            run to next jump / branch / call / return
g <label>                     set PC to address
b + <label>                   add breakpoint
b - <label>                   remove breakpoint
b ?                           list breakpoints
r ?                           read registers / useful in macros
d <addr> ? [<len>]            read memory content
d @ <X|Y|Z|SP|r<d>> ? [<len>] read memory content
p [<label>] ? [<len>]         list source
p @ <X|Y|Z|r<d>> ? [<len>]    list source
r<d>     = <bytes>            set register
d <addr> = <bytes>            set data memory
p <addr> = <words>            set program memory
sf ?                          list stack frames
ls [<pattern>]                list symbols containing <pattern>
io <name> = <bytes>           set next io read values (num)
io <name> = "<asc>"           set next io read values (str)
io ?                          list io port names
m <name>                      run macro file <name>.aem
mq                            quit macro execution
v io = <on|off>               verbose io on/off
v eeprom = <on|off>           verbose eeprom on/off
v data = <on|off>             verbose data error on/off
v prog = <on|off>             verbose program error on/off
v all = <on|off>              verbose all on/off
f + <io|eeprom|data|prog|all> <command> add filter for specified events
f ?                           list active filters
t on <name> [<addr>]          log to trace file until addr is reached (default 0x00000)
t off                         close trace file
$ <text>                      write text to output / useful in macros
q                             quit
h                             help
?                             help
<label> symbol or hex or dec address
<addr>  hex or dec address
<count> hex or dec number
<len>   hex or dec number
<d>     dec number 0 to 31
<bytes> list of hex or dec bytes
<words> list of hex or dec words

RESET
External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset
00000:   ..     c00e          RJMP   RESET		; 14 0x0000f Relative Jump
> s
       ________  00 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
RESET: RESET
0000f:   ..     e000          LDI    r16, 0x00		; 0 Load Immediate
>
       ________  00 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
00010:   ..     b903          OUT    ADCSRB, r16		; 0x03 Store Register to I/O Location
>
       ________  00 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
> r0=0xab
       ________  ab 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
> b + Main
00011:   ..     b904          OUT    ADCL, r16		; 0x04 Store Register to I/O Location
> r
       ________  ab 00 00 00 00 00 00 00
       SP: 025f  00 00 00 00 00 00 00 00
                 00 02 00 00 00 00 00 00
                 00 00 00 00 00 00 00 00
Main: 00035
00195:   ..     9ab8          SBI    DDRB, 0		; 0x17 Set Bit in I/O Register
> p ? 10
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
> q
```


## Modifications by Gilhad:

LDS/STS show variable name if possible

```txt
03caf:   ....   9180 1095     LDS    r24, IP        ; 0x1095 Load Direct from Data Space
03cb1:   ....   9190 1096     LDS    r25, IP+1      ; 0x1096 Load Direct from Data Space
03cb3:   ....   91a0 1097     LDS    r26, IP+2      ; 0x1097 Load Direct from Data Space
03cb5:   ....   91b0 1098     LDS    r27, IP+3      ; 0x1098 Load Direct from Data Space
...
03cbb:   ....   9380 1095     STS    IP, r24        ; 0x1095 Store Direct to Data Space
03cbd:   ....   9390 1096     STS    IP+1, r25      ; 0x1096 Store Direct to Data Space
03cbf:   ....   93a0 1097     STS    IP+2, r26      ; 0x1097 Store Direct to Data Space
03cc1:   ....   93b0 1098     STS    IP+3, r27      ; 0x1098 Store Direct to Data Space
```

Support for ATmega2560 (experimental)
Aliases: atmega328 -> ATmega328P, atmega2560 -> ATmega2560

