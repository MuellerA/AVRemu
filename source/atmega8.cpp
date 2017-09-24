////////////////////////////////////////////////////////////////////////////////
// atmegaXX8.cpp
// ATmega48A/PA/88A/PA/168A/PA/328/P
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{

  ATmega8A::ATmega8A() : Mcu(0x2000/2, 0x0040, 0x0060, 0x0400, 0x0200)
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

      /*&instrBREAK,*/ &instrNOP, &instrSLEEP, &instrWDR,
    } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x000, "RESET",            "External Pin, Power-on Reset, Brown-out Reset, and Watchdog Reset" },
        { 0x001, "IRQ_INT0",         "External Interrupt Request 0" },
        { 0x002, "IRQ_INT1",         "External Interrupt Request 1" },
        { 0x003, "IRQ_TIMER2_COMP",  "Timer/Counter2 Compare Match" },
        { 0x004, "IRQ_TIMER2_OVF",   "Timer/Counter2 Overflow" },
        { 0x005, "IRQ_TIMER1_CAPT",  "Timer/Counter1 Capture Event" },
        { 0x006, "IRQ_TIMER1_COMPA", "Timer/Counter1 Compare Match A" },
        { 0x007, "IRQ_TIMER1_COMPB", "Timer/Counter1 Compare Match B" },
        { 0x008, "IRQ_TIMER1_OVF",   "Timer/Counter1 Overflow" },
        { 0x009, "IRQ_TIMER0_OVF",   "Timer/Counter0 Overflow" },
        { 0x00A, "IRQ_SPI_STC",      "Serial Transfer Complete" },
        { 0x00B, "IRQ_USART_RXC",    "USART, Rx Complete" },
        { 0x00C, "IRQ_USART_UDRE",   "USART Data Register Empty" },
        { 0x00D, "IRQ_USART_TXC",    "USART, Tx Complete" },
        { 0x00E, "IRQ_ADC",          "ADC Conversion Complete" },
        { 0x00F, "IRQ_EE_RDY",       "EEPROM Ready" },
        { 0x010, "IRQ_ANA_COMP",     "Analog Comparator" },
        { 0x011, "IRQ_TWI",          "Two-wire Serial Interface" },
        { 0x012, "IRQ_SPM_RDY",      "Store Program Memory Ready        " },
      } ;
    
    std::vector<std::pair<uint32, std::string>> ioRegs
    {
      { 0x5A, "GIFR" },
      { 0x59, "TIMSK" },
      { 0x58, "TIFR" },
      { 0x57, "SPMCR" },
      { 0x56, "TWCR" },
      { 0x55, "MCUCR" },
      { 0x54, "MCUCSR" },
      { 0x53, "TCCR0" },
      { 0x52, "TCNT0" },
      { 0x51, "OSCCAL" },
      { 0x50, "SFIOR" },
      { 0x4F, "TCCR1A" },
      { 0x4E, "TCCR1B" },
      { 0x4D, "TCNT1H" },
      { 0x4C, "TCNT1L" },
      { 0x4B, "OCR1AH" },
      { 0x4A, "OCR1AL" },
      { 0x49, "OCR1BH" },
      { 0x48, "OCR1BL" },
      { 0x47, "ICR1H" },
      { 0x46, "ICR1L" },
      { 0x45, "TCCR2" },
      { 0x44, "TCNT2" },
      { 0x43, "OCR2" },
      { 0x42, "ASSR" },
      { 0x41, "WDTCR" },
      { 0x40, "UBRRH|UCSRC" },
      { 0x3F, "EEARH" },
      { 0x3E, "EEARL" },
      { 0x3D, "EEDR" },
      { 0x3C, "EECR" },
      { 0x38, "PORTB" },
      { 0x37, "DDRB" },
      { 0x36, "PINB" },
      { 0x35, "PORTC" },
      { 0x34, "DDRC" },
      { 0x33, "PINC" },
      { 0x32, "PORTD" },
      { 0x31, "DDRD" },
      { 0x30, "PIND" },
      { 0x2F, "SPDR" },
      { 0x2E, "SPSR" },
      { 0x2D, "SPCR" },
      { 0x2C, "UDR" },
      { 0x2B, "UCSRA" },
      { 0x2A, "UCSRB" },
      { 0x29, "UBRRL" },
      { 0x28, "ACSR" },
      { 0x27, "ADMUX" },
      { 0x26, "ADCSRA" },
      { 0x25, "ADCH" },
      { 0x24, "ADCL" },
      { 0x23, "TWDR" },
      { 0x22, "TWAR" },
      { 0x21, "TWSR" },
      { 0x20, "TWBR" },
    } ;
    for (const auto &iIoReg: ioRegs)
    {
      _io[iIoReg.first-0x20] = new IoRegisterNotImplemented(iIoReg.second) ;
    }
    _io[0x3f] = new IoSREG::SREG(_sreg) ;
    _io[0x3e] = new IoSP::SPH(_sp) ;
    _io[0x3d] = new IoSP::SPL(_sp) ;
  }

  ATmega8A::~ATmega8A()
  {
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
