////////////////////////////////////////////////////////////////////////////////
// attinyX5.cpp
// ATtiny25/V/45/V/85/V
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{

  ATtinyX5::ATtinyX5(std::size_t programSize, std::size_t dataSize, std::size_t eepromSize)
    : Mcu(programSize, 0x40, 0x60, dataSize, eepromSize), ioEeprom(*this)
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
        { 0x00, "RESET",        "External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset" },
        { 0x01, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x02, "IRQ_PCINT0",       "Pin Change Interrupt Request 0" },
        { 0x03, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x04, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x05, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x06, "IRQ_EE_RDY",       "EEPROM Ready" },
        { 0x07, "IRQ_ANA_COMP",     "Analog Comparator" },
        { 0x08, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x09, "IRQ_TIMER1_COMPB", "Timer/Counter1 Compare Match B" },
        { 0x0a, "IRQ_TIMER0_COMPA", "Timer/Counter0 Compare Match A" },
        { 0x0b, "IRQ_TIMER0_COMPB", "Timer/Counter0 Compare Match B" },
        { 0x0c, "IRQ_WDT",          "Watchdog Time-out" },
        { 0x0d, "IRQ_USI_START",    "USI START" },
        { 0x0e, "IRQ_USI_OVF",      "USI Overflow" },
      } ;

    std::vector<std::pair<uint32, std::string>> ioRegs
    {
      { 0x5B, "GIMSK" },
      { 0x5A, "GIFR" },
      { 0x59, "TIMSK" },
      { 0x58, "TIFR" },
      { 0x57, "SPMCSR" },
      { 0x55, "MCUCR" },
      { 0x54, "MCUSR" },
      { 0x53, "TCCR0B" },
      { 0x52, "TCNT0" },
      { 0x51, "OSCCAL" },
      { 0x50, "TCCR1" },
      { 0x4F, "TCNT1" },
      { 0x4E, "OCR1A" },
      { 0x4D, "OCR1C" },
      { 0x4C, "GTCCR" },
      { 0x4B, "OCR1B" },
      { 0x4A, "TCCR0A" },
      { 0x49, "OCR0A" },
      { 0x48, "OCR0B" },
      { 0x47, "PLLCSR" },
      { 0x46, "CLKPR" },
      { 0x45, "DT1A" },
      { 0x44, "DT1B" },
      { 0x43, "DTPS1" },
      { 0x42, "DWDR" },
      { 0x41, "WDTCR" },
      { 0x40, "PRR" },
      //{ 0x3F, "EEARH" },
      //{ 0x3E, "EEARL" },
      //{ 0x3D, "EEDR" },
      //{ 0x3C, "EECR" },
      { 0x38, "PORTB" },
      { 0x37, "DDRB" },
      { 0x36, "PINB" },
      { 0x35, "PCMSK" },
      { 0x34, "DIDR0" },
      { 0x33, "GPIOR2" },
      { 0x32, "GPIOR1" },
      { 0x31, "GPIOR0" },
      { 0x30, "USIBR" },
      { 0x2F, "USIDR" },
      { 0x2E, "USISR" },
      { 0x2D, "USICR" },
      { 0x28, "ACSR" },
      { 0x27, "ADMUX" },
      { 0x26, "ADCSRA" },
      { 0x25, "ADCH" },
      { 0x24, "ADCL" },
      { 0x23, "ADCSRB" },
    } ;
    for (const auto &iIoReg: ioRegs)
    {
      _io[iIoReg.first-0x20] = new IoRegisterNotImplemented(iIoReg.second) ;
    }
    _io[0x3f] = new IoSREG::SREG(_sreg) ;
    _io[0x3e] = new IoSP::SPH(_sp) ;
    _io[0x3d] = new IoSP::SPL(_sp) ;
  }
  ATtinyX5::~ATtinyX5()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny85::ATtiny85() : ATtinyX5(0x2000/2, 0x200, 0x200)
  {
    _io[0x1f] = new IoEeprom::EEARH(ioEeprom) ;
    _io[0x1e] = new IoEeprom::EEARL(ioEeprom) ;
    _io[0x1d] = new IoEeprom::EEDR (ioEeprom) ;
    _io[0x1c] = new IoEeprom::EECR (ioEeprom) ;
  }
  ATtiny85::~ATtiny85()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny45::ATtiny45() : ATtinyX5(0x1000/2, 0x100, 0x100)
  {
    _io[0x1e] = new IoEeprom::EEARL(ioEeprom) ;
    _io[0x1d] = new IoEeprom::EEDR (ioEeprom) ;
    _io[0x1c] = new IoEeprom::EECR (ioEeprom) ;
  }
  ATtiny45::~ATtiny45()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny25::ATtiny25() : ATtinyX5(0x800/2, 0x80, 0x80)
  {
    _io[0x1e] = new IoEeprom::EEARL(ioEeprom) ;
    _io[0x1d] = new IoEeprom::EEDR (ioEeprom) ;
    _io[0x1c] = new IoEeprom::EECR (ioEeprom) ;
  }
  ATtiny25::~ATtiny25()
  {
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
