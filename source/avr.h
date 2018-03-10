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
  class Filter ;

  using Command = uint16_t ; // 16 bit instruction
  using StackFrame = std::pair<uint16_t, uint32_t> ; // sp / pc

  ////////////////////////////////////////////////////////////////////////////////
  // VerboseType
  ////////////////////////////////////////////////////////////////////////////////

  enum class VerboseType
  {
    None      = 0x0000,
    Io        = 0x0001,
    Eeprom    = 0x0002,
    DataError = 0x4000,
    ProgError = 0x8000,
    All       = 0xffff,
  } ;

  bool operator&&(VerboseType a, VerboseType b) ;
  
  VerboseType operator&(VerboseType a, VerboseType b) ;
  VerboseType operator~(VerboseType a) ;
  VerboseType operator|=(VerboseType &a, VerboseType b) ;
  VerboseType operator&=(VerboseType &a, VerboseType b) ;

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
    Instruction(Command pattern, Command mask, const std::string &mnemonic, const std::string &description, bool isTwoWord, bool isJump, bool isBranch, bool isCall, bool isReturn) ;
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

    Command     Pattern()     const { return _pattern     ; }
    Command     Mask()        const { return _mask        ; }
    std::string Mnemonic()    const { return _mnemonic    ; }
    std::string Description() const { return _description ; }
    uint32_t    Size()        const { return _size        ; }
    bool        IsTwoWord()   const { return _size == 2   ; }
    bool        IsJump()      const { return _isJump      ; }
    bool        IsBranch()    const { return _isBranch    ; }
    bool        IsCall()      const { return _isCall      ; }
    bool        IsReturn()    const { return _isReturn    ; }
    
  protected:
    Command     _pattern ;
    Command     _mask    ;
    std::string _mnemonic ;
    std::string _description ;
    uint32_t    _size ;
    bool        _isJump ;
    bool        _isBranch ;
    bool        _isCall ;
    bool        _isReturn ;
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
        SPH(const Mcu &mcu, IoSP &sp) : Register(mcu, "SPH"), _sp(sp) {}
        virtual uint8_t  Get() const    { return _sp.GetHi() ; }
        virtual void     Set(uint8_t v) { _sp.SetHi(v) ; }
        virtual uint8_t  Init() const   { return _sp.Init() >> 8 ; }
      private:
        IoSP &_sp ;
      } ;
      class SPL : public Io::Register
      {
      public:
        SPL(const Mcu &mcu, IoSP &sp) : Register(mcu, "SPL"), _sp(sp) {}
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
        SREG(const Mcu &mcu, IoSREG &sreg) : Register(mcu, "SREG"), _sreg(sreg) {}
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

    class IoRamp : public Io
    {
    public:
      class Ramp : public Io::Register
      {
      public:
        Ramp(const Mcu &mcu, const std::string &name, IoRamp &ramp) : Register(mcu, name), _ramp(ramp) {}
    
        virtual uint8_t  Get() const    { return _ramp.Get() >> 16     ; }
        virtual void     Set(uint8_t v) { _ramp.Set((uint32_t)v << 16) ; }
      private:
        IoRamp &_ramp ;
      } ;

      IoRamp() : _ramp(0) {}
      uint32_t Get() const     { return _ramp         ; }
      void     Set(uint32_t v) { _ramp = v & 0xff0000 ; }

    private:      
      uint32_t _ramp ;
    } ;
    
    class Trace
    {
    public:
      Trace(const Mcu &mcu) ;
      ~Trace() ;

      bool Open(const std::string &filename, uint32_t addr = 0) ;
      void Add(uint32_t src, uint32_t dst, const Instruction &instr) ;
      bool Close() ;
      bool operator()() const { return _file != nullptr ; }
      uint32_t StopAddr() const { return _stop ; }

    private:
      const Mcu &_mcu ;
      FILE      *_file ;
      uint32_t   _src ;
      uint32_t   _dst ;
      uint32_t   _cnt ;
      bool       _isRet ;
      bool       _isCall ;
      uint32_t   _lvl ;
      uint32_t   _stop ;
    } ;
    
  protected:
    Mcu(const std::string &name, uint32_t flashSize, uint32_t ioSize, uint32_t ramSize , uint32_t eepromSize, uint32_t sp) ;
    Mcu() = delete ;
    Mcu& operator=(const Mcu&) = delete ;
  public:
    virtual ~Mcu() ;

  public:
    const std::string &Name() const { return _name ; }
    
    uint32_t FlashSize()  const { return _flashSize   ; }
    uint32_t IoSize()     const { return _ioSize      ; }
    uint32_t RamSize()    const { return _ramSize     ; }
    uint32_t EepromSize() const { return _eepromSize  ; }
    
    void Execute() ;
    void Skip() ;
    void Status() ;
    std::string Disasm() ;
    bool IoName(uint32_t addr, std::string &name) const ;
    bool ProgAddrName(uint32_t addr, std::string &name) const ;
    Command ProgramNext() ;

    uint32_t  PC() const { return _pc ; }
    uint32_t& PC()       { return _pc ; }

    uint32_t  Ticks() const { return _ticks ; }
    
    uint8_t  Reg(uint32_t reg) const ;
    void     Reg(uint32_t reg, uint8_t value) ;
    uint16_t RegW(uint32_t reg) const ;
    void     RegW(uint32_t reg, uint16_t value) ;
    uint8_t  Io(uint32_t io) const ;
    bool     Io(uint32_t io, uint8_t &byte) const ;
    void     Io(uint32_t io, uint8_t value) ;
    uint8_t  Ram(uint32_t addr) const ;
    void     Ram(uint32_t addr, uint8_t value) ;
    void     Eeprom(uint32_t address, uint8_t value, bool resetOnError = true) ;
    uint8_t  Eeprom(uint32_t address, bool resetOnError = true) const ;
    Command  Flash(uint32_t addr) const ;
    void     Flash(uint32_t addr, Command cmd) ;
    virtual uint8_t Data(uint32_t addr, bool resetOnError = true) const ;
    virtual bool    Data(uint32_t addr, uint8_t &byte) const ;
    virtual void    Data(uint32_t addr, uint8_t value, bool resetOnError = true) ;
    virtual Command Program(uint32_t addr) const ;
    virtual void    Program(uint32_t addr, Command cmd) ;
    virtual bool    InRam(uint32_t addr) const ;
    virtual void    RamRange(uint32_t &min, uint32_t &max) const ;
    const Instruction* Instr(uint32_t addr) const ;
    
    uint8_t  GetSREG() const      { return _sreg.Get()  ; }
    void     SetSREG(uint8_t v)   { _sreg.Set(v)        ; }
    uint16_t GetSP() const        { return _sp()        ; }
    void     SetSP(uint16_t v)    { _sp() = v           ; }
    uint8_t  GetSPL()  const      { return _sp.GetLo()  ; }
    void     SetSPL(uint8_t v)    { _sp.SetLo(v)        ; }
    uint8_t  GetSPH()  const      { return _sp.GetHi()  ; }
    void     SetSPH(uint8_t v)    { _sp.SetHi(v)        ; }
    uint32_t GetRampX() const     { return _rampx.Get() ; }
    void     SetRampX(uint32_t v) { _rampx.Set(v)       ; }
    uint32_t GetRampY() const     { return _rampy.Get() ; }
    void     SetRampY(uint32_t v) { _rampy.Set(v)       ; }
    uint32_t GetRampZ() const     { return _rampz.Get() ; }
    void     SetRampZ(uint32_t v) { _rampz.Set(v)       ; }
    uint32_t GetRampD() const     { return _rampd.Get() ; }
    void     SetRampD(uint32_t v) { _rampd.Set(v)       ; }
    uint32_t GetEind() const      { return _eind.Get()  ; }
    void     SetEind(uint32_t v)  { _eind.Set(v)        ; }
    
    void    Push(uint8_t value) ;
    uint8_t Pop() ;
    void  PushPC() ;
    void  PopPC() ;

    const std::vector<StackFrame>& StackFrames() const { return _stackFrames ; }
    void ResetStackFrames()                            { _stackFrames.clear() ; }
    
    void  Break() ; // call BREAK handlers
    void  Sleep() ;
    void  WDR() ;
    void  NotImplemented(const Instruction&) ; // unimplemented instructions

    const std::vector<const Instruction*>& Instructions() const { return _instructions ; }
    const std::vector<Command>&            Flash()        const { return _flash        ; }
    const std::vector<Io::Register*>&      Io()           const { return _io           ; }
    
    void   ClearFlash() ;
    uint32_t SetFlash(uint32_t address, const std::vector<Command> &prg) ;
    uint32_t SetEeprom(uint32_t address, const std::vector<uint8_t> &eeprom) ;

    const Xref* XrefByAddr(uint32_t addr) const ;
    const Xref* XrefByLabel(const std::string &label) const ;
    bool        XrefAdd(const Xref &xref) ;
    bool        XrefAdd(XrefType type, uint32_t target, uint32_t source) ;
    const std::vector<Xref*>&           Xrefs() const { return _xrefs ; }
    const std::map<uint32_t   , Xref*>& XrefByAddr()  const { return _xrefByAddr  ; }
    const std::map<std::string, Xref*>& XrefByLabel() const { return _xrefByLabel ; }

    void AddBreakpoint(uint32_t addr) { _breakpoints.insert(addr) ; }
    void DelBreakpoint(uint32_t addr) { _breakpoints.erase(addr) ; }
    bool IsBreakpoint(uint32_t addr) const { return _breakpoints.find(addr) != _breakpoints.end() ; }
    bool IsBreakpoint()                 const { return _breakpoints.find(_pc ) != _breakpoints.end() ; }
    const std::set<uint32_t>& Breakpoints() const { return _breakpoints ; }
    
    bool PcIs22bit()     const { return _pcIs22Bit     ; }
    bool IsXmega()       const { return _isXMega       ; }
    bool IsTinyReduced() const { return _isTinyReduced ; }

    bool TraceOn(const std::string &filename, uint32_t addr = 0) { return _trace.Open(filename, addr) ; }
    bool TraceOff()                                                 { return _trace.Close()              ; }

    VerboseType  Verbose() const { return _verbose ; }
    VerboseType& Verbose()       { return _verbose ; }
    void Verbose(VerboseType vt, const std::string &text) const ;
    void AddFilter(VerboseType vt, const std::string &command) ;
    void DelFilter(pid_t pid) ;
    const std::vector<Filter*>& Filters() const { return _filters ; }
    
  protected:
    void AddInstruction(const Instruction *instr) ;
    void AnalyzeXrefs() ;

  protected:
    const std::string _name ;
    
    uint32_t _pc ;
    IoSP     _sp ;
    IoSREG   _sreg ;
    IoRamp   _rampx, _rampy, _rampz ;
    IoRamp   _rampd, _eind ;
    uint32_t _ticks ;
    
    uint32_t             _flashSize ;
    uint32_t             _loadedFlashSize ;
    std::vector<Command> _flash ;

    uint8_t                     _reg[0x20] ;

    uint32_t                    _ioSize ;
    std::vector<Io::Register*>  _io ;

    uint32_t             _ramSize ;
    std::vector<uint8_t> _ram ;
    
    uint32_t             _eepromSize ;
    std::vector<uint8_t> _eeprom ;

    std::vector<StackFrame> _stackFrames ;
    
    bool _pcIs22Bit     ;
    bool _isXMega       ;
    bool _isTinyReduced ;
    
    std::vector<KnownProgramAddress> _knownProgramAddresses ;
    std::vector<Xref*>               _xrefs ;
    std::map<uint32_t, Xref*>        _xrefByAddr ;
    std::map<std::string, Xref*>     _xrefByLabel ;
    std::set<uint32_t>               _breakpoints ;
    std::vector<const Instruction*> _instructions ; // map cmd to instruction

    std::vector<Filter*> _filters ;
    Trace _trace ;

    VerboseType _verbose ;
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
    ATmegaXX8(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize) ;
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
    ATtinyX4(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize) ;
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
    ATtinyX5(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize) ;
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
    virtual uint8_t Data(uint32_t addr, bool resetOnError = true) const ;
    virtual bool    Data(uint32_t addr, uint8_t &byte) const ;
    virtual void    Data(uint32_t addr, uint8_t value, bool resetOnError = true) ;
    virtual Command Program(uint32_t addr) const ;
    virtual void    Program(uint32_t addr, Command cmd) ;
    virtual bool    InRam(uint32_t addr) const ;
    virtual void    RamRange(uint32_t &min, uint32_t &max) const ;

    uint8_t UserSignature(uint32_t addr) const ;
    uint8_t ProductionSignature(uint32_t addr) const ;
    
  protected:
    ATxmegaAU(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize) ;
    virtual ~ATxmegaAU() ;

    IoXmegaCpu   _cpu ;
    IoXmegaNvm   _nvm ;
    IoXmegaRtc   _rtc ;
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
