////////////////////////////////////////////////////////////////////////////////
// avr.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>

#include "io.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // Types
  ////////////////////////////////////////////////////////////////////////////////
  
  class Mcu ;
  class Instruction ;

  using Command = uint16_t ; // 16 bit instruction

  ////////////////////////////////////////////////////////////////////////////////
  // Xref
  ////////////////////////////////////////////////////////////////////////////////
  
  enum class XrefType
  {
    none = 0,
    jmp  = 1,
    call = 2,
    data = 4,
  } ;
  XrefType operator|(XrefType a, XrefType b) ;
  XrefType operator|=(XrefType &a, XrefType b) ;
  XrefType operator&(XrefType a, XrefType b) ;
  XrefType operator&=(XrefType &a, XrefType b) ;

  ////////////////////////////////////////////////////////////////////////////////
  // Instruction
  ////////////////////////////////////////////////////////////////////////////////
  
  class Instruction
  {
  protected:
    Instruction(Command pattern, Command mask, const std::string &mnemonic, const std::string &description, bool isTwoWord, bool isCall) ;
    Instruction() = delete ;
    Instruction& operator=(const Instruction&) = delete ;
    virtual ~Instruction() ;

  public:
    // returns execution time
    virtual uint8_t     Ticks  (Mcu &mcu, Command cmd) const = 0 ; // clock cycles needed for instruction
    virtual void        Execute(Mcu &mcu, Command cmd) const = 0 ; // execute next instruction
    virtual void        Skip   (Mcu &mcu, Command cmd) const = 0 ; // execute next instruction
    virtual std::string Disasm (Mcu &mcu, Command cmd) const = 0 ;
    virtual XrefType    Xref   (Mcu &mcu, Command cmd, uint32_t &addr) const = 0 ;

    virtual Command     Pattern()     const { return _pattern     ; }
    virtual Command     Mask()        const { return _mask        ; }
    virtual std::string Mnemonic()    const { return _mnemonic    ; }
    virtual std::string Description() const { return _description ; }
    virtual bool        IsTwoWord()   const { return _isTwoWord   ; }
    virtual bool        IsCall()      const { return _isCall      ; }
    
  protected:
    Command     _pattern ;
    Command     _mask    ;
    std::string _mnemonic ;
    std::string _description ;
    bool        _isTwoWord ;
    bool        _isCall ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // SREG
  ////////////////////////////////////////////////////////////////////////////////
  
  enum class SREG
  {
    C = 0,
    Z = 1,
    N = 2,
    V = 3,
    S = 4,
    H = 5,
    T = 6,
    I = 7
  } ;

  inline uint8_t operator|=(uint8_t &r, SREG b) { r |= (1 << (uint8_t)b) ; return r ; }
  inline uint8_t operator& (uint8_t  r, SREG b) { return r & (1 << (uint8_t)b) ; }
  
  inline bool  operator&& (uint8_t r, SREG b) { return (r & (1<<(uint8_t)b)) == (1<<(uint8_t)b) ; }

  ////////////////////////////////////////////////////////////////////////////////
  // Mcu
  ////////////////////////////////////////////////////////////////////////////////
  
  class Mcu
  {
  public:
    class Xref
    {
    public:
      Xref(uint32_t addr)
        : _addr(addr), _type(XrefType::none) {}
      Xref(uint32_t addr, XrefType type, const std::string &label, const std::string &description)
        : _addr(addr), _type(type), _label(label), _description(description) {}

      uint32_t                     Addr()        const { return _addr        ; }
      XrefType                     Type()        const { return _type        ; }
      const std::string&           Label()       const { return _label       ; }
      const std::string&           Description() const { return _description ; }
      const std::vector<uint32_t>& Sources()     const { return _sources     ; }
      void Type(XrefType type)             { _type |= type              ; }
      void Label(const std::string &label) { _label = label             ; }
      void AddSource(uint32_t source)      { _sources.push_back(source) ; }

    private:
      uint32_t              _addr ;
      XrefType              _type ;
      std::string           _label ;
      std::string           _description ;
      std::vector<uint32_t> _sources ;
    } ;

  protected:
    struct KnownProgramAddress
    {
      uint32_t    _addr ;
      std::string _label ;
      std::string _description ;
    } ;

    class IoSP : public Io
    {
    public:
      class SPH : public Io::Register
      {
      public:
        SPH(IoSP &sp) : Register("SPH"), _sp(sp) {}
        virtual uint8_t  Get() const    { return _sp.GetHi() ; }
        virtual void     Set(uint8_t v) { _sp.SetHi(v) ; }
        virtual uint8_t  Init() const   { return _sp.Init() >> 8 ; }
      private:
        IoSP &_sp ;
      } ;
      class SPL : public Io::Register
      {
      public:
        SPL(IoSP &sp) : Register("SPL"), _sp(sp) {}
        virtual uint8_t  Get() const    { return _sp.GetLo() ; }
        virtual void     Set(uint8_t v) { _sp.SetLo(v) ; }
        virtual uint8_t  Init() const   { return _sp.Init() >> 0 ; }
      private:
        IoSP &_sp ;
      } ;

      IoSP(uint16_t init) : _u16(init), _init(init) {}
      uint16_t  operator()() const { return _u16 ; }
      uint16_t& operator()()       { return _u16 ; }
      uint8_t  GetHi() const    { return _u8[1] ; }
      void     SetHi(uint8_t v) { _u8[1] = v ; }
      uint8_t  GetLo() const    { return _u8[0] ; }
      void     SetLo(uint8_t v) { _u8[0] = v ; }
      uint16_t Init() const     { return _init ; }
    private:
      union
      {
        uint16_t _u16 ;
        uint8_t  _u8[2] ;
      } ;
      uint16_t _init ;
    } ;

    class IoSREG : public Io
    {
    public:
      class SREG : public Io::Register
      {
      public:
        SREG(IoSREG &sreg) : Register("SREG"), _sreg(sreg) {}
        virtual uint8_t  Get() const    { return _sreg.Get() ; }
        virtual void     Set(uint8_t v) { _sreg.Set(v) ; }
      private:
        IoSREG &_sreg ;
      } ;

      IoSREG() : _sreg(0x00) {}
      uint8_t  Get() const    { return _sreg ; }
      void     Set(uint8_t v) { _sreg = v ; }

    private:
      uint8_t _sreg ;
    } ;

    class Trace
    {
    public:
      Trace() : _file{0} {}

      FILE        *_file ;
      std::size_t _src ;
      std::size_t _dst ;
      uint32_t    _cnt ;
      bool        _isRet ;
      uint32_t    _lvl ;
      std::size_t _stop ;

      bool Open(const std::string &filename, std::size_t addr = 0) ;
      bool Close() ;
    } ;
    
  protected:
    Mcu(std::size_t programSize, bool isRegDataMapped, std::size_t ioSize, std::size_t dataStart, std::size_t dataSize, std::size_t eepromSize) ;
    Mcu() = delete ;
    Mcu& operator=(const Mcu&) = delete ;
  public:
    virtual ~Mcu() ;

  public:
    std::size_t ProgramSize() const { return _programSize ; }
    std::size_t DataSize()    const { return _dataSize    ; }
    std::size_t EepromSize()  const { return _eepromSize  ; }
    
    void Execute() ;
    void Skip() ;
    void Status() ;
    std::string Disasm() ;
    bool DataAddrName(uint32_t addr, std::string &name) const ;
    bool ProgAddrName(uint32_t addr, std::string &name) const ;
    Command ProgramNext() ;

    std::size_t  PC() const { return _pc ; }
    std::size_t& PC()       { return _pc ; }

    uint32_t  Ticks() const { return _ticks ; }
    
    uint8_t  Reg(uint32_t reg) const ;
    void     Reg(uint32_t reg, uint8_t value) ;
    uint16_t RegW(uint32_t reg) const ;
    void     RegW(uint32_t reg, uint16_t value) ;
    uint8_t  Io(uint32_t io) const ;
    void     Io(uint32_t io, uint8_t value) ;
    uint8_t  Data(uint32_t addr, bool resetOnError = true) const ;
    void     Data(uint32_t addr, uint8_t value, bool resetOnError = true) ;
    void     Eeprom(size_t address, uint8_t value, bool resetOnError = true) ;
    uint8_t  Eeprom(size_t address, bool resetOnError = true) const ;
    Command  Prog(uint32_t addr) const ;
    void     Prog(uint32_t addr, uint16_t Command) ;
    const Instruction* Instr(uint32_t addr) const ;
    
    uint8_t  GetSREG() const    { return _sreg.Get()  ; }
    void     SetSREG(uint8_t v) { _sreg.Set(v)  ; }
    uint16_t GetSP() const      { return _sp() ; }
    void     SetSP(uint16_t v)  { _sp() = v ; }
    uint8_t  GetSPL()  const    { return _sp.GetLo() ; }
    void     SetSPL(uint8_t v)  { _sp.SetLo(v) ; }
    uint8_t  GetSPH()  const    { return _sp.GetHi() ; }
    void     SetSPH(uint8_t v)  { _sp.SetHi(v) ; }

    void    Push(uint8_t value) ;
    uint8_t Pop() ;
    virtual void  PushPC() ;
    virtual void  PopPC() ;

    void  Break() ; // call BREAK handlers
    void  Sleep() ;
    void  WDR() ;
    void  NotImplemented(const Instruction&) ; // unimplemented instructions

    const std::vector<const Instruction*>& Instructions() const { return _instructions ; }
    const std::vector<Command>&            Program()      const { return _program      ; }
    const std::vector<Io::Register*>&      Io()           const { return _io           ; }
    
    void   ClearProgram() ;
    size_t SetProgram(size_t address, const std::vector<Command> &prg) ;
    size_t SetEeprom(size_t address, const std::vector<uint8_t> &eeprom) ;

    const Xref* XrefByAddr(uint32_t addr) const ;
    const Xref* XrefByLabel(const std::string &label) const ;
    bool        XrefAdd(const Xref &xref) ;
    bool        XrefAdd(XrefType type, uint32_t target, uint32_t source) ;
    const std::vector<Xref*>&           Xrefs() const { return _xrefs ; }
    const std::map<uint32_t   , Xref*>& XrefByAddr()  const { return _xrefByAddr  ; }
    const std::map<std::string, Xref*>& XrefByLabel() const { return _xrefByLabel ; }

    void AddBreakpoint(std::size_t addr) { _breakpoints.insert(addr) ; }
    void DelBreakpoint(std::size_t addr) { _breakpoints.erase(addr) ; }
    bool IsBreakpoint(std::size_t addr) const { return _breakpoints.find(addr) != _breakpoints.end() ; }
    bool IsBreakpoint()                 const { return _breakpoints.find(_pc ) != _breakpoints.end() ; }
    const std::set<std::size_t>& Breakpoints() const { return _breakpoints ; }
    
    bool PcIs22bit()     const { return _pcIs22Bit     ; }
    bool IsXmega()       const { return _isXMega       ; }
    bool IsTinyReduced() const { return _isTinyReduced ; }

    bool TraceOn(const std::string &filename, std::size_t addr = 0) { return _trace.Open(filename, addr) ; }
    bool TraceOff()                                                 { return _trace.Close()              ; }
    
  protected:
    void AddInstruction(const Instruction *instr) ;
    void AnalyzeXrefs() ;

  protected:
    std::size_t _pc ;
    IoSP        _sp ;
    IoSREG      _sreg ;
    uint32_t    _ticks ;
    
    std::size_t _programSize ;

    std::size_t _regSize, _regStart, _regEnd ;
    std::size_t _ioSize, _ioStart, _ioEnd ;
    std::size_t _dataSize, _dataStart, _dataEnd ;
    std::size_t _eepromSize ;

    bool _pcIs22Bit     ;
    bool _isXMega       ;
    bool _isTinyReduced ;
    
    std::vector<Command>         _program ;
    uint8_t                      _reg[0x20] ;
    std::vector<Io::Register*>   _io ;
    std::vector<uint8_t>         _data ;
    std::vector<uint8_t>         _eeprom ;
    std::vector<KnownProgramAddress> _knownProgramAddresses ;
    std::vector<Xref*>           _xrefs ;
    std::map<uint32_t, Xref*>    _xrefByAddr ;
    std::map<std::string, Xref*> _xrefByLabel ;
    std::set<std::size_t>        _breakpoints ;
    
    std::vector<const Instruction*> _instructions ;       // map cmd to instruction

    Trace _trace ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // AVR any
  ////////////////////////////////////////////////////////////////////////////////

  class ATany : public Mcu
  {
  public:
    ATany() ;
    virtual ~ATany() ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // ATmega48A/PA/88A/PA/168A/PA/328/P
  ////////////////////////////////////////////////////////////////////////////////

  class ATmegaXX8 : public Mcu
  {
  protected:
    ATmegaXX8(std::size_t programSize, std::size_t dataSize, std::size_t eepromSize) ;
    virtual ~ATmegaXX8() ;

    IoEeprom ioEeprom ;
  } ;

  class ATmega328P : public ATmegaXX8
  {
  public:
    ATmega328P() ;
    virtual ~ATmega328P() ;
  } ;

  class ATmega168PA : public ATmegaXX8
  {
  public:
    ATmega168PA() ;
    virtual ~ATmega168PA() ;
  } ;

  class ATmega88PA : public ATmegaXX8
  {
  public:
    ATmega88PA() ;
    virtual ~ATmega88PA() ;
  } ;

  class ATmega48PA : public ATmegaXX8
  {
  public:
    ATmega48PA() ;
    virtual ~ATmega48PA() ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // ATmega8A
  ////////////////////////////////////////////////////////////////////////////////

  class ATmega8A : public Mcu
  {
  public:
    ATmega8A() ;
    virtual ~ATmega8A() ;

  protected:
    IoEeprom ioEeprom ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // ATtiny24A/44A/84A
  ////////////////////////////////////////////////////////////////////////////////

  class ATtinyX4 : public Mcu
  {
  protected:
    ATtinyX4(std::size_t programSize, std::size_t dataSize, std::size_t eepromSize) ;
    virtual ~ATtinyX4() ;

  protected:
    IoEeprom ioEeprom ;
  } ;

  class ATtiny84A : public ATtinyX4
  {
  public:
    ATtiny84A() ;
    virtual ~ATtiny84A() ;
  } ;

  class ATtiny44A : public ATtinyX4
  {
  public:
    ATtiny44A() ;
    virtual ~ATtiny44A() ;
  } ;

  class ATtiny24A : public ATtinyX4
  {
  public:
    ATtiny24A() ;
    virtual ~ATtiny24A() ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // ATtiny25/V/45/V/85/V
  ////////////////////////////////////////////////////////////////////////////////

  class ATtinyX5 : public Mcu
  {
  protected:
    ATtinyX5(std::size_t programSize, std::size_t dataSize, std::size_t eepromSize) ;
    virtual ~ATtinyX5() ;

  protected:
    IoEeprom ioEeprom ;
  } ;

  class ATtiny85 : public ATtinyX5
  {
  public:
    ATtiny85() ;
    virtual ~ATtiny85() ;
  } ;

  class ATtiny45 : public ATtinyX5
  {
  public:
    ATtiny45() ;
    virtual ~ATtiny45() ;
  } ;

  class ATtiny25 : public ATtinyX5
  {
  public:
    ATtiny25() ;
    virtual ~ATtiny25() ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // ATxmega AU
  ////////////////////////////////////////////////////////////////////////////////

  class ATxmegaAU : public Mcu
  {
  protected:
    ATxmegaAU(std::size_t programSize, std::size_t dataSize, std::size_t eepromSize) ;
    virtual ~ATxmegaAU() ;

    IoXmegaUsart _usartC0 ;
    IoXmegaUsart _usartC1 ;
    IoXmegaUsart _usartD0 ;
    IoXmegaUsart _usartD1 ;
    IoXmegaUsart _usartE0 ;
  } ;

  class ATxmega128A4U : public ATxmegaAU
  {
  public:
    ATxmega128A4U() ;
    virtual ~ATxmega128A4U() ;
  } ;

  class ATxmega64A4U : public ATxmegaAU
  {
  public:
    ATxmega64A4U() ;
    virtual ~ATxmega64A4U() ;
  } ;

  class ATxmega32A4U : public ATxmegaAU
  {
  public:
    ATxmega32A4U() ;
    virtual ~ATxmega32A4U() ;
  } ;

  class ATxmega16A4U : public ATxmegaAU
  {
  public:
    ATxmega16A4U() ;
    virtual ~ATxmega16A4U() ;
  } ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
