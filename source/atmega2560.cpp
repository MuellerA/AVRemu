////////////////////////////////////////////////////////////////////////////////
// atmegaXX8.cpp
// ATmega48A/PA/88A/PA/168A/PA/328/P
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega2560::ATmega2560() :  Mcu("ATmega2560", 0x40000/2, 0x200,  0x2000, 0x0400, 0x2000 + 0xff),
  ioEeprom(*this), _usart0(), _usart1(), _usart2(), _usart3()
  {
    _offset = 0x20;
    /*
    const Instruction *instructions[] { &instrEIJMP, &instrJMP, &instrEICALL, &instrCALL, 
    &instrELPM1, &instrELPM2, &instrELPM3, 
    } ;
    */
    const Instruction *instructions[]
    {
      &instrADD, &instrADC, &instrADIW, &instrSUB, &instrSUBI, &instrSBC, &instrSBCI, &instrSBIW, &instrAND, &instrANDI,
      &instrOR, &instrORI, &instrEOR, &instrCOM, &instrNEG, &instrINC, &instrDEC, &instrMUL, &instrMULS, &instrMULSU,
      &instrFMUL, &instrFMULS, &instrFMULSU, /*&instrDES,*/

      &instrRJMP, &instrIJMP, &instrEIJMP, &instrJMP, &instrRCALL, &instrICALL, &instrEICALL, &instrCALL, &instrRET,
      &instrRETI, &instrCPSE, &instrCP, &instrCPC, &instrCPI, &instrSBRC, &instrSBRS, &instrSBIC, &instrSBIS,
      &instrBRBS, &instrBRBC,

      &instrMOV, &instrMOVW, &instrLDI, &instrLDS, &instrLDx1, &instrLDx2, &instrLDx3, &instrLDy1, &instrLDy2,
      &instrLDy3, &instrLDy4, &instrLDz1, &instrLDz2, &instrLDz3, &instrLDz4, &instrSTS, &instrSTx1, &instrSTx2,
      &instrSTx3, &instrSTy1, &instrSTy2, &instrSTy3, &instrSTy4, &instrSTz1, &instrSTz2, &instrSTz3, &instrSTz4,
      &instrLPM1, &instrLPM2, &instrLPM3, &instrELPM1, &instrELPM2, &instrELPM3, &instrSPM1, /*&instrSPM2,*/ &instrIN,
      &instrOUT, &instrPUSH, &instrPOP, /*&instrXCH,*/ /*&instrLAS,*/ /*&instrLAC,*/ /*&instrLAT,*/

      &instrLSR, &instrROR, &instrASR, &instrSWAP, &instrBSET, &instrBCLR, &instrSBI, &instrCBI, &instrBST,
      &instrBLD,

      &instrBREAK, &instrNOP, &instrSLEEP, &instrWDR,
    } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    std::vector<std::pair<uint32_t, Io::Register*>> ioRegs
    {
      { 0x136, new IoUsart::UDRn(*this, _usart3) },
      { 0x135, new IoRegisterNotImplemented(*this, "UBRR3H") },
      { 0x134, new IoRegisterNotImplemented(*this, "UBRR3L") },
      { 0x132, new IoRegisterNotImplemented(*this, "UCSR3C") },
      { 0x131, new IoRegisterNotImplemented(*this, "UCSR3B") },
      { 0x130, new IoUsart::UCSRnA(*this, _usart3) },

      { 0x12D, new IoRegisterNotImplemented(*this, "OCR5BH") },
      { 0x12C, new IoRegisterNotImplemented(*this, "OCR5CL") },
      { 0x12B, new IoRegisterNotImplemented(*this, "OCR5BH") },
      { 0x12A, new IoRegisterNotImplemented(*this, "OCR5BL") },
      { 0x129, new IoRegisterNotImplemented(*this, "OCR5AH") },
      { 0x128, new IoRegisterNotImplemented(*this, "OCR5AL") },
      { 0x127, new IoRegisterNotImplemented(*this, "ICR5H") },
      { 0x126, new IoRegisterNotImplemented(*this, "ICR5L") },
      { 0x125, new IoRegisterNotImplemented(*this, "TCNT5H") },
      { 0x124, new IoRegisterNotImplemented(*this, "TCNT5L") },
      { 0x122, new IoRegisterNotImplemented(*this, "TCCR5C") },
      { 0x121, new IoRegisterNotImplemented(*this, "TCCR5B") },
      { 0x120, new IoRegisterNotImplemented(*this, "TCCR5A") },

      { 0x10B, new IoRegisterNotImplemented(*this, "PORTL") },
      { 0x10A, new IoRegisterNotImplemented(*this, "DDRL") },
      { 0x109, new IoRegisterNotImplemented(*this, "PINL") },
      { 0x108, new IoRegisterNotImplemented(*this, "PORTK") },
      { 0x107, new IoRegisterNotImplemented(*this, "DDRK") },
      { 0x106, new IoRegisterNotImplemented(*this, "PINK") },

      { 0x105, new IoRegisterNotImplemented(*this, "PORTJ") },
      { 0x104, new IoRegisterNotImplemented(*this, "DDRJ") },
      { 0x103, new IoRegisterNotImplemented(*this, "PINJ") },
      { 0x102, new IoRegisterNotImplemented(*this, "PORTH") },
      { 0x101, new IoRegisterNotImplemented(*this, "DDRH") },
      { 0x100, new IoRegisterNotImplemented(*this, "PINH") },

      { 0xD6, new IoUsart::UDRn(*this, _usart2) },
      { 0xD5, new IoRegisterNotImplemented(*this, "UBRR2H") },
      { 0xD4, new IoRegisterNotImplemented(*this, "UBRR2L") },
      { 0xD2, new IoRegisterNotImplemented(*this, "UCSR2C") },
      { 0xD1, new IoRegisterNotImplemented(*this, "UCSR2B") },
      { 0xD0, new IoUsart::UCSRnA(*this, _usart2) },

      { 0xCe, new IoUsart::UDRn(*this, _usart1) },
      { 0xCd, new IoRegisterNotImplemented(*this, "UBRR1H") },
      { 0xCc, new IoRegisterNotImplemented(*this, "UBRR1L") },
      { 0xCa, new IoRegisterNotImplemented(*this, "UCSR1C") },
      { 0xC9, new IoRegisterNotImplemented(*this, "UCSR1B") },
      { 0xC8, new IoUsart::UCSRnA(*this, _usart1) },

      { 0xC6, new IoUsart::UDRn(*this, _usart0) },
      { 0xC5, new IoRegisterNotImplemented(*this, "UBRR0H") },
      { 0xC4, new IoRegisterNotImplemented(*this, "UBRR0L") },
      { 0xC2, new IoRegisterNotImplemented(*this, "UCSR0C") },
      { 0xC1, new IoRegisterNotImplemented(*this, "UCSR0B") },
      { 0xC0, new IoUsart::UCSRnA(*this, _usart0) },
      { 0xBD, new IoRegisterNotImplemented(*this, "TWAMR") },
      { 0xBC, new IoRegisterNotImplemented(*this, "TWCR") },
      { 0xBB, new IoRegisterNotImplemented(*this, "TWDR") },
      { 0xBA, new IoRegisterNotImplemented(*this, "TWAR") },
      { 0xB9, new IoRegisterNotImplemented(*this, "TWSR") },
      { 0xB8, new IoRegisterNotImplemented(*this, "TWBR") },
      { 0xB6, new IoRegisterNotImplemented(*this, "ASSR") },
      { 0xB4, new IoRegisterNotImplemented(*this, "OCR2B") },
      { 0xB3, new IoRegisterNotImplemented(*this, "OCR2A") },
      { 0xB2, new IoRegisterNotImplemented(*this, "TCNT2") },
      { 0xB1, new IoRegisterNotImplemented(*this, "TCCR2B") },
      { 0xB0, new IoRegisterNotImplemented(*this, "TCCR2A") },

      { 0xAD, new IoRegisterNotImplemented(*this, "OCR4BH") },
      { 0xAC, new IoRegisterNotImplemented(*this, "OCR4CL") },
      { 0xAB, new IoRegisterNotImplemented(*this, "OCR4BH") },
      { 0xAA, new IoRegisterNotImplemented(*this, "OCR4BL") },
      { 0xA9, new IoRegisterNotImplemented(*this, "OCR4AH") },
      { 0xA8, new IoRegisterNotImplemented(*this, "OCR4AL") },
      { 0xA7, new IoRegisterNotImplemented(*this, "ICR4H") },
      { 0xA6, new IoRegisterNotImplemented(*this, "ICR4L") },
      { 0xA5, new IoRegisterNotImplemented(*this, "TCNT4H") },
      { 0xA4, new IoRegisterNotImplemented(*this, "TCNT4L") },

      { 0xA2, new IoRegisterNotImplemented(*this, "TCCR4C") },
      { 0xA1, new IoRegisterNotImplemented(*this, "TCCR4B") },
      { 0xA0, new IoRegisterNotImplemented(*this, "TCCR4A") },

      { 0x9D, new IoRegisterNotImplemented(*this, "OCR3BH") },
      { 0x9C, new IoRegisterNotImplemented(*this, "OCR3CL") },
      { 0x9B, new IoRegisterNotImplemented(*this, "OCR3BH") },
      { 0x9A, new IoRegisterNotImplemented(*this, "OCR3BL") },
      { 0x99, new IoRegisterNotImplemented(*this, "OCR3AH") },
      { 0x98, new IoRegisterNotImplemented(*this, "OCR3AL") },
      { 0x97, new IoRegisterNotImplemented(*this, "ICR3H") },
      { 0x96, new IoRegisterNotImplemented(*this, "ICR3L") },
      { 0x95, new IoRegisterNotImplemented(*this, "TCNT3H") },
      { 0x94, new IoRegisterNotImplemented(*this, "TCNT3L") },

      { 0x92, new IoRegisterNotImplemented(*this, "TCCR3C") },
      { 0x91, new IoRegisterNotImplemented(*this, "TCCR3B") },
      { 0x90, new IoRegisterNotImplemented(*this, "TCCR3A") },

      { 0x8D, new IoRegisterNotImplemented(*this, "OCR1BH") },
      { 0x8C, new IoRegisterNotImplemented(*this, "OCR1CL") },
      { 0x8B, new IoRegisterNotImplemented(*this, "OCR1BH") },
      { 0x8A, new IoRegisterNotImplemented(*this, "OCR1BL") },
      { 0x89, new IoRegisterNotImplemented(*this, "OCR1AH") },
      { 0x88, new IoRegisterNotImplemented(*this, "OCR1AL") },
      { 0x87, new IoRegisterNotImplemented(*this, "ICR1H") },
      { 0x86, new IoRegisterNotImplemented(*this, "ICR1L") },
      { 0x85, new IoRegisterNotImplemented(*this, "TCNT1H") },
      { 0x84, new IoRegisterNotImplemented(*this, "TCNT1L") },
      { 0x82, new IoRegisterNotImplemented(*this, "TCCR1C") },
      { 0x81, new IoRegisterNotImplemented(*this, "TCCR1B") },
      { 0x80, new IoRegisterNotImplemented(*this, "TCCR1A") },
      { 0x7F, new IoRegisterNotImplemented(*this, "DIDR1") },
      { 0x7E, new IoRegisterNotImplemented(*this, "DIDR0") },
      { 0x7D, new IoRegisterNotImplemented(*this, "DIDR2") },
      { 0x7C, new IoRegisterNotImplemented(*this, "ADMUX") },
      { 0x7B, new IoRegisterNotImplemented(*this, "ADCSRB") },
      { 0x7A, new IoRegisterNotImplemented(*this, "ADCSRA") },
      { 0x79, new IoRegisterNotImplemented(*this, "ADCH") },
      { 0x78, new IoRegisterNotImplemented(*this, "ADCL") },
      { 0x75, new IoRegisterNotImplemented(*this, "XMCRB") },
      { 0x74, new IoRegisterNotImplemented(*this, "XMCRA") },
      { 0x73, new IoRegisterNotImplemented(*this, "TIMSK5") },
      { 0x72, new IoRegisterNotImplemented(*this, "TIMSK4") },
      { 0x71, new IoRegisterNotImplemented(*this, "TIMSK3") },
      { 0x70, new IoRegisterNotImplemented(*this, "TIMSK2") },
      { 0x6F, new IoRegisterNotImplemented(*this, "TIMSK1") },
      { 0x6E, new IoRegisterNotImplemented(*this, "TIMSK0") },
      { 0x6D, new IoRegisterNotImplemented(*this, "PCMSK2") },
      { 0x6C, new IoRegisterNotImplemented(*this, "PCMSK1") },
      { 0x6B, new IoRegisterNotImplemented(*this, "PCMSK0") },
      { 0x6a, new IoRegisterNotImplemented(*this, "EICRB") },
      { 0x69, new IoRegisterNotImplemented(*this, "EICRA") },
      { 0x68, new IoRegisterNotImplemented(*this, "PCICR") },
      { 0x66, new IoRegisterNotImplemented(*this, "OSCCAL") },
      { 0x64, new IoRegisterNotImplemented(*this, "PRR") },
      { 0x61, new IoRegisterNotImplemented(*this, "CLKPR") },
      { 0x60, new IoRegisterNotImplemented(*this, "WDTCSR") },
      { 0x5f, new IoSREG::SREG(*this, _sreg) },
      { 0x5e, new IoSP::SPH(*this, _sp) },
      { 0x5d, new IoSP::SPL(*this, _sp) },
      { 0x5C, new IoRegisterNotImplemented(*this, "EIND") },
      { 0x5B, new IoRegisterNotImplemented(*this, "RAMPZ") },
      { 0x57, new IoRegisterNotImplemented(*this, "SPMCSR") },
      { 0x55, new IoRegisterNotImplemented(*this, "MCUCR") },
      { 0x54, new IoRegisterNotImplemented(*this, "MCUSR") },
      { 0x53, new IoRegisterNotImplemented(*this, "SMCR") },
      { 0x51, new IoRegisterNotImplemented(*this, "OCDR") },
      { 0x50, new IoRegisterNotImplemented(*this, "ACSR") },
      { 0x4E, new IoRegisterNotImplemented(*this, "SPDR") },
      { 0x4D, new IoRegisterNotImplemented(*this, "SPSR") },
      { 0x4C, new IoRegisterNotImplemented(*this, "SPCR") },
      { 0x4B, new IoRegisterNotImplemented(*this, "GPIOR2") },
      { 0x4A, new IoRegisterNotImplemented(*this, "GPIOR1") },
      { 0x48, new IoRegisterNotImplemented(*this, "OCR0B") },
      { 0x47, new IoRegisterNotImplemented(*this, "OCR0A") },
      { 0x46, new IoRegisterNotImplemented(*this, "TCNT0") },
      { 0x45, new IoRegisterNotImplemented(*this, "TCCR0B") },
      { 0x44, new IoRegisterNotImplemented(*this, "TCCR0A") },
      { 0x43, new IoRegisterNotImplemented(*this, "GTCCR") },
      //{ 0x42, new IoRegisterNotImplemented(*this, "EEARH") },
      //{ 0x41, new IoRegisterNotImplemented(*this, "EEARL") },
      //{ 0x40, new IoRegisterNotImplemented(*this, "EEDR") },
      //{ 0x3F, new IoRegisterNotImplemented(*this, "EECR") },
      { 0x3E, new IoRegisterNotImplemented(*this, "GPIOR0") },
      { 0x3D, new IoRegisterNotImplemented(*this, "EIMSK") },
      { 0x3C, new IoRegisterNotImplemented(*this, "EIFR") },
      { 0x3B, new IoRegisterNotImplemented(*this, "PCIFR") },
      { 0x3A, new IoRegisterNotImplemented(*this, "TIFR5") },
      { 0x39, new IoRegisterNotImplemented(*this, "TIFR4") },
      { 0x38, new IoRegisterNotImplemented(*this, "TIFR3") },
      { 0x37, new IoRegisterNotImplemented(*this, "TIFR2") },
      { 0x36, new IoRegisterNotImplemented(*this, "TIFR1") },
      { 0x35, new IoRegisterNotImplemented(*this, "TIFR0") },
      { 0x34, new IoRegisterNotImplemented(*this, "PORTG") },
      { 0x33, new IoRegisterNotImplemented(*this, "DDRG") },
      { 0x32, new IoRegisterNotImplemented(*this, "PING") },
      { 0x31, new IoRegisterNotImplemented(*this, "PORTF") },
      { 0x30, new IoRegisterNotImplemented(*this, "DDRF") },
      { 0x2F, new IoRegisterNotImplemented(*this, "PINF") },
      { 0x2E, new IoRegisterNotImplemented(*this, "PORTE") },
      { 0x2D, new IoRegisterNotImplemented(*this, "DDRE") },
      { 0x2C, new IoRegisterNotImplemented(*this, "PINE") },
      { 0x2B, new IoRegisterNotImplemented(*this, "PORTD") },
      { 0x2A, new IoRegisterNotImplemented(*this, "DDRD") },
      { 0x29, new IoRegisterNotImplemented(*this, "PIND") },
      { 0x28, new IoRegisterNotImplemented(*this, "PORTC") },
      { 0x27, new IoRegisterNotImplemented(*this, "DDRC") },
      { 0x26, new IoRegisterNotImplemented(*this, "PINC") },
      { 0x25, new IoRegisterNotImplemented(*this, "PORTB") },
      { 0x24, new IoRegisterNotImplemented(*this, "DDRB") },
      { 0x23, new IoRegisterNotImplemented(*this, "PINB") },
    } ;
    for (const auto &iIoReg: ioRegs)
    {
          _io[iIoReg.first-0x20] = iIoReg.second ;
    }
    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x00, "RESET",        "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x02, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x04, "IRQ_INT1",         "External Interrupt Request 1" },
        { 0x06, "IRQ_INT2",         "External Interrupt Request 2" },
        { 0x08, "IRQ_INT3",         "External Interrupt Request 3" },
        { 0x0a, "IRQ_INT4",         "External Interrupt Request 4" },
        { 0x0c, "IRQ_INT5",         "External Interrupt Request 5" },
        { 0x0e, "IRQ_INT6",         "External Interrupt Request 6" },
        { 0x10, "IRQ_INT7",         "External Interrupt Request 7" },
        { 0x12, "IRQ_PCINT0",       "Pin Change Interrupt Request 0" },
        { 0x14, "IRQ_PCINT1",       "Pin Change Interrupt Request 1" },
        { 0x16, "IRQ_PCINT2",       "Pin Change Interrupt Request 2" },
        { 0x18, "IRQ_WDT",          "Watchdog Time-out Interrupt" },
        { 0x1a, "IRQ_TIMER2_COMPA", "Timer/Counter2 Compare Match A" },
        { 0x1c, "IRQ_TIMER2_COMPB", "Timer/Counter2 Compare Match B" },
        { 0x1e, "IRQ_TIMER2_OVF",   "Timer/Counter2 Overflow" },
        { 0x20, "IRQ_TIMER1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x22, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x24, "IRQ_TIMER1_COMPB", "Timer/Coutner1 Compare Match B" },
        { 0x26, "IRQ_TIMER1_COMPC", "Timer/Coutner1 Compare Match C" },
        { 0x28, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x2a, "IRQ_TIMER0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x2c, "IRQ_TIMER0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x2e, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x30, "IRQ_SPI_STC",      "SPI Serial Transfer Complete" },
        { 0x32, "IRQ_USART0_RXC",   "USART0 Rx Complete" },
        { 0x34, "IRQ_USART0_UDRE",  "USART0, Data Register Empty" },
        { 0x36, "IRQ_USART0_TXC",   "USART0, Tx Complete" },
        { 0x38, "IRQ_ANA_COMP",     "Analog Comparator" },
        { 0x3a, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x3c, "IRQ_EE_READY",     "EEPROM Ready" },
        { 0x3e, "IRQ_TIMER3_CAPT",  "Timer/Counter3 Capture Event" },
        { 0x40, "IRQ_TIMER3_COMPA", "Timer/Counter3 Compare Match A" },
        { 0x42, "IRQ_TIMER3_COMPB", "Timer/Coutner3 Compare Match B" },
        { 0x44, "IRQ_TIMER3_COMPC", "Timer/Coutner3 Compare Match C" },
        { 0x46, "IRQ_TIMER3_OVF",   "Timer/Counter3 Overflow" },
        { 0x48, "IRQ_USART1_RXC",   "USART1 Rx Complete" },
        { 0x4a, "IRQ_USART1_UDRE",  "USART1, Data Register Empty" },
        { 0x4c, "IRQ_USART1_TXC",   "USART1, Tx Complete" },
        { 0x4e, "IRQ_TWI",          "2-wire Serial Interface" },
        { 0x50, "IRQ_SPM_READY",    "Store Program Memory Ready" },
        { 0x52, "IRQ_TIMER4_CAPT",  "Timer/Counter4 Capture Event" },
        { 0x54, "IRQ_TIMER4_COMPA", "Timer/Counter4 Compare Match A" },
        { 0x56, "IRQ_TIMER4_COMPB", "Timer/Coutner4 Compare Match B" },
        { 0x58, "IRQ_TIMER4_COMPC", "Timer/Coutner4 Compare Match C" },
        { 0x5a, "IRQ_TIMER4_OVF",   "Timer/Counter4 Overflow" },
        { 0x5c, "IRQ_TIMER5_CAPT",  "Timer/Counter5 Capture Event" },
        { 0x5e, "IRQ_TIMER5_COMPA", "Timer/Counter5 Compare Match A" },
        { 0x60, "IRQ_TIMER5_COMPB", "Timer/Coutner5 Compare Match B" },
        { 0x62, "IRQ_TIMER5_COMPC", "Timer/Coutner5 Compare Match C" },
        { 0x64, "IRQ_TIMER5_OVF",   "Timer/Counter5 Overflow" },
        { 0x66, "IRQ_USART2_RXC",   "USART2 Rx Complete" },
        { 0x68, "IRQ_USART2_UDRE",  "USART2, Data Register Empty" },
        { 0x6a, "IRQ_USART2_TXC",   "USART2, Tx Complete" },
        { 0x6c, "IRQ_USART3_RXC",   "USART3 Rx Complete" },
        { 0x6e, "IRQ_USART3_UDRE",  "USART3, Data Register Empty" },
        { 0x70, "IRQ_USART3_TXC",   "USART3, Tx Complete" },

      } ;

    _io[0x22] = new IoEeprom::EEARH(*this, ioEeprom) ;
    _io[0x21] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x20] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1f] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATmega2560::~ATmega2560()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
