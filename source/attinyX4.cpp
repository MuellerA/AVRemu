////////////////////////////////////////////////////////////////////////////////
// attiny24.cpp
// ATtiny24A/44A/84A
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{

  ATtinyX4::ATtinyX4(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize)
    : Mcu(name, flashSize, 0x40, ramSize, eepromSize, ramSize + 0x5F),
      ioEeprom(*this)
  {
    const Instruction *instructions[]
    {
      &instrADD, &instrADC, &instrADIW, &instrSUB, &instrSUBI, &instrSBC, &instrSBCI, &instrSBIW, &instrAND, &instrANDI,
      &instrOR, &instrORI, &instrEOR, &instrCOM, &instrNEG, &instrINC, &instrDEC, /*&instrMUL,*/ /*&instrMULS,*/ /*&instrMULSU,*/
      /*&instrFMUL,*/ /*&instrFMULS,*/ /*&instrFMULSU,*/ /*&instrDES,*/

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

    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x00, "RESET",      "External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset" },
        { 0x01, "IRQ_INT0",       "External Interrupt Request 0" },
        { 0x02, "IRQ_PCINT0",     "Pin Change Interrupt Request 0" },
        { 0x03, "IRQ_PCINT1",     "Pin Change Interrupt Request 1" },
        { 0x04, "IRQ_WDT",        "Watchdog Time-out" },
        { 0x05, "IRQ_TIM1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x06, "IRQ_TIM1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x07, "IRQ_TIM1_COMPB", "Timer/Counter1 Compare Match B" },
        { 0x08, "IRQ_TIM1_OVF",   "Timer/Counter1 Overflow" },
        { 0x09, "IRQ_TIM0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x0a, "IRQ_TIM0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x0b, "IRQ_TIM0_OVF",   "Timer/Counter0 Overflow" },
        { 0x0c, "IRQ_ANA_COMP",   "Analog Comparator" },
        { 0x0d, "IRQ_ADC",        "ADC Conversion Complete" },
        { 0x0e, "IRQ_EE_RDY",     "EEPROM Ready" },
        { 0x0f, "IRQ_USI_STR",    "USI START" },
        { 0x10, "IRQ_USI_OVF",    "USI Overflow" },
      } ;

    std::vector<std::pair<uint32_t, std::string>> ioRegs
    {
      { 0x5C, "OCR0B" },
      { 0x5B, "GIMSK" },
      { 0x5A, "GIFR" },
      { 0x59, "TIMSK0" },
      { 0x58, "TIFR0" },
      { 0x57, "SPMCSR" },
      { 0x56, "OCR0A" },
      { 0x55, "MCUCR" },
      { 0x54, "MCUSR" },
      { 0x53, "TCCR0B" },
      { 0x52, "TCNT0" },
      { 0x51, "OSCCAL" },
      { 0x50, "TCCR0A" },
      { 0x4F, "TCCR1A" },
      { 0x4E, "TCCR1B" },
      { 0x4D, "TCNT1H" },
      { 0x4C, "TCNT1L" },
      { 0x4B, "OCR1AH" },
      { 0x4A, "OCR1AL" },
      { 0x49, "OCR1BH" },
      { 0x48, "OCR1BL" },
      { 0x47, "DWDR" },
      { 0x46, "CLKPR" },
      { 0x45, "ICR1H" },
      { 0x44, "ICR1L" },
      { 0x43, "GTCCR" },
      { 0x42, "TCCR1C" },
      { 0x41, "WDTCSR" },
      { 0x40, "PCMSK1" },
      //{ 0x3F, "EEARH" },
      //{ 0x3E, "EEARL" },
      //{ 0x3D, "EEDR" },
      //{ 0x3C, "EECR" },
      { 0x3B, "PORTA" },
      { 0x3A, "DDRA" },
      { 0x39, "PINA" },
      { 0x38, "PORTB" },
      { 0x37, "DDRB" },
      { 0x36, "PINB" },
      { 0x35, "GPIOR2" },
      { 0x34, "GPIOR1" },
      { 0x33, "GPIOR0" },
      { 0x32, "PCMSK0" },
      { 0x31, "Reserved" },
      { 0x30, "USIBR" },
      { 0x2F, "USIDR" },
      { 0x2E, "USISR" },
      { 0x2D, "USICR" },
      { 0x2C, "TIMSK1" },
      { 0x2B, "TIFR1" },
      { 0x28, "ACSR" },
      { 0x27, "ADMUX" },
      { 0x26, "ADCSRA" },
      { 0x25, "ADCH" },
      { 0x24, "ADCL" },
      { 0x23, "ADCSRB" },
      { 0x21, "DIDR0" },
      { 0x20, "PRR" },
    } ;
    for (const auto &iIoReg: ioRegs)
    {
      _io[iIoReg.first-0x20] = new IoRegisterNotImplemented(*this, iIoReg.second) ;
    }
    _io[0x3f] = new IoSREG::SREG(*this, _sreg) ;
    _io[0x3e] = new IoSP::SPH(*this, _sp) ;
    _io[0x3d] = new IoSP::SPL(*this, _sp) ;
  }
  ATtinyX4::~ATtinyX4()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny84A::ATtiny84A() : ATtinyX4("ATtiny84A", 0x2000/2, 0x200, 0x200)
  {
    _io[0x1f] = new IoEeprom::EEARH(*this, ioEeprom) ;
    _io[0x1e] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x1d] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1c] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATtiny84A::~ATtiny84A()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny44A::ATtiny44A() : ATtinyX4("ATtiny44A", 0x1000/2, 0x100, 0x100)
  {
    _io[0x1e] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x1d] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1c] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATtiny44A::~ATtiny44A()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny24A::ATtiny24A() : ATtinyX4("ATtiny24A", 0x800/2, 0x80, 0x80)
  {
    _io[0x1e] = new IoEeprom::EEARL(*this, ioEeprom) ;
    _io[0x1d] = new IoEeprom::EEDR (*this, ioEeprom) ;
    _io[0x1c] = new IoEeprom::EECR (*this, ioEeprom) ;
  }
  ATtiny24A::~ATtiny24A()
  {
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
