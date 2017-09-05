AVRemu

For now only a disassembler for
- ATmega48PA, ATmega88PA, ATmega168PA, ATmega328P
- ATtiny24A, ATtiny44A, ATtiny84A
- ATtiny25, ATtiny45, ATtiny85

usage: ./AVRemu -h
          this help
       ./AVRemu [-mcu <mcu>] <avr-bin>
          <mcu> is one of 'ATmega48PA', 'ATmega88PA', 'ATmega168PA', 'ATmega328P',
          'ATtiny24A', 'ATtiny44A', 'ATtiny84A', 'ATtiny25', 'ATtiny45', 'ATtiny85'
          <avr-bin> is the binary file
