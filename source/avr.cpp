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
  Instruction::Instruction(Command pattern, Command mask, const std::string &mnemonic, const std::string &description, bool isTwoWord, bool isCall) : _pattern(pattern), _mask(mask), _mnemonic(mnemonic), _description(description), _isTwoWord(isTwoWord), _isCall(isCall)
  {
  }

  Instruction::~Instruction()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Xref
  XrefType operator|(XrefType a, XrefType b)
  {
    return static_cast<XrefType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)) ;
  }
  XrefType operator|=(XrefType &a, XrefType b)
  {
    a = static_cast<XrefType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)) ;
    return a ;
  }
  XrefType operator&(XrefType a, XrefType b)
  {
    return static_cast<XrefType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) ;
  }
  XrefType operator&=(XrefType &a, XrefType b)
  {
    a = static_cast<XrefType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) ;
    return a ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Mcu
  Mcu::Mcu(std::size_t programSize, bool isRegDataMapped, std::size_t ioSize, std::size_t dataStart, std::size_t dataSize, std::size_t eepromSize)
    : _pc(0), _sp(dataStart+dataSize-1), _program(programSize), _io(ioSize), _data(dataSize), _eeprom(eepromSize), _instructions(0x10000)
  {
    _programSize = programSize ;

    _regSize  = 0x20 ;
    _regStart = (isRegDataMapped) ? 0 : 1 ;
    _regEnd   = (isRegDataMapped) ? _regStart + _regSize - 1 : 0 ;
    
    _ioSize  = ioSize ;
    _ioStart = (isRegDataMapped) ? _regEnd + 1 : 0 ;
    _ioEnd   = _ioStart + _ioSize - 1 ;
    
    _dataSize  = dataSize ;
    _dataStart = dataStart ;
    _dataEnd   = _dataStart + _dataSize - 1 ;

    _pcIs22Bit     = false ;
    _isXMega       = false ;
    _isTinyReduced = false ;
    
    _eepromSize = eepromSize ;
#ifdef DEBUG
    _log = fopen("AVRemu.log", "w") ;
#endif
  }

  Mcu::~Mcu()
  {
    for (auto iIo : _io)
      if (iIo)
        delete (iIo) ;
    for (auto iXref : _xrefs)
      delete iXref ;

#ifdef DEBUG
    fclose(_log) ;
#endif
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

#ifdef DEBUG
    std::size_t pc0 = _pc ;

    static struct
    {
      std::size_t src ;
      std::size_t dst ;
      uint32_t    cnt ;
      bool        isRet ;
      uint32_t    lvl ;
    } trace{0, 0, 1, false, 0} ;
#endif
    
    Command cmd = _program[_pc++] ;
    const Instruction *instr = _instructions[cmd] ;
    
    if (!instr)
    {
      fprintf(stderr, "illegal instruction\n") ;
      _pc = 0 ;
      return ;
    }

    // todo Ticks
    instr->Execute(*this, cmd) ;

#ifdef DEBUG
    if (_pc != (pc0 + (instr->IsTwoWord() ? 2 : 1)))
    {
      if ((pc0 != trace.src) ||
          (_pc != trace.dst))
      {
        for (uint32_t i = 0 ; i < trace.lvl ; ++i)
          fputs("  ", _log) ;
        fprintf(_log, "%05zx -> %05zx %4ux", trace.src, trace.dst, trace.cnt) ;
        if (trace.isRet)
          fprintf(_log, "   RET") ;
        auto iXrefs = _xrefByAddr.find(trace.dst) ;
        if (iXrefs != _xrefByAddr.end())
        {
          const Xref *xref = iXrefs->second ;
          fprintf(_log, "   %s\n", xref->Label().c_str()) ;
          
          if (static_cast<uint32_t>(xref->Type() & XrefType::call))
          {
            if (trace.lvl < 20)
              trace.lvl++ ;
            fputc('\n', _log) ;
            for (uint32_t i = 0 ; i < trace.lvl ; ++i)
              fputs("  ", _log) ;
            fprintf(_log, "%s", xref->Label().c_str()) ;
            if (!xref->Description().empty())
              fprintf(_log, " | %s", xref->Description().c_str()) ;
            fprintf(_log, "\n") ;
          }
        }
        else
          fprintf(_log, "\n") ;
        if (trace.isRet)
        {
          fprintf(_log, "\n") ;
          if (trace.lvl > 0)
            trace.lvl-- ;
        }
        trace.src = pc0 ;
        trace.dst = _pc ;
        trace.cnt = 1 ;
        trace.isRet = instr == &instrRET ;
      }
      else
      {
        trace.cnt++ ;
      }
    }
#endif
  }

  void Mcu::Status()
  {    
    uint8_t sreg = _sreg.Get() ;
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
    auto iXrefs = _xrefByAddr.find(pc) ;
    if (iXrefs != _xrefByAddr.end())
    {
      const auto xref = iXrefs->second ;

      label.append(xref->Label()) ;

      bool first = true ;
      for (const auto &iXref: xref->Sources())
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
      if (!xref->Description().empty())
      {
        label.append(xref->Description()) ;
        label.append("\n", 1) ;
      }
    }

    const Instruction *instr = _instructions[cmd] ;
    
    char buff[32] ;
    std::string str ;
    str += label ;
    sprintf(buff, "%05zx:   ", pc) ;
    str += buff ;
    if (instr && instr->IsTwoWord())
    {
      str += Disasm_ASC(_program[pc]) ;
      str += Disasm_ASC(_program[pc+1]) ;
      sprintf(buff, "   %04x %04x     ", _program[pc], _program[pc+1]) ;
      str += buff ;
    }
    else
    {
      str += Disasm_ASC(_program[pc]) ;
      str += "  " ;
      sprintf(buff, "   %04x          ", _program[pc]) ;
      str += buff ;
    }

    str += (instr) ? instr->Disasm(*this, cmd) : "???" ;

    return str ;
  }

  bool Mcu::DataAddrName(uint32_t addr, std::string &name) const
  {
    static std::string reserved("Reserved") ;

    if (addr >_ioEnd)
      return false ;

    name = _io[addr] ? _io[addr]->Name() : reserved ;
    return true ;
  }

  bool Mcu::ProgAddrName(uint32_t addr, std::string &name) const
  {
    auto iProgAddrName = _xrefByAddr.find(addr) ;
    if (iProgAddrName == _xrefByAddr.end())
      return false ;

    name = iProgAddrName->second->Label() ;
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

  uint8_t  Mcu::Reg(uint32_t reg) const
  {
    return _reg[reg] ;
  }
  void   Mcu::Reg(uint32_t reg, uint8_t value)
  {
    _reg[reg] = value ;
  }
  uint16_t Mcu::RegW(uint32_t reg) const
  {
    return *(uint16_t*)(_reg + reg) ;
  }
  void   Mcu::RegW(uint32_t reg, uint16_t value)
  {
    *(uint16_t*)(_reg + reg) = value ;
  }
  uint8_t  Mcu::Io(uint32_t io) const
  {
    Io::Register *ioReg = _io[io] ;
    if (!ioReg)
    {
      fprintf(stderr, "illegal IO Register access 0x%x\n", io) ;
      return 0xff ;
    }
    return ioReg->Get() ;
  }
  void   Mcu::Io(uint32_t io, uint8_t value)
  {
    Io::Register *ioReg = _io[io] ;
    if (!ioReg)
    {
      fprintf(stderr, "illegal IO Register access 0x%x\n", io) ;
      return ;
    }
    ioReg->Set(value) ;
  }

  uint8_t  Mcu::Data(uint32_t addr, bool resetOnError) const
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

    fprintf(stderr, "illegal data read at %05zx: %04x\n", _pc, addr) ;
    if (resetOnError)
      const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }

  void Mcu::Data(uint32_t addr, uint8_t value, bool resetOnError)
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

    fprintf(stderr, "illegal data write at %05zx: %04x, %02x\n", _pc, addr, value) ;
    if (resetOnError)
      _pc = 0 ;
  }

  void Mcu::Eeprom(size_t address, uint8_t value, bool resetOnError)
  {
    if (address < _eepromSize)
      _eeprom[address] = value ;
    if (resetOnError)
      _pc = 0 ;
  }
  
  uint8_t Mcu::Eeprom(size_t address, bool resetOnError) const
  {
    if (address < _eepromSize)
      return _eeprom[address] ;

    if (resetOnError)
      const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }  
  
  Command  Mcu::Prog(uint32_t addr) const
  {
    if (addr >= _programSize)
    {
      fprintf(stderr, "invalid program address\n") ;
      return 0xffff ;
    }
    return _program[addr] ;
  }
  
  void Mcu::Prog(uint32_t addr, Command cmd)
  {
    if (addr >= _programSize)
    {
      fprintf(stderr, "invalid program address\n") ;
      return ;
    }
    _program[addr] = cmd ;
  }

  const Instruction* Mcu::Instr(uint32_t addr) const
  {
    Command cmd = Prog(addr) ;
    return _instructions[cmd] ;
  }
  
  void  Mcu::Push(uint8_t value)
  {
    uint16_t sp = _sp() ;
    if ((sp < _dataStart) || (_dataEnd < sp))
    {
      fprintf(stderr, "stack underflow\n") ;
      return ;
    }
    _data[sp-_dataStart] = value ;
    _sp() = sp - 1 ;
  }

  uint8_t Mcu::Pop()
  {
    uint16_t sp = _sp() + 1 ;
    if ((sp < _dataStart) || (_dataEnd < sp))
    {
      fprintf(stderr, "stack overflow\n") ;
      return 0xff ;
    }
    _sp() = sp ;
    return _data[sp-_dataStart] ;
  }

  void Mcu::PushPC()
  {
    Push(_pc >> 0) ;
    Push(_pc >> 8) ;
    if (_pcIs22Bit)
      Push(_pc >> 16) ;
  }

  void Mcu::PopPC()
  {
    if (_pcIs22Bit)
      _pc = (Pop() << 16) | (Pop() << 8) | (Pop() << 0) ;
    else
      _pc = (Pop() << 8) | (Pop() << 0) ;      
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

  void Mcu::NotImplemented(const Instruction &instr)
  {
    fprintf(stderr, "not implemented instruction %s %s\n", instr.Mnemonic().c_str(), instr.Description().c_str()) ;
    // todo
  }

  void Mcu::ClearProgram()
  {
    for (auto &iPrg :_program)
      iPrg = 0 ;
    for (auto iXref : _xrefs)
      delete iXref ;
    _xrefs.clear() ;
    _xrefByAddr.clear() ;
    _xrefByLabel.clear() ;
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

  size_t Mcu::SetEeprom(size_t startAddress, const std::vector<uint8_t> &eeprom)
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

  const Mcu::Xref* Mcu::XrefByAddr(uint32_t addr) const
  {
    auto iXref = _xrefByAddr.find(addr) ;
    return (iXref != _xrefByAddr.end()) ? iXref->second : nullptr ;
  }
  
  const Mcu::Xref* Mcu::XrefByLabel(const std::string &label) const
  {
    auto iXref = _xrefByLabel.find(label) ;
    return (iXref != _xrefByLabel.end()) ? iXref->second : nullptr ;
  }

  bool Mcu::XrefAdd(const Xref &xref0)
  {
    Xref *xref = nullptr ;
    
    auto iXrefByAddr  = _xrefByAddr .find(xref0.Addr() ) ;
    if (iXrefByAddr != _xrefByAddr.end())
    {
      xref = iXrefByAddr->second ;

      xref->Type(xref0.Type()) ;
      _xrefByLabel.erase(xref->Label()) ;
      xref->Label(xref0.Label()) ;
      _xrefByLabel.insert(std::pair<std::string, Xref*>(xref->Label(), xref)) ;
    }
    else
    {
      xref = new Xref(xref0) ;
      _xrefs.push_back(xref) ;
      _xrefByAddr.insert(std::pair<uint32_t, Xref*>(xref->Addr() , xref)) ;
      _xrefByLabel.insert(std::pair<std::string, Xref*>(xref->Label(), xref)) ;
    }

    return true ;
  }
  
  void Mcu::AddInstruction(const Instruction *instr)
  {
    // loop should be optimized considering mask()
    for (uint32_t m = 0 ; m < 0x10000 ; ++m)
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

  bool Mcu::XrefAdd(XrefType type, uint32_t target, uint32_t source)
  {
    Xref *xref = nullptr ;
          
    auto iXref = _xrefByAddr.find(target) ;
    if (iXref != _xrefByAddr.end())
    {
      xref = iXref->second ;
    }
    else
    {
      xref = new Xref(target) ;
      _xrefs.push_back(xref) ;
      _xrefByAddr.insert(std::pair<uint32_t, Xref*>(target, xref)) ;
    }

    xref->Type(type) ;
    auto iPc = std::find(xref->Sources().begin(), xref->Sources().end(), source) ;
    if (iPc == xref->Sources().end())
      xref->AddSource(source) ;

    if (xref->Label().empty())
    {
      std::string label ;
      
      if      (static_cast<uint32_t>(xref->Type() & XrefType::call)) label.append("Fct_", 4) ;
      else if (static_cast<uint32_t>(xref->Type() & XrefType::jmp )) label.append("Lbl_", 4) ;
      else if (static_cast<uint32_t>(xref->Type() & XrefType::data)) label.append("Dat_", 4) ;
      else printf("xref type %d unknown\n", (uint32_t)xref->Type()) ;

      char buff[32] ; sprintf(buff, "%05x", xref->Addr()) ;
      label.append(buff) ;

      xref->Label(label) ;

      _xrefByLabel.insert(std::pair<std::string, Xref*>(xref->Label(), xref)) ;
    }
    return true ;
  }
  
  void Mcu::AnalyzeXrefs()
  {
    _xrefs.clear() ;

    // add known addresses
    for (const auto &iKnownAddr : _knownProgramAddresses)
    {
      Xref *xref = new Xref(iKnownAddr._addr, XrefType::jmp, iKnownAddr._label, iKnownAddr._description) ;
      _xrefs.push_back(xref) ;
      _xrefByAddr[xref->Addr()] = xref ;
      _xrefByLabel[xref->Label()] = xref ;
    }

    // check branch instructions
    uint32_t pc0 = _pc ;
    for (_pc = 0 ; _pc < _programSize ; )
    {
      uint32_t pc = _pc ;
      uint32_t addr ;
      Command cmd = _program[_pc++] ;

      const Instruction *instr = _instructions[cmd] ;
      if (instr)
      {
        XrefType xt = instr->Xref(*this, cmd, addr) ;

        if ((xt != XrefType::none) &&
            !(!instr->IsTwoWord() && (addr == _pc + 0)) &&
            !( instr->IsTwoWord() && (addr == _pc + 1)))
        {
          XrefAdd(xt, addr, pc) ;
        }
      }
    }
    
    _pc = pc0 ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATany
  ////////////////////////////////////////////////////////////////////////////////

  ATany::ATany() : Mcu(0x40000, true, 0x1000, 0x1000, 0x1000, 0x1000)
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
