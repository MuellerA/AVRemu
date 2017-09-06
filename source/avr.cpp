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
    switch ((_pc - pc) % _programSize)
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

  size_t Mcu::SetProgram(size_t startAddress, const std::vector<Command> &prg)
  {
    if (startAddress >= _programSize)
      return 0 ;

    size_t nCopy = prg.size() ;
    if ((startAddress + nCopy) > _programSize)
    {
      fprintf(stderr, "Mcu::SetProgram(): data too big for program memory\n") ;
      nCopy = _programSize - startAddress ;
    }
        
    std::copy(prg.begin(), prg.begin()+nCopy, _program.begin()+startAddress) ;
    return nCopy ;
  }

  size_t Mcu::SetEeprom(size_t startAddress, const std::vector<uint8> &eeprom)
  {
    if (startAddress >= _eepromSize)
      return 0 ;

    size_t nCopy = eeprom.size() ;
    if ((startAddress + nCopy) > _eepromSize)
    {
      fprintf(stderr, "Mcu::SetEeprom(): data too big for eeprom memory\n") ;
      nCopy = _eepromSize - startAddress ;
    }
    
    std::copy(eeprom.begin(), eeprom.begin()+nCopy, _eeprom.begin()+startAddress) ;
    return nCopy ;
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
      &instrLPM1, &instrLPM2, &instrLPM3, /*&instrELPM1,*/ /*&instrELPM2,*/ /*&instrELPM3,*/ &instrSPM1, /*&instrSPM2,*/ &instrIN,
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
  
  ATmega328P::ATmega328P() : ATmegaXX8(0x8000/2, 0x00e0, 0x0800, 0x0400)
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
        { 0x32, "Store Program Memory Ready" },
      } ;
  }
  ATmega328P::~ATmega328P() {}

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega168PA::ATmega168PA() : ATmegaXX8(0x4000/2, 0x00e0, 0x0400, 0x0200)
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
        { 0x32, "Store Program Memory Ready" },
      } ;
  }
  ATmega168PA::~ATmega168PA() {}

  ////////////////////////////////////////////////////////////////////////////////
  
  ATmega88PA::ATmega88PA() : ATmegaXX8(0x2000/2, 0x00e0, 0x0400, 0x0200)
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
  
  ATmega48PA::ATmega48PA() : ATmegaXX8(0x1000/2, 0x00e0, 0x0200, 0x0100)
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
  // ATtiny24A/44A/84A
  ////////////////////////////////////////////////////////////////////////////////

  ATtinyX4::ATtinyX4(std::size_t programSize, std::size_t ioSize, std::size_t dataSize, std::size_t eepromSize)
    : Mcu(programSize, ioSize, dataSize, eepromSize)
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

    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x00, "External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset" },
        { 0x01, "External Interrupt Request 0" },
        { 0x02, "Pin Change Interrupt Request 0" },
        { 0x03, "Pin Change Interrupt Request 1" },
        { 0x04, "Watchdog Time-out" },
        { 0x05, "Timer/Counter1 Capture Event" },
        { 0x06, "Timer/Counter1 Compare Match A" },
        { 0x07, "Timer/Counter1 Compare Match B" },
        { 0x08, "Timer/Counter1 Overflow" },
        { 0x09, "Timer/Counter0 Compare Match A" },
        { 0x0a, "Timer/Counter0 Compare Match B" },
        { 0x0b, "Timer/Counter0 Overflow" },
        { 0x0c, "Analog Comparator" },
        { 0x0d, "ADC Conversion Complete" },
        { 0x0e, "EEPROM Ready" },
        { 0x0f, "USI START" },
        { 0x10, "USI Overflow" },
      } ;
  }
  ATtinyX4::~ATtinyX4()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny84A::ATtiny84A() : ATtinyX4(0x2000/2, 0x40, 0x200, 0x200)
  {
  }
  ATtiny84A::~ATtiny84A()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny44A::ATtiny44A() : ATtinyX4(0x1000/2, 0x40, 0x100, 0x100)
  {
  }
  ATtiny44A::~ATtiny44A()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny24A::ATtiny24A() : ATtinyX4(0x800/2, 0x40, 0x80, 0x80)
  {
  }
  ATtiny24A::~ATtiny24A()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATtiny25/V/45/V/85/V
  ////////////////////////////////////////////////////////////////////////////////

  ATtinyX5::ATtinyX5(std::size_t programSize, std::size_t ioSize, std::size_t dataSize, std::size_t eepromSize)
    : Mcu(programSize, ioSize, dataSize, eepromSize)
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

    _knownProgramAddresses = std::map<uint32, std::string>
      {
        { 0x00, "External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset" },
        { 0x01, "External Interrupt Request 0" },
        { 0x02, "Pin Change Interrupt Request 0" },
        { 0x03, "Timer/Counter1 Compare Match A" },
        { 0x04, "Timer/Counter1 Overflow" },
        { 0x05, "Timer/Counter0 Overflow" },
        { 0x06, "EEPROM Ready" },
        { 0x07, "Analog Comparator" },
        { 0x08, "ADC Conversion Complete" },
        { 0x09, "Timer/Counter1 Compare Match B" },
        { 0x0a, "Timer/Counter0 Compare Match A" },
        { 0x0b, "Timer/Counter0 Compare Match B" },
        { 0x0c, "Watchdog Time-out" },
        { 0x0d, "USI START" },
        { 0x0e, "USI Overflow" },
      } ;
  }
  ATtinyX5::~ATtinyX5()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny85::ATtiny85() : ATtinyX5(0x2000/2, 0x40, 0x200, 0x200)
  {
  }
  ATtiny85::~ATtiny85()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny45::ATtiny45() : ATtinyX5(0x1000/2, 0x40, 0x100, 0x100)
  {
  }
  ATtiny45::~ATtiny45()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATtiny25::ATtiny25() : ATtinyX5(0x800/2, 0x40, 0x80, 0x80)
  {
  }
  ATtiny25::~ATtiny25()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATxmega128A4U / 64A4U / 32A4U / 16A4U
  ////////////////////////////////////////////////////////////////////////////////

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



  ////////////////////////////////////////////////////////////////////////////////
  // todo
  ////////////////////////////////////////////////////////////////////////////////
  /*
    ATmega8
  */
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
