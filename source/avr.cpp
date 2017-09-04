////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <algorithm>

#include "avr.h"
#include "instr.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // Instruction
  Instruction::Instruction(Command pattern, Command mask, const std::string &mnemonic, const std::string &description) : _pattern(pattern), _mask(mask), _mnemonic(mnemonic), _description(description)
  {
  }

  Instruction::~Instruction()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Mcu
  Mcu::Mcu(std::size_t programSize, std::size_t ioSize, std::size_t dataSize, std::size_t eepromSize)
    : _program(programSize), _io(ioSize), _data(dataSize), _eeprom(eepromSize), _instructions(0x10000)
  {
    _pc = 0 ;

    _programSize = programSize ;

    _regSize  = 32 ;
    _regStart = 0 ;
    _regEnd   = _regStart + _regSize - 1 ;

    _ioSize  = ioSize ;
    _ioStart = _regEnd + 1 ;
    _ioEnd   = _ioStart + _ioSize - 1 ;

    _dataSize  = dataSize ;
    _dataStart = _ioEnd + 1 ;
    _dataEnd   = _dataStart + _dataSize - 1 ;

    _eepromSize = eepromSize ;
  }

  Mcu::~Mcu()
  {
  }

  std::size_t Mcu::Execute()
  {
    Command cmd = _program[_pc++] ;
    _pc %= _programSize ;

    const Instruction *instr = _instructions[cmd] ;

    return instr ? instr->Execute(*this, cmd) : 0 ;
  }

  std::string Disasm_ASC(Command cmd)
  {
    std::string str ;
    str.resize(2) ;
    char ch ;
    ch = (cmd >> 0) & 0xff ; str[0] = ((' ' <= ch) && (ch <= '~')) ? ch : '.' ;
    ch = (cmd >> 8) & 0xff ; str[1] = ((' ' <= ch) && (ch <= '~')) ? ch : '.' ;
    return str ;
  }

  std::string Mcu::Disasm()
  {
    size_t pc = _pc ;
    Command cmd = _program[_pc++] ;
    _pc %= _programSize ;

    auto iKnownAddr = _knownProgramAddresses.find(pc) ;
    std::string knownAddress = (iKnownAddr != _knownProgramAddresses.end()) ? iKnownAddr->second + ":\n" : "" ;      
    
    const Instruction *instr = _instructions[cmd] ;
    if (!instr)
    {
      char buff[1024] ;
      sprintf(buff, "%s%05lx:   %s     %04x          ???", knownAddress.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), _program[pc]) ;
      return std::string(buff) ;
    }

    std::string instrDisasm = instr->Disasm(*this, cmd) ;

    char buff[1024] ;
    switch (_pc - pc)
    {
    case 1:
      sprintf(buff, "%s%05lx:   %s     %04x          ", knownAddress.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), _program[pc]) ;
      break ;
    case 2:
      sprintf(buff, "%s%05lx:   %s%s   %04x %04x     ", knownAddress.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), Disasm_ASC(_program[pc+1]).c_str(), _program[pc], _program[pc+1]) ;
      break ;
    }
    std::string disasm(buff) ;

    return disasm + instrDisasm ;
  }

  Command Mcu::ProgramNext()
  {
    Command cmd = _program[_pc++] ;
    _pc %= _programSize ;

    return cmd ;
  }

  void Mcu::AddInstruction(const Instruction *instr)
  {
    // loop should be optimized considering mask()
    for (uint32 m = 0 ; m < 0x10000 ; ++m)
    {
      if (!(m & instr->Mask()))
      {
        Command cmd = instr->Pattern() | m ;
        if (!_instructions[cmd])
          _instructions[cmd] = instr ;
        else
        {
          //const AVR::Instruction *instr2 = _instructions[cmd] ;
          //printf("double instructions: %04x\n   %04x %04x %s\n   %04x %04x %s\n", cmd,
          //       instr->Pattern(), instr->Mask(), instr->Mnemonic().c_str(),
          //       instr2->Pattern(), instr2->Mask(), instr2->Mnemonic().c_str()) ;
        }
      }
    }
  }

  void Mcu::SetProgram(size_t address, const std::vector<Command> &prg)
  {
    if (address >= _programSize)
      return ;

    for (Command cmd : prg)
    {
      _program[address++] = cmd ;
      if (address >= _programSize)
        return ;
    }
  }

  void Mcu::SetEeprom(size_t address, const std::vector<uint8> &eeprom)
  {
    if (address >= _eepromSize)
      return ;

    for (uint8 byte : eeprom)
    {
      _eeprom[address++] = byte ;
      if (address >= _eepromSize)
        return ;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATany
  ////////////////////////////////////////////////////////////////////////////////

  ATany::ATany() : Mcu(0x40000, 0x1000, 0x1000, 0x1000)
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
  }

  ATany::~ATany()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATmega48A/PA/88A/PA/168A/PA/328/P
  ////////////////////////////////////////////////////////////////////////////////

  ATmegaXX8::ATmegaXX8(std::size_t programSize, std::size_t ioSize, std::size_t dataSize, std::size_t eepromSize)
    : Mcu(programSize, ioSize, dataSize, eepromSize)
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
      &instrLPM1, &instrLPM2, &instrLPM3, /*&instrELPM1,*/ /*&instrELPM2,*/ /*&instrELPM3,*/ &instrSPM1, &instrSPM2, &instrIN,
      &instrOUT, &instrPUSH, &instrPOP, /*&instrXCH,*/ /*&instrLAS,*/ /*&instrLAC,*/ /*&instrLAT,*/

      &instrLSR, &instrROR, &instrASR, &instrSWAP, &instrBSET, &instrBCLR, &instrSBI, &instrCBI, &instrBST,
      &instrBLD,

      &instrBREAK, &instrNOP, &instrSLEEP, &instrWDR,
    } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;
  }
  ATmegaXX8::~ATmegaXX8()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega328P::ATmega328P() : ATmegaXX8(0x4000, 0x00e0, 0x0800, 0x0400)
  {
    const Instruction *instructions[] { &instrJMP, &instrCALL, } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x00, "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x02, "External Interrupt Request 0" },
        { 0x04, "External Interrupt Request 1" },
        { 0x06, "Pin Change Interrupt Request 0" },
        { 0x08, "Pin Change Interrupt Request 1" },
        { 0x0a, "Pin Change Interrupt Request 2" },
        { 0x0c, "Watchdog Time-out Interrupt" },
        { 0x0e, "Timer/Counter2 Compare Match A" },
        { 0x10, "Timer/Counter2 Compare Match B" },
        { 0x12, "Timer/Counter2 Overflow" },
        { 0x14, "Timer/Counter1 Capture Event" },
        { 0x16, "Timer/Counter1 Compare Match A" },
        { 0x18, "Timer/Coutner1 Compare Match B" },
        { 0x1a, "Timer/Counter1 Overflow" },
        { 0x1c, "Timer/Counter0 Compare Match A" },
        { 0x1e, "Timer/Counter0 Compare Match B" },
        { 0x20, "Timer/Counter0 Overflow" },
        { 0x22, "SPI Serial Transfer Complete" },
        { 0x24, "USART Rx Complete" },
        { 0x26, "USART, Data Register Empty" },
        { 0x28, "USART, Tx Complete" },
        { 0x2a, "ADC Conversion Complete" },
        { 0x2c, "EEPROM Ready" },
        { 0x2e, "Analog Comparator" },
        { 0x30, "2-wire Serial Interface" },
        { 0x31, "Store Program Memory Ready" },
      } ;
  }
  ATmega328P::~ATmega328P() {}

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega168PA::ATmega168PA() : ATmegaXX8(0x2000, 0x00e0, 0x0400, 0x0200)
  {
    const Instruction *instructions[] { &instrJMP, &instrCALL, } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x00, "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x02, "External Interrupt Request 0" },
        { 0x04, "External Interrupt Request 1" },
        { 0x06, "Pin Change Interrupt Request 0" },
        { 0x08, "Pin Change Interrupt Request 1" },
        { 0x0a, "Pin Change Interrupt Request 2" },
        { 0x0c, "Watchdog Time-out Interrupt" },
        { 0x0e, "Timer/Counter2 Compare Match A" },
        { 0x10, "Timer/Counter2 Compare Match B" },
        { 0x12, "Timer/Counter2 Overflow" },
        { 0x14, "Timer/Counter1 Capture Event" },
        { 0x16, "Timer/Counter1 Compare Match A" },
        { 0x18, "Timer/Coutner1 Compare Match B" },
        { 0x1a, "Timer/Counter1 Overflow" },
        { 0x1c, "Timer/Counter0 Compare Match A" },
        { 0x1e, "Timer/Counter0 Compare Match B" },
        { 0x20, "Timer/Counter0 Overflow" },
        { 0x22, "SPI Serial Transfer Complete" },
        { 0x24, "USART Rx Complete" },
        { 0x26, "USART, Data Register Empty" },
        { 0x28, "USART, Tx Complete" },
        { 0x2a, "ADC Conversion Complete" },
        { 0x2c, "EEPROM Ready" },
        { 0x2e, "Analog Comparator" },
        { 0x30, "2-wire Serial Interface" },
        { 0x31, "Store Program Memory Ready" },
      } ;
  }
  ATmega168PA::~ATmega168PA() {}

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega88PA::ATmega88PA() : ATmegaXX8(0x1000, 0x00e0, 0x0400, 0x0200)
  {
    // ignoring BOOTRST / IVSEL Fuses
    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x00, "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x01, "External Interrupt Request 0" },
        { 0x02, "External Interrupt Request 1" },
        { 0x03, "Pin Change Interrupt Request 0" },
        { 0x04, "Pin Change Interrupt Request 1" },
        { 0x05, "Pin Change Interrupt Request 2" },
        { 0x06, "Watchdog Time-out Interrupt" },
        { 0x07, "Timer/Counter2 Compare Match A" },
        { 0x08, "Timer/Counter2 Compare Match B" },
        { 0x09, "Timer/Counter2 Overflow" },
        { 0x0a, "Timer/Counter1 Capture Event" },
        { 0x0b, "Timer/Counter1 Compare Match A" },
        { 0x0c, "Timer/Coutner1 Compare Match B" },
        { 0x0d, "Timer/Counter1 Overflow" },
        { 0x0e, "Timer/Counter0 Compare Match A" },
        { 0x0f, "Timer/Counter0 Compare Match B" },
        { 0x10, "Timer/Counter0 Overflow" },
        { 0x11, "SPI Serial Transfer Complete" },
        { 0x12, "USART Rx Complete" },
        { 0x13, "USART, Data Register Empty" },
        { 0x14, "USART, Tx Complete" },
        { 0x15, "ADC Conversion Complete" },
        { 0x16, "EEPROM Ready" },
        { 0x17, "Analog Comparator" },
        { 0x18, "2-wire Serial Interface" },
        { 0x19, "Store Program Memory Ready" },
      } ;
  }
  ATmega88PA::~ATmega88PA() {}

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega48PA::ATmega48PA() : ATmegaXX8(0x0800, 0x00e0, 0x0200, 0x0100)
  {
    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x00, "External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset" },
        { 0x01, "External Interrupt Request 0" },
        { 0x02, "External Interrupt Request 1" },
        { 0x03, "Pin Change Interrupt Request 0" },
        { 0x04, "Pin Change Interrupt Request 1" },
        { 0x05, "Pin Change Interrupt Request 2" },
        { 0x06, "Watchdog Time-out Interrupt" },
        { 0x07, "Timer/Counter2 Compare Match A" },
        { 0x08, "Timer/Counter2 Compare Match B" },
        { 0x09, "Timer/Counter2 Overflow" },
        { 0x0a, "Timer/Counter1 Capture Event" },
        { 0x0b, "Timer/Counter1 Compare Match A" },
        { 0x0c, "Timer/Coutner1 Compare Match B" },
        { 0x0d, "Timer/Counter1 Overflow" },
        { 0x0e, "Timer/Counter0 Compare Match A" },
        { 0x0f, "Timer/Counter0 Compare Match B" },
        { 0x10, "Timer/Counter0 Overflow" },
        { 0x11, "SPI Serial Transfer Complete" },
        { 0x12, "USART Rx Complete" },
        { 0x13, "USART, Data Register Empty" },
        { 0x14, "USART, Tx Complete" },
        { 0x15, "ADC Conversion Complete" },
        { 0x16, "EEPROM Ready" },
        { 0x17, "Analog Comparator" },
        { 0x18, "2-wire Serial Interface" },
        { 0x19, "Store Program Memory Ready" },
      } ;
  }
  ATmega48PA::~ATmega48PA() {}
  
  ////////////////////////////////////////////////////////////////////////////////
  // todo
  ////////////////////////////////////////////////////////////////////////////////
  /*
    ATmega8  (0x1000, 0x0040, 0x0800, 0x0200) ;

    ATtiny* ;

    ATxmega*
  */
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
