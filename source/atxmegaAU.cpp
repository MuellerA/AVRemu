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

    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x000, "RESET" },
        { 0x002, "Crystal oscillator failure interrupt (NMI)" },
        { 0x004, "Port C interrupt 0" },
        { 0x006, "Port C interrupt 1" },
        { 0x008, "Port R interrupt 0" },
        { 0x00a, "Port R interrupt 1" },
        { 0x00C, "DMA controller channel 0 interrupt" },
        { 0x00E, "DMA controller channel 1 interrupt" },
        { 0x010, "DMA controller channel 2 interrupt" },
        { 0x012, "DMA controller channel 3 interrupt" },
        { 0x014, "Real time counter overflow interrupt" },
        { 0x016, "Real time counter compare interrupt" },
        { 0x018, "Two-Wire Interface on Port C slave interrupt" },
        { 0x01A, "Two-Wire Interface on Port C master interrupt" },
        { 0x01C, "Timer/counter 0 on port C interrupt" },
        { 0x01E, "Timer/counter 0 on port C interrupt" },
        { 0x020, "Timer/counter 0 on port C interrupt" },
        { 0x022, "Timer/counter 0 on port C interrupt" },
        { 0x024, "Timer/counter 0 on port C interrupt" },
        { 0x026, "Timer/counter 0 on port C interrupt" },
        { 0x028, "Timer/counter 1 on port C interrupt" },
        { 0x02A, "Timer/counter 1 on port C interrupt" },
        { 0x02C, "Timer/counter 1 on port C interrupt" },
        { 0x02E, "Timer/counter 1 on port C interrupt" },
        { 0x030, "SPI on port C interrupt" },
        { 0x032, "USART 0 on port C receive complete interrupt" },
        { 0x034, "USART 0 on port C data register empty interrupt" },
        { 0x036, "USART 0 on port C transmit complete interrupt" },
        { 0x038, "USART 1 on port C receive complete interrupt" },
        { 0x03A, "USART 1 on port C data register empty interrupt" },
        { 0x03C, "USART 1 on port C transmit complete interrupt" },
        { 0x03E, "AES interrupt" },
        { 0x040, "Nonvolatile Memory EEPROM interrupt" },
        { 0x042, "Nonvolatile Memory SPM interrupt" },
        { 0x044, "Port B interrupt 0" },
        { 0x046, "Port B interrupt 1" },
        { 0x056, "Port E interrupt 0" },
        { 0x058, "Port E interrupt 1" },
        { 0x05A, "Two-wire Interface on Port E slave interrupt" },
        { 0x05C, "Two-wire Interface on Port E master interrupt" },
        { 0x05E, "Timer/counter 0 on port E interrupt" },
        { 0x060, "Timer/counter 0 on port E interrupt" },
        { 0x062, "Timer/counter 0 on port E interrupt" },
        { 0x064, "Timer/counter 0 on port E interrupt" },
        { 0x066, "Timer/counter 0 on port E interrupt" },
        { 0x068, "Timer/counter 0 on port E interrupt" },
        { 0x06A, "Timer/counter 1 on port E interrupt" },
        { 0x06C, "Timer/counter 1 on port E interrupt" },
        { 0x06E, "Timer/counter 1 on port E interrupt" },
        { 0x070, "Timer/counter 1 on port E interrupt" },
        { 0x074, "USART 0 on port E receive complete interrupt" },
        { 0x076, "USART 0 on port E data register empty interrupt" },
        { 0x078, "USART 0 on port E transmit complete interrupt" },
        { 0x080, "Port D interrupt 0" },
        { 0x082, "Port D interrupt 1" },
        { 0x084, "Port A interrupt 0" },
        { 0x086, "Port A interrupt 1" },
        { 0x088, "Analog Comparator 0 on Port A interrupt" },
        { 0x08A, "Analog Comparator 0 on Port A interrupt" },
        { 0x08C, "Analog Comparator window on Port A interrupt" },
        { 0x08E, "Analog to Digital Converter channel 0 on Port A interrupt base" },
        { 0x090, "Analog to Digital Converter channel 1 on Port A interrupt base" },
        { 0x092, "Analog to Digital Converter channel 2 on Port A interrupt base" },
        { 0x094, "Analog to Digital Converter channel 3 on Port A interrupt base" },
        { 0x09A, "Timer/counter 0 on port D interrupt" },
        { 0x09C, "Timer/counter 0 on port D interrupt" },
        { 0x09E, "Timer/counter 0 on port D interrupt" },
        { 0x0A0, "Timer/counter 0 on port D interrupt" },
        { 0x0A2, "Timer/counter 0 on port D interrupt" },
        { 0x0A4, "Timer/counter 0 on port D interrupt" },
        { 0x0A6, "Timer/counter 1 on port D interrupt" },
        { 0x0A8, "Timer/counter 1 on port D interrupt" },
        { 0x0AA, "Timer/counter 1 on port D interrupt" },
        { 0x0AC, "Timer/counter 1 on port D interrupt" },
        { 0x0AE, "SPI on port D interrupt" },
        { 0x0B0, "USART 0 on port D receive complete interrupt" },
        { 0x0B2, "USART 0 on port D data register empty interrupt" },
        { 0x0B4, "USART 0 on port D transmit complete interrupt" },
        { 0x0B6, "USART 1 on port D receive complete interrupt" },
        { 0x0B8, "USART 1 on port D data register empty interrupt" },
        { 0x0BA, "USART 1 on port D transmit complete interrupt" },
        { 0x0FA, "USB on port D interrupt base" },
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
