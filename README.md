AVRemu

For now only a disassembler for
- ATmega48PA
- ATmega88PA
- ATmega168PA
- ATmega328P

Usage: 
<pre>
./AVRemu [-mcu &lt;mcu&gt;] &lt;avr-bin&gt;
         &lt;mcu&gt; is one of 'ATmega48PA', 'ATmega88PA', 'ATmega168PA', 'ATmega328P'
         &lt;avr-bin&gt; is the binary file to disassemble
./AVRemu -h
         this help
</pre>

Compile:
<pre>
make -k
</pre>
