////////////////////////////////////////////////////////////////////////////////
// avr.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include "io.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // Types
  ////////////////////////////////////////////////////////////////////////////////
  
  class Mcu ;
  class Instruction ;

  using Command = unsigned short ; // 16 bit instruction
  using int8    = signed   char  ; //  8 bit
  using uint8   = unsigned char  ; //  8 bit
  using int16   = signed   short ; // 16 bit
  using uint16  = unsigned short ; // 16 bit
  using int32   = signed   int   ; // 32 bit
  using uint32  = unsigned int   ; // 32 bit

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
    virtual uint8       Ticks  (Mcu &mcu, Command cmd) const = 0 ; // clock cycles needed for instruction
    virtual void        Execute(Mcu &mcu, Command cmd) const = 0 ; // execute next instruction
    virtual void        Skip   (Mcu &mcu, Command cmd) const = 0 ; // execute next instruction
    virtual std::string Disasm (Mcu &mcu, Command cmd) const = 0 ;
    virtual XrefType    Xref   (Mcu &mcu, Command cmd, uint32 &addr) const = 0 ;

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

  inline uint8 operator|=(uint8 &r, SREG b) { r |= (1 << (uint8)b) ; return r ; }
  inline uint8 operator& (uint8  r, SREG b) { return r & (1 << (uint8)b) ; }
  
  inline bool  operator&& (uint8 r, SREG b) { return (r & (1<<(uint8)b)) == (1<<(uint8)b) ; }

  ////////////////////////////////////////////////////////////////////////////////
  // Mcu
  ////////////////////////////////////////////////////////////////////////////////
  
  class Mcu
  {
  public:
    struct Xref
    {
      Xref(uint32 addr)
        : _addr(addr), _type(XrefType::none) {}
      Xref(uint32 addr, XrefType type, const std::string &label, const std::string &description)
        : _addr(addr), _type(type), _label(label), _description(description) {}

      uint32              _addr ;
      XrefType            _type ;
      std::string         _label ;
      std::string         _description ;
      std::vector<uint32> _addrs ;
    } ;

  protected:
    struct KnownProgramAddress
    {
      uint32      _addr ;
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
        virtual uint8  Get() const  { return _sp.GetHi() ; }
        virtual void   Set(uint8 v) { _sp.SetHi(v) ; }
        virtual uint8  Init() const { return _sp.Init() >> 8 ; }
      private:
        IoSP &_sp ;
      } ;
      class SPL : public Io::Register
      {
      public:
        SPL(IoSP &sp) : Register("SPL"), _sp(sp) {}
        virtual uint8  Get() const  { return _sp.GetLo() ; }
        virtual void   Set(uint8 v) { _sp.SetLo(v) ; }
        virtual uint8  Init() const       { return _sp.Init() >> 0 ; }
      private:
        IoSP &_sp ;
      } ;

      IoSP(uint16 init) : _u16(init), _init(init) {}
      uint16  operator()() const { return _u16 ; }
      uint16& operator()()       { return _u16 ; }
      uint8  GetHi() const  { return _u8[1] ; }
      void   SetHi(uint8 v) { _u8[1] = v ; }
      uint8  GetLo() const  { return _u8[0] ; }
      void   SetLo(uint8 v) { _u8[0] = v ; }
      uint16 Init() const   { return _init ; }
    private:
      union
      {
        uint16 _u16 ;
        uint8  _u8[2] ;
      } ;
      uint16 _init ;
    } ;

    class IoSREG : public Io
    {
    public:
      class SREG : public Io::Register
      {
      public:
        SREG(IoSREG &sreg) : Register("SREG"), _sreg(sreg) {}
        virtual uint8  Get() const  { return _sreg.Get() ; }
        virtual void   Set(uint8 v) { _sreg.Set(v) ; }
      private:
        IoSREG &_sreg ;
      } ;

      IoSREG() : _sreg(0x00) {}
      uint8  Get() const  { return _sreg ; }
      void   Set(uint8 v) { _sreg = v ; }

    private:
      uint8 _sreg ;
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
    bool DataAddrName(uint32 addr, std::string &name) const ;
    bool ProgAddrName(uint32 addr, std::string &name) const ;
    Command ProgramNext() ;

    std::size_t  PC() const { return _pc ; }
    std::size_t& PC()       { return _pc ; }

    uint32  Ticks() const { return _ticks ; }
    
    uint8   Reg(uint32 reg) const ;
    void    Reg(uint32 reg, uint8 value) ;
    uint16  RegW(uint32 reg) const ;
    void    RegW(uint32 reg, uint16 value) ;
    uint8   Io(uint32 io) const ;
    void    Io(uint32 io, uint8 value) ;
    uint8   Data(uint32 addr, bool resetOnError = true) const ;
    void    Data(uint32 addr, uint8 value, bool resetOnError = true) ;
    void    Eeprom(size_t address, uint8 value, bool resetOnError = true) ;
    uint8   Eeprom(size_t address, bool resetOnError = true) const ;
    Command Prog(uint32 addr) const ;
    void    Prog(uint32 addr, uint16 Command) ;
    const Instruction* Instr(uint32 addr) const ;
    
    uint8  GetSREG() const  { return _sreg.Get()  ; }
    void   SetSREG(uint8 v) { _sreg.Set(v)  ; }
    uint8  GetSPL()  const  { return _sp.GetLo() ; }
    void   SetSPL(uint8 v)  { _sp.SetLo(v) ; }
    uint8  GetSPH()  const  { return _sp.GetHi() ; }
    void   SetSPH(uint8 v)  { _sp.SetHi(v) ; }

    void  Push(uint8 value) ;
    uint8 Pop() ;
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
    size_t SetEeprom(size_t address, const std::vector<uint8> &eeprom) ;

    const Xref* XrefByAddr(uint32 addr) const ;
    const Xref* XrefByLabel(const std::string &label) const ;
    bool        XrefAdd(const Xref &xref) ;
    bool        XrefAdd(XrefType type, uint32 target, uint32 source) ;
    const std::vector<Xref*>&           Xrefs() const { return _xrefs ; }
    const std::map<uint32     , Xref*>& XrefByAddr()  const { return _xrefByAddr  ; }
    const std::map<std::string, Xref*>& XrefByLabel() const { return _xrefByLabel ; }

    void AddBreakpoint(std::size_t addr) { _breakpoints.insert(addr) ; }
    void DelBreakpoint(std::size_t addr) { _breakpoints.erase(addr) ; }
    bool IsBreakpoint(std::size_t addr) const { return _breakpoints.find(addr) != _breakpoints.end() ; }
    bool IsBreakpoint()                 const { return _breakpoints.find(_pc ) != _breakpoints.end() ; }
    const std::set<std::size_t>& Breakpoints() const { return _breakpoints ; }
    
    virtual bool PcIs22bit()     { return false ; }
    virtual bool IsXmega()       { return false ; }
    virtual bool IsTinyReduced() { return false ; }
    
  protected:
    void AddInstruction(const Instruction *instr) ;
    void AnalyzeXrefs() ;

  protected:
    std::size_t _pc ;
    IoSP        _sp ;
    IoSREG      _sreg ;
    uint32      _ticks ;
    
    std::size_t _programSize ;

    std::size_t _regSize, _regStart, _regEnd ;
    std::size_t _ioSize, _ioStart, _ioEnd ;
    std::size_t _dataSize, _dataStart, _dataEnd ;
    std::size_t _eepromSize ;

    std::vector<Command>         _program ;
    uint8                        _reg[0x20] ;
    std::vector<Io::Register*>   _io ;
    std::vector<uint8>           _data ;
    std::vector<uint8>           _eeprom ;
    std::vector<KnownProgramAddress> _knownProgramAddresses ;
    std::vector<Xref*>           _xrefs ;
    std::map<uint32, Xref*>      _xrefByAddr ;
    std::map<std::string, Xref*> _xrefByLabel ;
    std::set<std::size_t>        _breakpoints ;
    
    std::vector<const Instruction*> _instructions ;       // map cmd to instruction

#ifdef DEBUG
    FILE *_log ;
#endif
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

    virtual void  PushPC() ;
    virtual void  PopPC() ;

    virtual bool PcIs22bit()     { return false ; }
    virtual bool IsXmega()       { return true  ; }

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
