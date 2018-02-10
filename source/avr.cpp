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
  Mcu::Mcu(uint32_t flashSize, uint32_t ioSize, uint32_t ramSize, uint32_t eepromSize, uint32_t sp)
    : _pc(0), _sp(sp),
      _flashSize(flashSize), _loadedFlashSize(0), _flash(_flashSize),
      _ioSize(ioSize), _io(_ioSize),
      _ramSize(ramSize), _ram(_ramSize),
      _eepromSize(eepromSize), _eeprom(_eepromSize),
      _instructions(0x10000)
  {
    _pcIs22Bit     = false ;
    _isXMega       = false ;
    _isTinyReduced = false ;
  }

  Mcu::~Mcu()
  {
    for (auto iIo : _io)
      if (iIo)
        delete (iIo) ;
    for (auto iXref : _xrefs)
      delete iXref ;

    if (_trace._file)
      _trace.Close() ;
  }

  void Mcu::Execute()
  {
    _ticks += 1 ;

    if (_pc >= _flashSize)
    {
      fprintf(stderr, "invalid program memory read at %05x\n", _pc) ;
      _pc = 0 ;
      return ;
    }

    if (_pc >= _loadedFlashSize)
      fprintf(stderr, "uninitialized program memory read at %05x\n", _pc) ;
    
    uint32_t pc0 = _pc ;
    Command cmd = (_pc < _loadedFlashSize) ? _flash[_pc++] : 0x9508 ;
    const Instruction *instr = _instructions[cmd] ;
    
    if (!instr)
    {
      fprintf(stderr, "illegal instruction at %05x\n", _pc) ;
      _pc = 0 ;
      return ;
    }

    // todo Ticks
    instr->Execute(*this, cmd) ;

    if (_trace._file)
    {
      if (_pc != (pc0 + (instr->IsTwoWord() ? 2 : 1)))
      {
        if ((pc0 != _trace._src) ||
            (_pc != _trace._dst))
        {
          fprintf(_trace._file, "%2d", _trace._lvl) ;
          for (uint32_t i = 0 ; i < _trace._lvl ; ++i)
            fputs("  ", _trace._file) ;
          fprintf(_trace._file, "%05x -> %05x %4ux", _trace._src, _trace._dst, _trace._cnt) ;
          if (_trace._isRet)
            fprintf(_trace._file, "   RET") ;
          auto iXrefs = _xrefByAddr.find(_trace._dst) ;
          if (iXrefs != _xrefByAddr.end())
          {
            const Xref *xref = iXrefs->second ;
            fprintf(_trace._file, "   %s\n", xref->Label().c_str()) ;
          
            if (static_cast<uint32_t>(xref->Type() & XrefType::call))
            {
              if (_trace._lvl < 20)
                _trace._lvl++ ;
              fputc('\n', _trace._file) ;
              fprintf(_trace._file, "%2d", _trace._lvl) ;
              for (uint32_t i = 0 ; i < _trace._lvl ; ++i)
                fputs("  ", _trace._file) ;
              fprintf(_trace._file, "%s", xref->Label().c_str()) ;
              if (!xref->Description().empty())
                fprintf(_trace._file, " | %s", xref->Description().c_str()) ;
              fprintf(_trace._file, "\n") ;
            }
          }
          else
            fprintf(_trace._file, "\n") ;
          if (_trace._isRet)
          {
            fprintf(_trace._file, "\n") ;
            if (_trace._lvl > 0)
              _trace._lvl-- ;
          }
          _trace._src = pc0 ;
          _trace._dst = _pc ;
          _trace._cnt = 1 ;
          _trace._isRet = instr == &instrRET ;
        }
        else
        {
          _trace._cnt++ ;
        }
      }

      if (_pc == _trace._stop)
      {
        fprintf(stderr, "trace file closed\n") ;
        _trace.Close() ;
      }
    }
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
    
    printf(" %02x %02x %02x %02x  %02x %02x %02x %02x\n",
                _reg[0], _reg[1], _reg[2], _reg[3], _reg[4], _reg[5], _reg[6], _reg[7]) ;

    printf("       SP: %04x ", _sp()) ;

    printf(" %02x %02x %02x %02x  %02x %02x %02x %02x\n",
                _reg[8], _reg[9], _reg[10], _reg[11], _reg[12], _reg[13], _reg[14], _reg[15]) ;

    printf("                ") ;

    printf(" %02x %02x %02x %02x  %02x %02x %02x %02x\n",
                _reg[16], _reg[17], _reg[18], _reg[19], _reg[20], _reg[21], _reg[22], _reg[23]) ;
    printf("                ") ;
    printf(" %02x %02x %02x %02x  %02x %02x %02x %02x\n",
                _reg[24], _reg[25], _reg[26], _reg[27], _reg[28], _reg[29], _reg[30], _reg[31]) ;
  }

  void Mcu::Skip()
  {
    if (_pc >= _flashSize)
    {
      fprintf(stderr, "illegal program memory read at %05x\n", _pc) ;
      _pc = 0 ;
      return ;
    }

    Command cmd = _flash[_pc++] ;
    const Instruction *instr = _instructions[cmd] ;

    if (!instr)
    {
      fprintf(stderr, "illegal instruction at %05x: %04x\n", _pc, cmd) ;
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
    if (_pc >= _flashSize)
    {
      fprintf(stderr, "illegal program memory read at %05x\n", _pc) ;
      return "" ;
    }

    uint32_t pc = _pc ;
    Command cmd = _flash[_pc++] ;

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
    sprintf(buff, "%05x:   ", pc) ;
    str += buff ;
    if (instr && instr->IsTwoWord())
    {
      str += Disasm_ASC(_flash[pc]) ;
      str += Disasm_ASC(_flash[pc+1]) ;
      sprintf(buff, "   %04x %04x     ", _flash[pc], _flash[pc+1]) ;
      str += buff ;
    }
    else
    {
      str += Disasm_ASC(_flash[pc]) ;
      str += "  " ;
      sprintf(buff, "   %04x          ", _flash[pc]) ;
      str += buff ;
    }

    str += (instr) ? instr->Disasm(*this, cmd) : "???" ;

    return str ;
  }

  bool Mcu::IoName(uint32_t addr, std::string &name) const
  {
    static std::string reserved("Reserved") ;

    if (addr >_ioSize)
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
    if (_pc >= _flashSize)
    {
      fprintf(stderr, "illegal program memory read at %05x\n", _pc) ;
      return 0 ;
    }
    Command cmd = _flash[_pc++] ;

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
      fprintf(stderr, "illegal IO Register read at %05x: 0x%02x\n", _pc, io) ;
      return 0xff ;
    }
    return ioReg->Get() ;
  }
  void   Mcu::Io(uint32_t io, uint8_t value)
  {
    Io::Register *ioReg = _io[io] ;
    if (!ioReg)
    {
      fprintf(stderr, "illegal IO Register write at %05x: 0x%02x\n", _pc, io) ;
      return ;
    }
    ioReg->Set(value) ;
  }

  uint8_t  Mcu::Ram(uint32_t addr) const
  {
    if (addr < _ramSize)
      return _ram[addr] ;

    fprintf(stderr, "illegal RAM read at %05x: %04x\n", _pc, addr) ;
    return 0xff ;
  }

  void     Mcu::Ram(uint32_t addr, uint8_t value)
  {
    if (addr < _ramSize)
    {
      _ram[addr] = value ;
      return ;
    }

    fprintf(stderr, "illegal RAM write at %05x: %05x, %02x\n", _pc, addr, value) ;
  }

  void Mcu::Eeprom(uint32_t address, uint8_t value, bool resetOnError)
  {
    if (address < _eepromSize)
    {
      _eeprom[address] = value ;
      return ;
    }
    
    fprintf(stderr, "illegal eeprom write at %05x: %05x, %02x\n", _pc, address, value) ;
    //if (resetOnError)
    //  _pc = 0 ;
  }
  
  uint8_t Mcu::Eeprom(uint32_t address, bool resetOnError) const
  {
    if (address < _eepromSize)
      return _eeprom[address] ;

    fprintf(stderr, "illegal eeprom read at %05x: %04x\n", _pc, address) ;
    //if (resetOnError)
    //  const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }  

  Command  Mcu::Flash(uint32_t addr) const
  {
    if (addr < _loadedFlashSize)
      return _flash[addr] ;

    if (addr < _flashSize)
      fprintf(stderr, "uninitialized program memory read at %05x: %05x\n", _pc, addr) ;
    else
      fprintf(stderr, "invalid program memory read at %05x\n", addr) ;
    
    return 0xffff ;
  }
  
  void     Mcu::Flash(uint32_t addr, Command cmd)
  {
    if (addr < _flashSize)
    {
      _flash[addr] = cmd ;
      return ;
    }

    fprintf(stderr, "invalid program memory write at %05x: %05x %04x\n", _pc, addr, cmd) ;
  }

  uint8_t  Mcu::Data(uint32_t addr, bool resetOnError) const
  {
    if (addr < 0x32)
    {
      return Reg(addr) ;
    }
    if (addr < (0x32 + _ioSize))
    {
      return Io(addr - 0x32) ;
    }
    else if (addr <= (0x32 + _ioSize + _ramSize))
    {
      return _ram[addr - 0x32 - _ioSize] ;
    }

    fprintf(stderr, "illegal data read at %05x: %04x\n", _pc, addr) ;
    //if (resetOnError)
    //  const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }

  void Mcu::Data(uint32_t addr, uint8_t value, bool resetOnError)
  {
    if (addr < 0x32)
    {
      Reg(addr, value) ;
      return ;
    }
    if (addr < (0x32 + _ioSize))
    {
      Io(addr - 0x32, value) ;
      return ;
    }
    else if (addr <= (0x32 + _ioSize + _ramSize))
    {
      _ram[addr - 0x32 - _ioSize] = value ;
      return ;
    }

    fprintf(stderr, "illegal data write at %05x: %05x, %02x\n", _pc, addr, value) ;
    //if (resetOnError)
    //  _pc = 0 ;
  }

  Command  Mcu::Program(uint32_t addr) const
  {
    return Flash(addr) ;
  }
  
  void Mcu::Program(uint32_t addr, Command cmd)
  {
    Flash(addr, cmd) ;
  }

  bool Mcu::InRam(uint32_t addr) const
  {
    return (0x32 + _ioSize <= addr) && (addr < 0x32 + _ioSize + _ramSize) ;
  }  
  
  const Instruction* Mcu::Instr(uint32_t addr) const
  {
    Command cmd = Flash(addr) ;
    return _instructions[cmd] ;
  }
  
  void  Mcu::Push(uint8_t value)
  {
    uint16_t sp = _sp() ;
    if (!InRam(sp))
    {
      fprintf(stderr, "stack underflow at %05x\n", _pc) ;
      return ;
    }
    Data(sp, value) ;
    _sp() = sp - 1 ;
  }

  uint8_t Mcu::Pop()
  {
    uint16_t sp = _sp() + 1 ;
    if (!InRam(sp))
    {
      fprintf(stderr, "stack overflow at %05x\n", _pc) ;
      return 0xff ;
    }
    _sp() = sp ;
    return Data(sp) ;
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
    fprintf(stderr, "not implemented instruction at %05x: %s %s\n", _pc, instr.Mnemonic().c_str(), instr.Description().c_str()) ;
    // todo
  }

  void Mcu::ClearFlash()
  {
    for (auto &iPrg :_flash)
      iPrg = 0 ;
    for (auto iXref : _xrefs)
      delete iXref ;
    _xrefs.clear() ;
    _xrefByAddr.clear() ;
    _xrefByLabel.clear() ;
  }

  uint32_t Mcu::SetFlash(uint32_t startAddress, const std::vector<Command> &prg)
  {
    if (startAddress >= _flashSize)
      return 0 ;

    uint32_t nCopy = prg.size() ;
    if ((startAddress + nCopy) > _flashSize)
    {
      fprintf(stderr, "Mcu::SetProgram(): data too big for program memory\n") ;
      nCopy = _flashSize - startAddress ;
    }

    std::copy(prg.begin(), prg.begin()+nCopy, _flash.begin()+startAddress) ;
    _loadedFlashSize = nCopy + startAddress ;
    
    AnalyzeXrefs() ;

    return nCopy ;
  }

  uint32_t Mcu::SetEeprom(uint32_t startAddress, const std::vector<uint8_t> &eeprom)
  {
    if (startAddress >= _eepromSize)
      return 0 ;

    uint32_t nCopy = eeprom.size() ;
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
    for (_pc = 0 ; _pc < _flashSize ; )
    {
      uint32_t pc = _pc ;
      uint32_t addr ;
      Command cmd = _flash[_pc++] ;

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
  // Trace
  ////////////////////////////////////////////////////////////////////////////////
  
  bool Mcu::Trace::Open(const std::string &filename, uint32_t stopAddr)
  {
    if (_file)
    {
      fprintf(stderr, "trace file already open\n") ;
      return false ;
    }
    
    _file = fopen(filename.c_str(), "w") ;
    if (!_file)
    {
      fprintf(stderr, "trace file open failed\n") ;
      return false ;
    }
    _src   = 0 ;
    _dst   = 0 ;
    _cnt   = 1 ;
    _isRet = false ;
    _lvl   = 0 ;
    _stop  = stopAddr ;

    return true ;
  }
  
  bool Mcu::Trace::Close()
  {
    if (!_file)
    {
      fprintf(stderr, "trace file not open\n") ;
      return false ;
    }
    fclose(_file) ;
    _file = 0 ;
    return true ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ATany
  ////////////////////////////////////////////////////////////////////////////////

  ATany::ATany() : Mcu(0x40000, 0x1000, 0x1000, 0x1000, 0x1fff)
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
