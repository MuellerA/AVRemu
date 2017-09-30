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
  // Xref
  XrefType operator|(XrefType a, XrefType b)
  {
    return static_cast<XrefType>(static_cast<uint32>(a) | static_cast<uint32>(b)) ;
  }
  XrefType operator|=(XrefType &a, XrefType b)
  {
    a = static_cast<XrefType>(static_cast<uint32>(a) | static_cast<uint32>(b)) ;
    return a ;
  }
  XrefType operator&(XrefType a, XrefType b)
  {
    return static_cast<XrefType>(static_cast<uint32>(a) & static_cast<uint32>(b)) ;
  }
  XrefType operator&=(XrefType &a, XrefType b)
  {
    a = static_cast<XrefType>(static_cast<uint32>(a) & static_cast<uint32>(b)) ;
    return a ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Mcu
  Mcu::Mcu(std::size_t programSize, std::size_t ioSize, std::size_t dataStart, std::size_t dataSize, std::size_t eepromSize)
    : _pc(0), _sp(dataStart+dataSize-1), _program(programSize), _io(ioSize), _data(dataSize), _eeprom(eepromSize), _instructions(0x10000)
  {
    _programSize = programSize ;

    _regSize  = 0x20 ;
    _regStart = 0 ;
    _regEnd   = _regStart + _regSize - 1 ;

    _ioSize  = ioSize ;
    _ioStart = _regEnd + 1 ;
    _ioEnd   = _ioStart + _ioSize - 1 ;

    _dataSize  = dataSize ;
    _dataStart = dataStart ;
    _dataEnd   = _dataStart + _dataSize - 1 ;
    
    _eepromSize = eepromSize ;
  }

  Mcu::~Mcu()
  {
  }

  void Mcu::Execute()
  {
    _ticks += 1 ;
    
    if (_pc >= _programSize)
    {
      fprintf(stderr, "program counter overflow\n") ;
      _pc = 0 ;
      return ;
    }

    printf("%05zx:", _pc) ;
    Command cmd = _program[_pc++] ;
    const Instruction *instr = _instructions[cmd] ;
    
    if (!instr)
    {
      fprintf(stderr, "illegal instruction\n") ;
      _pc = 0 ;
      return ;
    }

    {
      uint32 pc = _pc ;
      printf(" %s\n", instr->Disasm(*this, cmd).c_str()) ;
      _pc = pc ;
    }
    
    // todo Ticks
    instr->Execute(*this, cmd) ;
  }

  void Mcu::Status()
  {    
    uint8 sreg = _sreg() ;
    printf("       %c%c%c%c%c%c%c%c ",
           (sreg && AVR::SREG::I) ? 'I' : '_',
           (sreg && AVR::SREG::T) ? 'T' : '_',
           (sreg && AVR::SREG::H) ? 'H' : '_',
           (sreg && AVR::SREG::S) ? 'S' : '_',
           (sreg && AVR::SREG::V) ? 'V' : '_',
           (sreg && AVR::SREG::N) ? 'N' : '_',
           (sreg && AVR::SREG::Z) ? 'Z' : '_',
           (sreg && AVR::SREG::C) ? 'C' : '_') ;
    
    for (size_t iR =  0 ; iR < 8 ; ++iR)
      printf(" %02x", _reg[iR]) ;

    printf("\n       SP: %04x ", _sp()) ;

    for (size_t iR =  8 ; iR < 16 ; ++iR)
      printf(" %02x", _reg[iR]) ;
    printf("\n                ") ;
    for (size_t iR = 16 ; iR < 24 ; ++iR)
      printf(" %02x", _reg[iR]) ;
    printf("\n                ") ;
    for (size_t iR = 24 ; iR < 32 ; ++iR)
      printf(" %02x", _reg[iR]) ;

    printf("\n") ;
  }

  void Mcu::Skip()
  {
    if (_pc >= _programSize)
    {
      fprintf(stderr, "program counter overflow\n") ;
      _pc = 0 ;
      return ;
    }

    Command cmd = _program[_pc++] ;
    const Instruction *instr = _instructions[cmd] ;

    if (!instr)
    {
      fprintf(stderr, "illegal instruction\n") ;
      _pc = 0 ;
      return ;
    }

    instr->Skip(*this, cmd) ;
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

      label.append(xref._label) ;

      bool first = true ;
      for (const auto &iXref: xref._addrs)
      {
        if (first)
        {
          first = false ;
          label.append(": ", 2) ;
        }
        else
          label.append(", ", 2) ;
        std::string xrefName ;
        if (ProgAddrName(iXref, xrefName))
        {
          label.append(xrefName) ;
        }
        else
        {
          char buff[32] ;
          sprintf(buff, "%05x", iXref) ;
          label.append(buff) ;
        }
      }
      label.append("\n", 1) ;
      if (!xref._description.empty())
      {
        label.append(xref._description) ;
        label.append("\n", 1) ;
      }
    }

    const Instruction *instr = _instructions[cmd] ;
    if (!instr)
    {
      char buff[1024] ;
      sprintf(buff, "%s%05zx:   %s     %04x          ???", label.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), _program[pc]) ;
      return std::string(buff) ;
    }

    std::string instrDisasm = instr->Disasm(*this, cmd) ;

    char buff[1024] ;
    switch (_pc - pc)
    {
    case 1:
      sprintf(buff, "%s%05zx:   %s     %04x          ", label.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), _program[pc]) ;
      break ;
    case 2:
      sprintf(buff, "%s%05zx:   %s%s   %04x %04x     ", label.c_str(), pc, Disasm_ASC(_program[pc]).c_str(), Disasm_ASC(_program[pc+1]).c_str(), _program[pc], _program[pc+1]) ;
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

  bool Mcu::ProgAddrName(uint32 addr, std::string &name) const
  {
    auto iProgAddrName = _xrefs.find(addr) ;
    if (iProgAddrName == _xrefs.end())
      return false ;

    name = iProgAddrName->second._label ;
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

  uint8  Mcu::Reg(uint32 reg) const
  {
    return _reg[reg] ;
  }
  void   Mcu::Reg(uint32 reg, uint8 value)
  {
    _reg[reg] = value ;
  }
  uint16 Mcu::RegW(uint32 reg) const
  {
    return *(uint16*)(_reg + reg) ;
  }
  void   Mcu::RegW(uint32 reg, uint16 value)
  {
    *(uint16*)(_reg + reg) = value ;
  }
  uint8  Mcu::Io(uint32 io) const
  {
    Io::Register *ioReg = _io[io] ;
    if (!ioReg)
    {
      fprintf(stderr, "illegal IO Register access\n") ;
      return 0xff ;
    }
    return (*ioReg)() ;
  }
  void   Mcu::Io(uint32 io, uint8 value)
  {
    Io::Register *ioReg = _io[io] ;
    if (!ioReg)
    {
      fprintf(stderr, "illegal IO Register access\n") ;
      return ;
    }
    (*ioReg)() = value ;
  }

  uint8  Mcu::Data(uint32 addr) const
  {
    if ((_regStart <= addr) && (addr <= _regEnd))
    {
      return Reg(addr) ;
    }
    if ((_ioStart <= addr) && (addr <= _ioEnd))
    {
      return Io(addr - _ioStart) ;
    }
    else if ((_dataStart <= addr) && (addr <= _dataEnd))
    {
      return _data[addr - _dataStart] ;
    }

    fprintf(stderr, "program counter overflow\n") ;
    const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }

  void Mcu::Data(uint32 addr, uint8 value)
  {
    if ((_regStart <= addr) && (addr <= _regEnd))
    {
      Reg(addr, value) ;
      return ;
    }
    else if ((_ioStart <= addr) && (addr <= _ioEnd))
    {
      Io(addr - _ioStart, value) ;
      return ;
    }
    else if ((_dataStart <= addr) && (addr <= _dataEnd))
    {
      _data[addr - _dataStart] = value ;
      return ;
    }

    fprintf(stderr, "program counter overflow\n") ;
    _pc = 0 ;
  }

  Command  Mcu::Prog(uint32 addr) const
  {
    if (addr >= _programSize)
    {
      fprintf(stderr, "invalid program address\n") ;
      return 0xffff ;
    }
    return _program[addr] ;
  }
  
  void Mcu::Prog(uint32 addr, Command cmd)
  {
    if (addr >= _programSize)
    {
      fprintf(stderr, "invalid program address\n") ;
      return ;
    }
    _program[addr] = cmd ;
  }
  
  void  Mcu::Push(uint8 value)
  {
    uint16 sp = _sp() ;
    if ((sp < _dataStart) || (_dataEnd < sp))
    {
      fprintf(stderr, "stack underflow\n") ;
      return ;
    }
    _data[sp] = value ;
    _sp() = sp - 1 ;
  }

  uint8 Mcu::Pop()
  {
    uint16 sp = _sp() + 1 ;
    if ((sp < _dataStart) || (_dataEnd < sp))
    {
      fprintf(stderr, "stack overflow\n") ;
      return 0xff ;
    }
    _sp() = sp ;
    return _data[sp] ;
  }

  void Mcu::PushPC()
  {
    Push(_pc >> 8) ;
    Push(_pc >> 0) ;
  }

  void Mcu::PopPC()
  {
    _pc = (Pop() << 0) | (Pop() << 8) ;
  }

  void Mcu::Break()
  {
    // todo
  }

  void Mcu::Sleep()
  {
    // todo
  }

  void Mcu::WDR()
  {
    // todo
  }

  void Mcu::NotImplemented(const Instruction&)
  {
    // todo
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
    for (const auto &iKnownAddr : _knownProgramAddresses)
    {
      auto iXref = _xrefs.insert(std::pair<uint32, Xref>(iKnownAddr._addr, Xref(iKnownAddr._addr))).first ;
      Xref &xref = iXref->second ;
      xref._type  = XrefType::jmp ;
      xref._label = iKnownAddr._label ;
      xref._description = iKnownAddr._description ;
    }

    // check branch instructions
    uint32 pc0 = _pc ;
    for (_pc = 0 ; _pc < _programSize ; )
    {
      uint32 pc = _pc ;
      uint32 addr ;
      Command cmd = _program[_pc++] ;

      const Instruction *instr = _instructions[cmd] ;
      if (instr)
      {
        XrefType xt = instr->Xref(*this, cmd, addr) ;

        if (xt != XrefType::none)
        {
          auto iXrefs = _xrefs.find(addr) ;
          if (iXrefs == _xrefs.end())
            iXrefs = _xrefs.insert(std::pair<uint32, Xref>(addr, Xref(addr))).first ;

          Xref &xref = iXrefs->second ;
          xref._type |= xt ;
          xref._addrs.push_back(pc) ;
        }
      }
    }

    // create labels
    for (auto &iXref: _xrefs)
    {
      Xref &xref = iXref.second ;
      if (!xref._label.empty())
        continue ;

      if      (static_cast<uint32>(xref._type & XrefType::call)) xref._label.append("Fct_", 4) ;
      else if (static_cast<uint32>(xref._type & XrefType::jmp )) xref._label.append("Lbl_", 4) ;
      else if (static_cast<uint32>(xref._type & XrefType::data)) xref._label.append("Dat_", 4) ;
      else printf("xref type %d\n", (uint32)xref._type) ;

      char buff[32] ; sprintf(buff, "%05x", xref._addr) ;
      xref._label.append(buff) ;
    }

    _pc = pc0 ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATany
  ////////////////////////////////////////////////////////////////////////////////

  ATany::ATany() : Mcu(0x40000, 0x1000, 0x1000, 0x1000, 0x1000)
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
