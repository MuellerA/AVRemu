////////////////////////////////////////////////////////////////////////////////
// atmegaXX8.cpp
// ATmega48A/PA/88A/PA/168A/PA/328/P
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{

  ATmegaXX8::ATmegaXX8(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize)
    : Mcu(name, flashSize, 0xe0, ramSize, eepromSize, ramSize + 0xff),
      ioEeprom(*this), _usart0()
  {
    const Instruction *instructions[]
    {
      &instrADD, &instrADC, &instrADIW, &instrSUB, &instrSUBI, &instrSBC, &instrSBCI, &instrSBIW, &instrAND, &instrANDI,
      &instrOR, &instrORI, &instrEOR, &instrCOM, &instrNEG, &instrINC, &instrDEC, &instrMUL, &instrMULS, &instrMULSU,
      &instrFMUL, &instrFMULS, &instrFMULSU, /*&instrDES,*/

      &instrRJMP, &instrIJMP, /*&instrEIJMP,*/ /*&instrJMP,*/ &instrRCALL, &instrICALL, /*&instrEICALL,*/ /*&instrCALL,*/ &instrRET,
      &instrRETI, &instrCPSE, &instrCP, &instrCPC, &instrCPI, &instrSBRC, &instrSBRS, &instrSBIC, &instrSBIS,
      &instrBRBS, &instrBRBC,

      &instrMOV, &instrMOVW, &instrLDI, &instrLDS, &instrLDx1, &instrLDx2, &instrLDx3, &instrLDy1, &instrLDy2,
      &instrLDy3, &instrLDy4, &instrLDz1, &instrLDz2, &instrLDz3, &instrLDz4, &instrSTS, &instrSTx1, &instrSTx2,
      &instrSTx3, &instrSTy1, &instrSTy2, &instrSTy3, &instrSTy4, &instrSTz1, &instrSTz2, &instrSTz3, &instrSTz4,
      &instrLPM1, &instrLPM2, &instrLPM3, /*&instrELPM1,*/ /*&instrELPM2,*/ /*&instrELPM3,*/ &instrSPM1, /*&instrSPM2,*/ &instrIN,
      &instrOUT, &instrPUSH, &instrPOP, /*&instrXCH,*/ /*&instrLAS,*/ /*&instrLAC,*/ /*&instrLAT,*/

      &instrLSR, &instrROR, &instrASR, &instrSWAP, &instrBSET, &instrBCLR, &instrSBI, &instrCBI, &instrBST,
      &instrBLD,

      &instrBREAK, &instrNOP, &instrSLEEP, &instrWDR,
    } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    std::vector<std::pair<uint32_t, Io::Register*>> ioRegs
    {
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
      { 0x7C, new IoRegisterNotImplemented(*this, "ADMUX") },
      { 0x7B, new IoRegisterNotImplemented(*this, "ADCSRB") },
      { 0x7A, new IoRegisterNotImplemented(*this, "ADCSRA") },
      { 0x79, new IoRegisterNotImplemented(*this, "ADCH") },
      { 0x78, new IoRegisterNotImplemented(*this, "ADCL") },
      { 0x70, new IoRegisterNotImplemented(*this, "TIMSK2") },
      { 0x6F, new IoRegisterNotImplemented(*this, "TIMSK1") },
      { 0x6E, new IoRegisterNotImplemented(*this, "TIMSK0") },
      { 0x6D, new IoRegisterNotImplemented(*this, "PCMSK2") },
      { 0x6C, new IoRegisterNotImplemented(*this, "PCMSK1") },
      { 0x6B, new IoRegisterNotImplemented(*this, "PCMSK0") },
      { 0x69, new IoRegisterNotImplemented(*this, "EICRA") },
      { 0x68, new IoRegisterNotImplemented(*this, "PCICR") },
      { 0x66, new IoRegisterNotImplemented(*this, "OSCCAL") },
      { 0x64, new IoRegisterNotImplemented(*this, "PRR") },
      { 0x61, new IoRegisterNotImplemented(*this, "CLKPR") },
      { 0x60, new IoRegisterNotImplemented(*this, "WDTCSR") },
      { 0x5f, new IoSREG::SREG(*this, _sreg) },
      { 0x5e, new IoSP::SPH(*this, _sp) },
      { 0x5d, new IoSP::SPL(*this, _sp) },
      { 0x57, new IoRegisterNotImplemented(*this, "SPMCSR") },
      { 0x55, new IoRegisterNotImplemented(*this, "MCUCR") },
      { 0x54, new IoRegisterNotImplemented(*this, "MCUSR") },
      { 0x53, new IoRegisterNotImplemented(*this, "SMCR") },
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
      { 0x37, new IoRegisterNotImplemented(*this, "TIFR2") },
      { 0x36, new IoRegisterNotImplemented(*this, "TIFR1") },
      { 0x35, new IoRegisterNotImplemented(*this, "TIFR0") },
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
  }
  ATmegaXX8::~ATmegaXX8()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega328P::ATmega328P() : ATmegaXX8("ATmega328P", 0x8000/2, 0x0800, 0x0400)
  {
    const Instruction *instructions[] { &instrJMP, &instrCALL, } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x00, "RESET",        "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x02, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x04, "IRQ_INT1",         "External Interrupt Request 1" },
        { 0x06, "IRQ_PCINT0",       "Pin Change Interrupt Request 0" },
        { 0x08, "IRQ_PCINT1",       "Pin Change Interrupt Request 1" },
        { 0x0a, "IRQ_PCINT2",       "Pin Change Interrupt Request 2" },
        { 0x0c, "IRQ_WDT",          "Watchdog Time-out Interrupt" },
        { 0x0e, "IRQ_TIMER2_COMPA", "Timer/Counter2 Compare Match A" },
        { 0x10, "IRQ_TIMER2_COMPB", "Timer/Counter2 Compare Match B" },
        { 0x12, "IRQ_TIMER2_OVF",   "Timer/Counter2 Overflow" },
        { 0x14, "IRQ_TIMER1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x16, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x18, "IRQ_TIMER1_COMPB", "Timer/Coutner1 Compare Match B" },
        { 0x1a, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x1c, "IRQ_TIMER0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x1e, "IRQ_TIMER0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x20, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x22, "IRQ_SPI_STC",      "SPI Serial Transfer Complete" },
        { 0x24, "IRQ_USART_RX",     "USART Rx Complete" },
        { 0x26, "IRQ_USART_UDRE",   "USART, Data Register Empty" },
        { 0x28, "IRQ_USART_TX",     "USART, Tx Complete" },
        { 0x2a, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x2c, "IRQ_EE_READY",     "EEPROM Ready" },
        { 0x2e, "IRQ_ANALOG_COMP",  "Analog Comparator" },
        { 0x30, "IRQ_TWI",          "2-wire Serial Interface" },
        { 0x32, "IRQ_SPM_READY",    "Store Program Memory Ready" },
      } ;

    _io[0x22] = new IoEeprom::EEARH(*this, ioEeprom) ;
    _io[0x21] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x20] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1f] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATmega328P::~ATmega328P()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega168PA::ATmega168PA() : ATmegaXX8("ATmega168PA", 0x4000/2, 0x0400, 0x0200)
  {
    const Instruction *instructions[] { &instrJMP, &instrCALL, } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x00, "RESET",        "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x02, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x04, "IRQ_INT1",         "External Interrupt Request 1" },
        { 0x06, "IRQ_PCINT0",       "Pin Change Interrupt Request 0" },
        { 0x08, "IRQ_PCINT1",       "Pin Change Interrupt Request 1" },
        { 0x0a, "IRQ_PCINT2",       "Pin Change Interrupt Request 2" },
        { 0x0c, "IRQ_WDT",          "Watchdog Time-out Interrupt" },
        { 0x0e, "IRQ_TIMER2_COMPA", "Timer/Counter2 Compare Match A" },
        { 0x10, "IRQ_TIMER2_COMPB", "Timer/Counter2 Compare Match B" },
        { 0x12, "IRQ_TIMER2_OVF",   "Timer/Counter2 Overflow" },
        { 0x14, "IRQ_TIMER1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x16, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x18, "IRQ_TIMER1_COMPB", "Timer/Coutner1 Compare Match B" },
        { 0x1a, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x1c, "IRQ_TIMER0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x1e, "IRQ_TIMER0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x20, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x22, "IRQ_SPI_STC",      "SPI Serial Transfer Complete" },
        { 0x24, "IRQ_USART_RX",     "USART Rx Complete" },
        { 0x26, "IRQ_USART_UDRE",   "USART, Data Register Empty" },
        { 0x28, "IRQ_USART_TX",     "USART, Tx Complete" },
        { 0x2a, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x2c, "IRQ_EE_READY",     "EEPROM Ready" },
        { 0x2e, "IRQ_ANALOG_COMP",  "Analog Comparator" },
        { 0x30, "IRQ_TWI",          "2-wire Serial Interface" },
        { 0x32, "IRQ_SPM_READY",    "Store Program Memory Ready" },
      } ;

    _io[0x22] = new IoEeprom::EEARH(*this, ioEeprom) ;
    _io[0x21] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x20] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1f] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATmega168PA::~ATmega168PA()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega88PA::ATmega88PA() : ATmegaXX8("ATmega88PA", 0x2000/2, 0x0400, 0x0200)
  {
    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x00, "RESET",        "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x01, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x02, "IRQ_INT1",         "External Interrupt Request 1" },
        { 0x03, "IRQ_PCINT0",       "Pin Change Interrupt Request 0" },
        { 0x04, "IRQ_PCINT1",       "Pin Change Interrupt Request 1" },
        { 0x05, "IRQ_PCINT2",       "Pin Change Interrupt Request 2" },
        { 0x06, "IRQ_WDT",          "Watchdog Time-out Interrupt" },
        { 0x07, "IRQ_TIMER2_COMPA", "Timer/Counter2 Compare Match A" },
        { 0x08, "IRQ_TIMER2_COMPB", "Timer/Counter2 Compare Match B" },
        { 0x09, "IRQ_TIMER2_OVF",   "Timer/Counter2 Overflow" },
        { 0x0a, "IRQ_TIMER1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x0b, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x0c, "IRQ_TIMER1_COMPB", "Timer/Coutner1 Compare Match B" },
        { 0x0d, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x0e, "IRQ_TIMER0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x0f, "IRQ_TIMER0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x10, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x11, "IRQ_SPI_STC",      "SPI Serial Transfer Complete" },
        { 0x12, "IRQ_USART_RX",     "USART Rx Complete" },
        { 0x13, "IRQ_USART_UDRE",   "USART, Data Register Empty" },
        { 0x14, "IRQ_USART_TX",     "USART, Tx Complete" },
        { 0x15, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x16, "IRQ_EE_READY",     "EEPROM Ready" },
        { 0x17, "IRQ_ANALOG_COMP",  "Analog Comparator" },
        { 0x18, "IRQ_TWI",          "2-wire Serial Interface" },
        { 0x19, "IRQ_SPM_READY",    "Store Program Memory Ready" },
      } ;

    _io[0x22] = new IoEeprom::EEARH(*this, ioEeprom) ;
    _io[0x21] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x20] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1f] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATmega88PA::~ATmega88PA()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega48PA::ATmega48PA() : ATmegaXX8("ATmega48PA", 0x1000/2, 0x0200, 0x0100)
  {
    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x00, "RESET",        "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x01, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x02, "IRQ_INT1",         "External Interrupt Request 1" },
        { 0x03, "IRQ_PCINT0",       "Pin Change Interrupt Request 0" },
        { 0x04, "IRQ_PCINT1",       "Pin Change Interrupt Request 1" },
        { 0x05, "IRQ_PCINT2",       "Pin Change Interrupt Request 2" },
        { 0x06, "IRQ_WDT",          "Watchdog Time-out Interrupt" },
        { 0x07, "IRQ_TIMER2_COMPA", "Timer/Counter2 Compare Match A" },
        { 0x08, "IRQ_TIMER2_COMPB", "Timer/Counter2 Compare Match B" },
        { 0x09, "IRQ_TIMER2_OVF",   "Timer/Counter2 Overflow" },
        { 0x0a, "IRQ_TIMER1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x0b, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x0c, "IRQ_TIMER1_COMPB", "Timer/Coutner1 Compare Match B" },
        { 0x0d, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x0e, "IRQ_TIMER0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x0f, "IRQ_TIMER0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x10, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x11, "IRQ_SPI_STC",      "SPI Serial Transfer Complete" },
        { 0x12, "IRQ_USART_RX",     "USART Rx Complete" },
        { 0x13, "IRQ_USART_UDRE",   "USART, Data Register Empty" },
        { 0x14, "IRQ_USART_TX",     "USART, Tx Complete" },
        { 0x15, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x16, "IRQ_EE_READY",     "EEPROM Ready" },
        { 0x17, "IRQ_ANALOG_COMP",  "Analog Comparator" },
        { 0x18, "IRQ_TWI",          "2-wire Serial Interface" },
        { 0x19, "IRQ_SPM_READY",    "Store Program Memory Ready" },
      } ;

    _io[0x21] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x20] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1f] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATmega48PA::~ATmega48PA()
  {
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
