AVRemu

For now only a disassembler on Linux for
- ATmega48PA, ATmega88PA, ATmega168PA, ATmega328P
- ATtiny24A, ATtiny44A, ATtiny84A
- ATtiny25, ATtiny45, ATtiny85
- ATxmega128A4U, ATxmega64A4U, ATxmega32A4U, ATxmega16A4U,

Usage: 
<pre>
usage: AVRemu [-mcu &lt;mcu&gt;] &lt;avr-bin&gt;
       AVRemu -h
parameter:
   -mcu &lt;mcu&gt;  MCU type, see below
   &lt;avr-bin&gt;   binary file to be disassembled
   -h          this help
Supported MCU types: ATany ATmega168PA ATmega328P ATmega48PA ATmega88PA ATtiny24A ATtiny25 ATtiny44A ATtiny45 ATtiny84A ATtiny85 ATxmega128A4U ATxmega16A4U ATxmega32A4U ATxmega64A4U
</pre>

Compile:
<pre>
cd source
make -k
</pre>

Example:
<pre>
AVRemu/source > ./AVRemu -mcu ATtiny45 ../test/attiny45.bin 
External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset:
00000:   ..     c00e          RJMP   14		; 0x0000f Relative Jump
External Interrupt Request 0:
00001:   @.     c040          RJMP   64		; 0x00042 Relative Jump
Pin Change Interrupt Request 0:
00002:   ..     9518          RETI  		; Return from Interrupt
Timer/Counter1 Compare Match A:
00003:   U.     c055          RJMP   85		; 0x00059 Relative Jump
Timer/Counter1 Overflow:
00004:   ..     9518          RETI  		; Return from Interrupt
Timer/Counter0 Overflow:
00005:   ..     9518          RETI  		; Return from Interrupt
EEPROM Ready:
00006:   ..     9518          RETI  		; Return from Interrupt
Analog Comparator:
00007:   ..     9518          RETI  		; Return from Interrupt
ADC Conversion Complete:
00008:   ..     9518          RETI  		; Return from Interrupt
Timer/Counter1 Compare Match B:
00009:   ..     9518          RETI  		; Return from Interrupt
Timer/Counter0 Compare Match A:
0000a:   ..     9518          RETI  		; Return from Interrupt
Timer/Counter0 Compare Match B:
0000b:   ..     9518          RETI  		; Return from Interrupt
Watchdog Time-out:
0000c:   ..     9518          RETI  		; Return from Interrupt
USI START:
0000d:   ..     9518          RETI  		; Return from Interrupt
USI Overflow:
0000e:   ..     9518          RETI  		; Return from Interrupt
0000f:   ..     e50f          LDI    r16, 0x5f		; 95 Load Immediate
00010:   ..     bf0d          OUT    0x3d, r16		; Store Register to I/O Location
00011:   ..     e001          LDI    r16, 0x01		; 1 Load Immediate
00012:   ..     bf0e          OUT    0x3e, r16		; Store Register to I/O Location
00013:   ..     e010          LDI    r17, 0x00		; 0 Load Immediate
00014:   ..     e400          LDI    r16, 0x40		; 64 Load Immediate
00015:   ..     bf0b          OUT    0x3b, r16		; Store Register to I/O Location
00016:   ..     e800          LDI    r16, 0x80		; 128 Load Immediate
00017:   ..     bd06          OUT    0x26, r16		; Store Register to I/O Location
00018:   ..     e003          LDI    r16, 0x03		; 3 Load Immediate
00019:   ..     bd06          OUT    0x26, r16		; Store Register to I/O Location
0001a:   ..     e402          LDI    r16, 0x42		; 66 Load Immediate
0001b:   ..     bd0a          OUT    0x2a, r16		; Store Register to I/O Location
0001c:   ..     bf13          OUT    0x33, r17		; Store Register to I/O Location
0001d:   ..     e001          LDI    r16, 0x01		; 1 Load Immediate
0001e:   ..     bf00          OUT    0x30, r16		; Store Register to I/O Location
0001f:   ..     ef0a          LDI    r16, 0xfa		; 250 Load Immediate
00020:   ..     bd0e          OUT    0x2e, r16		; Store Register to I/O Location
00021:   ..     e400          LDI    r16, 0x40		; 64 Load Immediate
00022:   ..     bf09          OUT    0x39, r16		; Store Register to I/O Location
00023:   ..     bb17          OUT    0x17, r17		; Store Register to I/O Location
00024:   ..     bb18          OUT    0x18, r17		; Store Register to I/O Location
00025:   ..     9ab8          SBI    0x17, 0		; Set Bit in I/O Register
00026:   ..     9abc          SBI    0x17, 4		; Set Bit in I/O Register
00027:   ..     9ac2          SBI    0x18, 2		; Set Bit in I/O Register
00028:   ..     bb15          OUT    0x15, r17		; Store Register to I/O Location
00029:   ..     e003          LDI    r16, 0x03		; 3 Load Immediate
0002a:   ..     bd00          OUT    0x20, r16		; Store Register to I/O Location
</pre>
