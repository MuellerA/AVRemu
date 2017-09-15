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
    : _program(programSize), _io(ioSize+32), _data(dataSize), _eeprom(eepromSize), _instructions(0x10000)
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

    std::vector<std::pair<uint32, std::string>> ioRegs
    {
      { 0x00, "r0" },
      { 0x01, "r1" },
      { 0x02, "r2" },
      { 0x03, "r3" },
      { 0x04, "r4" },
      { 0x05, "r5" },
      { 0x06, "r6" },
      { 0x07, "r7" },
      { 0x08, "r8" },
      { 0x09, "r9" },
      { 0x0a, "r10" },
      { 0x0b, "r11" },
      { 0x0c, "r12" },
      { 0x0d, "r13" },
      { 0x0e, "r14" },
      { 0x0f, "r15" },
      { 0x10, "r16" },
      { 0x11, "r17" },
      { 0x12, "r18" },
      { 0x13, "r19" },
      { 0x14, "r20" },
      { 0x15, "r21" },
      { 0x16, "r22" },
      { 0x17, "r23" },
      { 0x18, "r24" },
      { 0x19, "r25" },
      { 0x1a, "r26" },
      { 0x1b, "r27" },
      { 0x1c, "r28" },
      { 0x1d, "r29" },
      { 0x1e, "r30" },
      { 0x1f, "r31" },
    } ;
    for (auto iIoReg: ioRegs)
    {
      _io[iIoReg.first] = new IoRegisterNotImplemented(iIoReg.second) ;
    }
  }

  Mcu::~Mcu()
  {
  }

  std::size_t Mcu::Execute()
  {
    if (_pc >= _programSize)
    {
      fprintf(stderr, "program counter overflow\n") ;
      return 0 ;
    }
    Command cmd = _program[_pc++] ;

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
    if (_pc >= _programSize)
    {
      fprintf(stderr, "program counter overflow\n") ;
      return "" ;
    }
    
    size_t pc = _pc ;
    Command cmd = _program[_pc++] ;

    std::string label ;
    auto iXrefs = _xrefs.find(pc) ;
    if (iXrefs != _xrefs.end())
    {
      const auto &xref = iXrefs->second ;

      if (xref._label.empty())
        label.append("Xref", 4) ;
      else
        label.append(xref._label) ;

      bool first = true ;
      for (auto &iXref: xref._addrs)
      {
        if (first)
        {
          first = false ;
          label.append(": ", 2) ;
        }
        else
          label.append(", ", 2) ;
        char buff[32] ;
        sprintf(buff, "%05x", iXref) ;
        label.append(buff) ;
      }
      label.append("\n", 1) ;
    }
    
    const Instruction *instr = _instructions[cmd] ;
    if (!instr)
    {
      char buff[1024] ;
      sprintf(buff, "%s%05lx:   %s     %04x          ???", label.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), _program[pc]) ;
      return std::string(buff) ;
    }

    std::string instrDisasm = instr->Disasm(*this, cmd) ;

    char buff[1024] ;
    switch (_pc - pc)
    {
    case 1:
      sprintf(buff, "%s%05lx:   %s     %04x          ", label.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), _program[pc]) ;
      break ;
    case 2:
      sprintf(buff, "%s%05lx:   %s%s   %04x %04x     ", label.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), Disasm_ASC(_program[pc+1]).c_str(), _program[pc], _program[pc+1]) ;
      break ;
    }
    std::string disasm(buff) ;

    return disasm + instrDisasm ;
  }

  bool Mcu::DataAddrName(uint32 addr, std::string &name) const
  {
    static std::string reserved("Reserved") ;

    if (addr >_ioEnd)
      return false ;

    name = _io[addr] ? _io[addr]->Name() : reserved ;
    return true ;
  }
  
  Command Mcu::ProgramNext()
  {
    if (_pc >= _programSize)
    {
      fprintf(stderr, "program counter overflow\n") ;
      return 0 ;
    }
    Command cmd = _program[_pc++] ;

    return cmd ;
  }

  void Mcu::ClearProgram()
  {
    for (auto &iPrg :_program)
      iPrg = 0 ;
    _xrefs.clear() ;
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

    AnalyzeXrefs() ;
    
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

  void Mcu::AnalyzeXrefs()
  {
    _xrefs.clear() ;

    // add known addresses
    for (auto iKnownAddr : _knownProgramAddresses)
    {
      Xref &xref = _xrefs[iKnownAddr.first] ;
      xref._addr  = iKnownAddr.first  ;
      xref._label = iKnownAddr.second ;
    }

    // check branch instructions
    uint32 pc0 = _pc ;
    for (_pc = 0 ; _pc < _programSize ; )
    {
      uint32 pc = _pc ;
      uint32 addr ;
      Command cmd = _program[_pc++] ;
      
      const Instruction *instr = _instructions[cmd] ;
      if (instr && instr->Xref(*this, cmd, addr))
      {
        auto iXrefs = _xrefs.find(addr) ;
        if (iXrefs != _xrefs.end())
        {
          Xref &xref = iXrefs->second ;
          xref._addrs.push_back(pc) ;
        }
        else
        {
          Xref &xref = _xrefs[addr] ;
          xref._addr = addr ;
          xref._addrs.push_back(pc) ;          
        }
      }      
    }
    _pc = pc0 ;
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

}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
