////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <algorithm>

#include "avr.h"
#include "instr.h"
#include "filter.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // VerboseType

  bool operator&&(VerboseType a, VerboseType b)
  {
    return (unsigned int)a & (unsigned int)b ;
  }
  
  VerboseType operator&(VerboseType a, VerboseType b)
  {
    return (VerboseType)((unsigned int)a & (unsigned int)b) ;
  }

  VerboseType operator|(VerboseType a, VerboseType b)
  {
    return (VerboseType)((unsigned int)a | (unsigned int)b) ;
  }

  VerboseType operator~(VerboseType a)
  {
    return (VerboseType)~(unsigned int)a ;
  }
  
  VerboseType operator|=(VerboseType &a, VerboseType b)
  {
    a = a | b ;
    return a ;
  }
  
  VerboseType operator&=(VerboseType &a, VerboseType b)
  {
    a = a & b ;
    return a ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Instruction
  Instruction::Instruction(Command pattern, Command mask, const std::string &mnemonic, const std::string &description, bool isTwoWord, bool isJump, bool isBranch, bool isCall, bool isReturn) : _pattern(pattern), _mask(mask), _mnemonic(mnemonic), _description(description), _size(isTwoWord?2:1), _isJump(isJump), _isBranch(isBranch), _isCall(isCall), _isReturn(isReturn)
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
  Mcu::Mcu(const std::string &name, uint32_t flashSize, uint32_t ioSize, uint32_t ramSize, uint32_t eepromSize, uint32_t sp)
    : _name(name),
      _pc(0), _sp(sp),
      _ticks(0),
      _flashSize(flashSize), _loadedFlashSize(0), _flash(_flashSize),
      _ioSize(ioSize), _io(_ioSize),
      _ramSize(ramSize), _ram(_ramSize),
      _eepromSize(eepromSize), _eeprom(_eepromSize, 0xff),
      _instructions(0x10000),
      _trace(*this),
      _verbose(VerboseType::None)
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

    for (auto iF : _filters)
      delete iF ;
  }

  void Mcu::Execute()
  {
    if (_pc >= _flashSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "invalid program memory read at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
      _pc = 0 ;
      return ;
    }

    if (_pc >= _loadedFlashSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "uninitialized program memory read at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
    }
    
    Command cmd = (_pc < _loadedFlashSize) ? _flash[_pc] : 0x9508 ;
    const Instruction *instr = _instructions[cmd] ;
    
    if (!instr)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal instruction at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
      _pc = 0 ;
      return ;
    }

    uint32_t pc0 = _pc ;
    uint32_t pcNext = _pc + instr->Size() ;
    uint16_t sp0 = _sp() ;
    _pc += 1 ;
    
    _ticks += instr->Execute(*this, cmd) ;

    if (_pc != pcNext) // call / jump / return
    {
      if (instr->IsCall())
        _stackFrames.push_back(StackFrame(sp0, _pc)) ;

      if (instr->IsReturn() && !_stackFrames.empty())
        _stackFrames.pop_back() ;

      if (_trace())
        _trace.Add(pc0, _pc, *instr) ;
    }

    if (_trace() && (_pc == _trace.StopAddr()))
    {
      fprintf(stdout, "trace file closed\n") ;
      _trace.Close() ;
    }
  }

  void StatusBytes(const Mcu &mcu, uint32_t addr)
  {
    for (unsigned int i = 0 ; i < 0x10 ; ++i)
    {
      uint8_t b ;

      if (mcu.Data(addr+i, b))
        printf(" %02x", b) ;
      else
        printf(" --") ;
    }
  }

  void Mcu::Status()
  {
    /*
    VerboseType vt = _verbose ;
    _verbose = VerboseType::None ;
    */

    uint64_t mSec    = _ticks / 32000 ;
    uint64_t seconds = mSec    / 60 ; mSec    -= seconds * 60 ;
    uint64_t minutes = seconds / 60 ; seconds -= minutes * 60 ;
    uint64_t hours   = minutes / 60 ; minutes -= hours   * 60 ;
    
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
    
    printf(" [ 0] %02x %02x [ 2] %02x %02x [ 4] %02x %02x [ 6] %02x %02x     Ticks: %11ld\n",
           _reg[0], _reg[1], _reg[2], _reg[3], _reg[4], _reg[5], _reg[6], _reg[7],
           _ticks) ;

    printf("       SP: %04x ", _sp()) ;

    printf(" [ 8] %02x %02x [10] %02x %02x [12] %02x %02x [14] %02x %02x     Time: %02ld:%02ld:%02ld.%03ld\n",
           _reg[8], _reg[9], _reg[10], _reg[11], _reg[12], _reg[13], _reg[14], _reg[15],
           hours, minutes, seconds, mSec) ;

    printf("                ") ;
    printf(" [16] %02x %02x [18] %02x %02x [20] %02x %02x [22] %02x %02x\n",
           _reg[16], _reg[17], _reg[18], _reg[19], _reg[20], _reg[21], _reg[22], _reg[23]) ;

    printf("                ") ;
    printf(" [24] %02x %02x [26] %02x %02x [28] %02x %02x [30] %02x %02x\n",
           _reg[24], _reg[25], _reg[26], _reg[27], _reg[28], _reg[29], _reg[30], _reg[31]) ;

    /*
    if (_pcIs22Bit)
      printf("       RAMPX: %02x   X:", GetRampX()) ;
    else
      printf("       X: ") ;
    StatusBytes(*this, GetRampX() | RegW(26)) ;
    printf("\n") ;

    if (_pcIs22Bit)
      printf("       RAMPY: %02x   Y:", GetRampY()) ;
    else
      printf("       Y: ") ;
    StatusBytes(*this, GetRampY() | RegW(28)) ;
    printf("\n") ;

    if (_pcIs22Bit)
      printf("       RAMPZ: %02x   Z:", GetRampZ()) ;
    else
      printf("       Z: ") ;
    StatusBytes(*this, GetRampZ() | RegW(30)) ;
    printf("\n") ;

    printf("      SP: ") ;
    StatusBytes(*this, GetSP()) ;
    printf("\n") ;

    _verbose = vt ;
    */
  }

  uint8_t Mcu::Skip()
  {
    if (_pc >= _flashSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal program memory read at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
      _pc = 0 ;
      return 0 ;
    }

    Command cmd = _flash[_pc] ;
    const Instruction *instr = _instructions[cmd] ;

    if (!instr)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal instruction at %05x: %04x\n", _pc, cmd) ;
      Verbose(VerboseType::ProgError, buff) ;
      _pc = 0 ;
      return 0 ;
    }

    uint8_t size = instr->Size() ;
    _pc += size ;
    return size ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal program memory read at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal program memory read at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal IO Register read at %05x: 0x%02x\n", _pc, io) ;
      Verbose(VerboseType::DataError, buff) ;
      return 0xff ;
    }
    return ioReg->Get() ;
  }
  bool Mcu::Io(uint32_t addr, uint8_t &byte) const
  {
    Io::Register *ioReg = _io[addr] ;
    if (!ioReg)
    {
      return false ;
    }
    byte = ioReg->Get() ;
    return true ;
  }
  
  void   Mcu::Io(uint32_t io, uint8_t value)
  {
    Io::Register *ioReg = _io[io] ;
    if (!ioReg)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal IO Register write at %05x: 0x%02x\n", _pc, io) ;
      Verbose(VerboseType::DataError, buff) ;
      return ;
    }
    ioReg->Set(value) ;
  }

  uint8_t  Mcu::Ram(uint32_t addr) const
  {
    if (addr < _ramSize)
      return _ram[addr] ;

    char buff[80] ;
    snprintf(buff, sizeof(buff), "illegal RAM read at %05x: %04x\n", _pc, addr) ;
    Verbose(VerboseType::DataError, buff) ;
    return 0xff ;
  }

  void     Mcu::Ram(uint32_t addr, uint8_t value)
  {
    if (addr < _ramSize)
    {
      _ram[addr] = value ;
      return ;
    }

    char buff[80] ;
    snprintf(buff, sizeof(buff), "illegal RAM write at %05x: %05x, %02x\n", _pc, addr, value) ;
    Verbose(VerboseType::DataError, buff) ;
  }

  void Mcu::Eeprom(uint32_t address, uint8_t value, bool resetOnError)
  {
    if (address >= _eepromSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal eeprom write at %05x: %05x, %02x\n", _pc, address, value) ;
      Verbose(VerboseType::DataError, buff) ;
      address %= _eepromSize ;
    }

    {
      char buff[1024] ;
      char *ptr = buff ;
      
      ptr += sprintf(ptr, "EEPROM write at %05x: %04x %02x", _pc, address, value) ;
      if ((' ' < value) && (value <= '~'))
        ptr += sprintf(ptr, " %c", value) ;
      ptr += sprintf(ptr, "\n") ;
      Verbose(VerboseType::Eeprom, buff) ;
    }
      
    _eeprom[address] = value ;
    return ;
  }
  
  uint8_t Mcu::Eeprom(uint32_t address, bool resetOnError) const
  {
    if (address >= _eepromSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "illegal eeprom read at %05x: %04x\n", _pc, address) ;
      Verbose(VerboseType::DataError, buff) ;
      address %= _eepromSize ;
    }

    uint8_t v = _eeprom[address] ;
      
    {
      char buff[1024] ;
      char *ptr = buff ;
      
      ptr += sprintf(ptr, "EEPROM read at %05x: %04x %02x", _pc, address, v) ;
      if ((' ' < v) && (v <= '~'))
        ptr += sprintf(ptr, " %c", v) ;
      ptr += sprintf(ptr, "\n") ;
      Verbose(VerboseType::Eeprom, buff) ;
    }

    return v ;
  }  

  Command  Mcu::Flash(uint32_t addr) const
  {
    if (addr < _loadedFlashSize)
      return _flash[addr] ;

    if (addr < _flashSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "uninitialized program memory read at %05x: %05x\n", _pc, addr) ;
      Verbose(VerboseType::ProgError, buff) ;
    }
    else
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "invalid program memory read at %05x\n", addr) ;
      Verbose(VerboseType::ProgError, buff) ;
    }
    
    return 0xffff ;
  }
  
  void     Mcu::Flash(uint32_t addr, Command cmd)
  {
    if (addr < _flashSize)
    {
      _flash[addr] = cmd ;
      return ;
    }

    char buff[80] ;
    snprintf(buff, sizeof(buff), "invalid program memory write at %05x: %05x %04x\n", _pc, addr, cmd) ;
    Verbose(VerboseType::ProgError, buff) ;
  }

  uint8_t  Mcu::Data(uint32_t addr, bool resetOnError) const
  {
    if (addr < 0x20)
    {
      return Reg(addr) ;
    }
    if (addr < (0x20 + _ioSize))
    {
      return Io(addr - 0x20) ;
    }
    else if (addr <= (0x20 + _ioSize + _ramSize))
    {
      return _ram[addr - 0x20 - _ioSize] ;
    }

    char buff[80] ;
    snprintf(buff, sizeof(buff), "illegal data read at %05x: %04x\n", _pc, addr) ;
    Verbose(VerboseType::DataError, buff) ;
    //if (resetOnError)
    //  const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }

  bool Mcu::Data(uint32_t addr, uint8_t &byte) const
  {
    if (addr < 0x20)
    {
      byte = Reg(addr) ;
      return true ;
    }
    if (addr < (0x20 + _ioSize))
    {
      byte = Io(addr - 0x20) ;
      return true ;
    }
    else if (addr <= (0x20 + _ioSize + _ramSize))
    {
      byte = _ram[addr - 0x20 - _ioSize] ;
      return true ;
    }

    return false ;
  }
  
  void Mcu::Data(uint32_t addr, uint8_t value, bool resetOnError)
  {
    if (addr < 0x20)
    {
      Reg(addr, value) ;
      return ;
    }
    if (addr < (0x20 + _ioSize))
    {
      Io(addr - 0x20, value) ;
      return ;
    }
    else if (addr <= (0x20 + _ioSize + _ramSize))
    {
      _ram[addr - 0x20 - _ioSize] = value ;
      return ;
    }

    char buff[80] ;
    snprintf(buff, sizeof(buff), "illegal data write at %05x: %04x %02x\n", _pc, addr, value) ;
    Verbose(VerboseType::DataError, buff) ;
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
    return (0x20 + _ioSize <= addr) && (addr < 0x20 + _ioSize + _ramSize) ;
  }  
  
  void Mcu::RamRange(uint32_t &min, uint32_t &max) const
  {
    min = 0x20 + _ioSize ;
    max = 0x20 + _ioSize + _ramSize - 1 ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "stack underflow at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "stack overflow at %05x\n", _pc) ;
      Verbose(VerboseType::ProgError, buff) ;
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
    char buff[80] ;
    snprintf(buff, sizeof(buff), "not implemented instruction at %05x: %s %s\n", _pc, instr.Mnemonic().c_str(), instr.Description().c_str()) ;
    printf(buff) ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "Mcu::SetProgram(): data too big for program memory\n") ;
      Verbose(VerboseType::ProgError, buff) ;
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
      char buff[80] ;
      snprintf(buff, sizeof(buff), "Mcu::SetEeprom(): data too big for eeprom memory\n") ;
      Verbose(VerboseType::DataError, buff) ;
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

  void Mcu::Verbose(VerboseType vt, const std::string &text) const
  {
    if (_verbose && vt)
      fprintf(stdout, text.c_str()) ;

    for (auto filter : _filters)
    {
      if (filter->Verbose() && vt)
      {
        std::string fromFilter ;
        (*filter)(text, fromFilter) ;
        if (fromFilter.size())
          fprintf(stdout, "=> %s\n", fromFilter.c_str()) ;
      }
    }
  }

  void Mcu::AddFilter(VerboseType vt, const std::string &command)
  {
    _filters.push_back(new Filter(command, vt)) ;
  }
  
  void Mcu::DelFilter(pid_t pid)
  {
    _filters.erase(std::find_if(_filters.begin(), _filters.end(), [pid](const Filter *f){ return f->Pid() == pid ; })) ;    
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

  Mcu::Trace::Trace(const Mcu &mcu) : _mcu(mcu), _file{0}
  {
  }
  
  Mcu::Trace::~Trace()
  {
    if (_file)
      Close() ;
  }
  
  bool Mcu::Trace::Open(const std::string &filename, uint32_t stopAddr)
  {
    if (_file)
    {
      fprintf(stdout, "trace file already open\n") ;
      return false ;
    }
    
    _file = fopen(filename.c_str(), "w") ;
    if (!_file)
    {
      fprintf(stdout, "trace file open failed\n") ;
      return false ;
    }
    _src    = 0 ;
    _dst    = 0 ;
    _cnt    = 1 ;
    _isRet  = false ;
    _isCall = false ;
    _lvl    = 0 ;
    _stop   = stopAddr ;

    return true ;
  }
  
  bool Mcu::Trace::Close()
  {
    if (!_file)
    {
      fprintf(stdout, "trace file not open\n") ;
      return false ;
    }

    Add(0, 0, instrNOP) ;
    fclose(_file) ;
    _file = 0 ;
    return true ;
  }

  void Mcu::Trace::Add(uint32_t src, uint32_t dst, const Instruction &instr)
  {
    if ((src != _src) ||
        (dst != _dst))
    {
      fprintf(_file, "%2d  ", _lvl) ;
      for (uint32_t i = 0, e = (_lvl < 20) ? _lvl : 20 ; i < e ; ++i)
        fputs("  ", _file) ;
      fprintf(_file, "%05x -> %05x %4ux", _src, _dst, _cnt) ;
      
      if (_isRet)
      {
        fprintf(_file, "   RET\n\n") ;
        if (_lvl > 0)
          _lvl-- ;
      }
      else
      {
        auto iXrefs = _mcu.XrefByAddr().find(_dst) ;
        if (iXrefs != _mcu.XrefByAddr().end())
        {
          const Xref *xref = iXrefs->second ;
          fprintf(_file, "   %s\n", xref->Label().c_str()) ;
        
          if (_isCall)
          {
            _lvl++ ;
            fputc('\n', _file) ;
            fprintf(_file, "%2d  ", _lvl) ;
            for (uint32_t i = 0, e = (_lvl < 20) ? _lvl : 20 ; i < e ; ++i)
              fputs("  ", _file) ;
            fprintf(_file, "%s", xref->Label().c_str()) ;
            if (!xref->Description().empty())
              fprintf(_file, " | %s", xref->Description().c_str()) ;
            fprintf(_file, "\n") ;
          }
        }
        else
          fprintf(_file, "\n") ;
      }
      
      _src    = src ;
      _dst    = dst ;
      _cnt    = 1 ;
      _isRet  = instr.IsReturn() ;
      _isCall = instr.IsCall() ;
    }
    else
    {
      _cnt++ ;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // ATany
  ////////////////////////////////////////////////////////////////////////////////

  ATany::ATany() : Mcu("ATany", 0x40000, 0x1000, 0x1000, 0x1000, 0x1fff)
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
