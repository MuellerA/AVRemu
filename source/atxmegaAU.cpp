////////////////////////////////////////////////////////////////////////////////
// atxmegaAU.cpp
// ATxmega128A4U / 64A4U / 32A4U / 16A4U
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{
  ATxmegaAU::ATxmegaAU(std::size_t programSize, std::size_t ioSize, std::size_t dataSize, std::size_t eepromSize)
    : Mcu(programSize, ioSize, dataSize, eepromSize)
  {
    const Instruction *instructions[]
    {
      &instrADD, &instrADC, &instrADIW, &instrSUB, &instrSUBI, &instrSBC, &instrSBCI, &instrSBIW, &instrAND, &instrANDI,
      &instrOR, &instrORI, &instrEOR, &instrCOM, &instrNEG, &instrINC, &instrDEC, &instrMUL, &instrMULS, &instrMULSU,
      &instrFMUL, &instrFMULS, &instrFMULSU, &instrDES,

      &instrRJMP, &instrIJMP, &instrEIJMP, &instrJMP, &instrRCALL, &instrICALL, &instrEICALL, &instrCALL, &instrRET,
      &instrRETI, &instrCPSE, &instrCP, &instrCPC, &instrCPI, &instrSBRC, &instrSBRS, &instrSBIC, &instrSBIS,
      &instrBRBS, &instrBRBC,

      &instrMOV, &instrMOVW, &instrLDI, &instrLDS, &instrLDx1, &instrLDx2, &instrLDx3, &instrLDy1, &instrLDy2,
      &instrLDy3, &instrLDy4, &instrLDz1, &instrLDz2, &instrLDz3, &instrLDz4, &instrSTS, &instrSTx1, &instrSTx2,
      &instrSTx3, &instrSTy1, &instrSTy2, &instrSTy3, &instrSTy4, &instrSTz1, &instrSTz2, &instrSTz3, &instrSTz4,
      &instrLPM1, &instrLPM2, &instrLPM3, &instrELPM1, &instrELPM2, &instrELPM3, &instrSPM1, &instrSPM2, &instrIN,
      &instrOUT, &instrPUSH, &instrPOP, &instrXCH, &instrLAS, &instrLAC, &instrLAT,

      &instrLSR, &instrROR, &instrASR, &instrSWAP, &instrBSET, &instrBCLR, &instrSBI, &instrCBI, &instrBST,
      &instrBLD,

      &instrBREAK, &instrNOP, &instrSLEEP, &instrWDR,
    } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x000, "RESET"           , "RESET" },
        { 0x002, "OSCF_INT"        , "Crystal oscillator failure interrupt (NMI)" },
        { 0x004, "PORTC_INT0"      , "Port C interrupt 0" },
        { 0x006, "PORTC_INT1"      , "Port C interrupt 1" },
        { 0x008, "PORTR_INT0"      , "Port R interrupt 0" },
        { 0x00a, "PORTR_INT1"      , "Port R interrupt 1" },
        { 0x00C, "DMA_INT_CH0"     , "DMA controller channel 0 interrupt" },
        { 0x00E, "DMA_INT_CH1"     , "DMA controller channel 1 interrupt" },
        { 0x010, "DMA_INT_CH2"     , "DMA controller channel 2 interrupt" },
        { 0x012, "DMA_INT_CH3"     , "DMA controller channel 3 interrupt" },
        { 0x014, "RTC_INT_OVF"     , "Real time counter overflow interrupt" },
        { 0x016, "RTC_INT_COMP"    , "Real time counter compare interrupt" },
        { 0x018, "TWIC_INT_SLAVE"  , "Two-Wire Interface on Port C slave interrupt" },
        { 0x01A, "TWIC_INT_MASTER" , "Two-Wire Interface on Port C master interrupt" },
        { 0x01C, "TCC0_INT_OVF"    , "Timer/counter 0 on port C interrupt" },
        { 0x01E, "TCC0_INT_ERR"    , "Timer/counter 0 on port C interrupt" },
        { 0x020, "TCC0_INT_CCA"    , "Timer/counter 0 on port C interrupt" },
        { 0x022, "TCC0_INT_CCB"    , "Timer/counter 0 on port C interrupt" },
        { 0x024, "TCC0_INT_CCC"    , "Timer/counter 0 on port C interrupt" },
        { 0x026, "TCC0_INT_CCD"    , "Timer/counter 0 on port C interrupt" },
        { 0x028, "TCC1_INT_OVF"    , "Timer/counter 1 on port C interrupt" },
        { 0x02A, "TCC1_INT_ERR"    , "Timer/counter 1 on port C interrupt" },
        { 0x02C, "TCC1_INT_CCA"    , "Timer/counter 1 on port C interrupt" },
        { 0x02E, "TCC1_INT_CCB"    , "Timer/counter 1 on port C interrupt" },
        { 0x030, "SPIC_INT"        , "SPI on port C interrupt" },
        { 0x032, "USARTC0_INT_RXC" , "USART 0 on port C receive complete interrupt" },
        { 0x034, "USARTC0_INT_DRE" , "USART 0 on port C data register empty interrupt" },
        { 0x036, "USARTC0_INT_TXC" , "USART 0 on port C transmit complete interrupt" },
        { 0x038, "USARTC1_INT_RXC" , "USART 1 on port C receive complete interrupt" },
        { 0x03A, "USARTC1_INT_DRE" , "USART 1 on port C data register empty interrupt" },
        { 0x03C, "USARTC1_INT_TXC" , "USART 1 on port C transmit complete interrupt" },
        { 0x03E, "AES_INT"         , "AES interrupt" },
        { 0x040, "NVM_INT_EE"      , "Nonvolatile Memory EEPROM interrupt" },
        { 0x042, "NVM_INT_SPM"     , "Nonvolatile Memory SPM interrupt" },
        { 0x044, "PORTB_INT0"      , "Port B interrupt 0" },
        { 0x046, "PORTB_INT1"      , "Port B interrupt 1" },
        { 0x056, "PORTE_INT0"      , "Port E interrupt 0" },
        { 0x058, "PORTE_INT1"      , "Port E interrupt 1" },
        { 0x05A, "TWIE_INT_SLAVE"  , "Two-wire Interface on Port E slave interrupt" },
        { 0x05C, "TWIE_INT_MASTER" , "Two-wire Interface on Port E master interrupt" },
        { 0x05E, "TCE0_INT_OVF"    , "Timer/counter 0 on port E interrupt" },
        { 0x060, "TCE0_INT_ERR"    , "Timer/counter 0 on port E interrupt" },
        { 0x062, "TCE0_INT_CCA"    , "Timer/counter 0 on port E interrupt" },
        { 0x064, "TCE0_INT_CCB"    , "Timer/counter 0 on port E interrupt" },
        { 0x066, "TCE0_INT_CCC"    , "Timer/counter 0 on port E interrupt" },
        { 0x068, "TCE0_INT_CCD"    , "Timer/counter 0 on port E interrupt" },
        { 0x06A, "TCE1_INT_OVF"    , "Timer/counter 1 on port E interrupt" },
        { 0x06C, "TCE1_INT_ERR"    , "Timer/counter 1 on port E interrupt" },
        { 0x06E, "TCE1_INT_CCA"    , "Timer/counter 1 on port E interrupt" },
        { 0x070, "TCE1_INT_CCB"    , "Timer/counter 1 on port E interrupt" },
        { 0x074, "USARTE0_INT_RXC" , "USART 0 on port E receive complete interrupt" },
        { 0x076, "USARTE0_INT_DRE" , "USART 0 on port E data register empty interrupt" },
        { 0x078, "USARTE0_INT_TXC" , "USART 0 on port E transmit complete interrupt" },
        { 0x080, "PORTD_INT0"      , "Port D interrupt 0" },
        { 0x082, "PORTD_INT1"      , "Port D interrupt 1" },
        { 0x084, "PORTA_INT0"      , "Port A interrupt 0" },
        { 0x086, "PORTA_INT1"      , "Port A interrupt 1" },
        { 0x088, "ACA_INT_COMP0"   , "Analog Comparator 0 on Port A interrupt" },
        { 0x08A, "ACA_INT_COMP1"   , "Analog Comparator 0 on Port A interrupt" },
        { 0x08C, "ACA_INT_WINDOW"  , "Analog Comparator window on Port A interrupt" },
        { 0x08E, "ADCA_INT_CH0"    , "Analog to Digital Converter channel 0 on Port A interrupt base" },
        { 0x090, "ADCA_INT_CH1"    , "Analog to Digital Converter channel 1 on Port A interrupt base" },
        { 0x092, "ADCA_INT_CH2"    , "Analog to Digital Converter channel 2 on Port A interrupt base" },
        { 0x094, "ADCA_INT_CH3"    , "Analog to Digital Converter channel 3 on Port A interrupt base" },
        { 0x09A, "TCD0_INT_OVF"    , "Timer/counter 0 on port D interrupt" },
        { 0x09C, "TCD0_INT_ERR"    , "Timer/counter 0 on port D interrupt" },
        { 0x09E, "TCD0_INT_CCA"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A0, "TCD0_INT_CCB"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A2, "TCD0_INT_CCC"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A4, "TCD0_INT_CCD"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A6, "TCD1_INT_OVF"    , "Timer/counter 1 on port D interrupt" },
        { 0x0A8, "TCD1_INT_ERR"    , "Timer/counter 1 on port D interrupt" },
        { 0x0AA, "TCD1_INT_CCA"    , "Timer/counter 1 on port D interrupt" },
        { 0x0AC, "TCD1_INT_CCB"    , "Timer/counter 1 on port D interrupt" },
        { 0x0AE, "SPID_INT"        , "SPI on port D interrupt" },
        { 0x0B0, "USARTD0_INT_RXC" , "USART 0 on port D receive complete interrupt" },
        { 0x0B2, "USARTD0_INT_DRE" , "USART 0 on port D data register empty interrupt" },
        { 0x0B4, "USARTD0_INT_TXC" , "USART 0 on port D transmit complete interrupt" },
        { 0x0B6, "USARTD1_INT_RXC" , "USART 1 on port D receive complete interrupt" },
        { 0x0B8, "USARTD1_INT_DRE" , "USART 1 on port D data register empty interrupt" },
        { 0x0BA, "USARTD1_INT_TXC" , "USART 1 on port D transmit complete interrupt" },
        { 0x0FA, "USB_INT_BUSEVENT", "USB on port D SOF, suspend, resume, bus reset, CRC, underflow, overflow, and stall error interrupts" },
        { 0x0FC, "USB_INT_TRNCOMPL", "USB on port D Transaction complete interrupt" },
      } ;
  }

  ATxmegaAU::~ATxmegaAU()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega128A4U::ATxmega128A4U() : ATxmegaAU(0x20000/2, 0x1000, 0x2000, 0x800)
  {
  }
  ATxmega128A4U::~ATxmega128A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega64A4U::ATxmega64A4U() : ATxmegaAU(0x10000/2, 0x1000, 0x1000, 0x800)
  {
  }
  ATxmega64A4U::~ATxmega64A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega32A4U::ATxmega32A4U() : ATxmegaAU(0x8000/2, 0x1000, 0x1000, 0x400)
  {
  }
  ATxmega32A4U::~ATxmega32A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega16A4U::ATxmega16A4U() : ATxmegaAU(0x4000/2, 0x1000, 0x800, 0x400)
  {
  }
  ATxmega16A4U::~ATxmega16A4U()
  {
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
