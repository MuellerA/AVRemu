////////////////////////////////////////////////////////////////////////////////
// io.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

namespace AVR
{
  class Mcu ;
  
  using Command = unsigned short ; // 16 bit instruction
  using int8    = signed   char  ; //  8 bit
  using uint8   = unsigned char  ; //  8 bit
  using int16   = signed   short ; // 16 bit
  using uint16  = unsigned short ; // 16 bit
  using int32   = signed   int   ; // 32 bit
  using uint32  = unsigned int   ; // 32 bit

  ////////////////////////////////////////////////////////////////////////////////
  // Io
  ////////////////////////////////////////////////////////////////////////////////
  
  class Io
  {
  public:
    class Register
    {
    public:
      Register(const std::string &name) : _name(name) {}
      virtual ~Register() {} ;
      virtual const std::string& Name() const { return _name ; }
      virtual uint8  Get() const  = 0 ;
      virtual void   Set(uint8 v) = 0 ;
      virtual uint8  Init() const { return 0x00 ; } // bootup value
      virtual void   Add(const std::vector<uint8> &data) { ; }
      
    protected:
      std::string _name ;
    } ;

  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoRegisterNotImplemented
  ////////////////////////////////////////////////////////////////////////////////
  
  class IoRegisterNotImplemented : public Io::Register
  {
  public:
    IoRegisterNotImplemented(const std::string &name, uint8 init = 0) : Register(name), _value(init), _errorMsgIssued(false) {}

    virtual uint8  Get() const  ;
    virtual void   Set(uint8 v) ;
  private:
    uint8         _value ;
    mutable bool  _errorMsgIssued ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaUsart
  ////////////////////////////////////////////////////////////////////////////////
  
  class IoXmegaUsart : public Io
  {
  public:
    class Data : public Io::Register
    {
    public:
      Data(IoXmegaUsart &port) : Register(port.Name() + "_DATA"), _port(port) {}
      virtual uint8  Get() const  ;
      virtual void   Set(uint8 v) ;
      virtual void   Add(const std::vector<uint8> &data) { _port.Add(data) ; }
      
    private:
      IoXmegaUsart &_port ;
    } ;
    class Status : public Io::Register
    {
    public:
      Status(IoXmegaUsart &port) : Register(port.Name() + "_STATUS"), _port(port), _value(0x20) {}
      virtual uint8  Get() const  { return (_port.RxAvail() ? 0x80 : 0x00) | 0x40 | 0x20 ; }
      virtual void   Set(uint8 v) { ; }
    private:
      IoXmegaUsart &_port ;
      uint8 _value ;
    } ;
    class CtrlA : public Io::Register
    {
    public:
      CtrlA(IoXmegaUsart &port) : Register(port.Name() + "_CTRLA"), _port(port), _value(0x00) {}
      virtual uint8  Get() const  { return _value ; }
      virtual void   Set(uint8 v) { _value = v & 0x3f ; }
    private:
      IoXmegaUsart &_port ;
      uint8 _value ;
    } ;
    class CtrlB : public Io::Register
    {
    public:
      CtrlB(IoXmegaUsart &port) : Register(port.Name() + "_CTRLB"), _port(port), _value(0x00) {}
      virtual uint8  Get() const  { return _value ; }
      virtual void   Set(uint8 v) { _value = v & 0x1f ; }
    private:
      IoXmegaUsart &_port ;
      uint8 _value ;
    } ;
    class CtrlC : public Io::Register
    {
    public:
      CtrlC(IoXmegaUsart &port) : Register(port.Name() + "_CTRLC"), _port(port), _value(0x02) {}
      virtual uint8  Get() const  { return _value ; }
      virtual void   Set(uint8 v) { _value = v ; }
    private:
      IoXmegaUsart &_port ;
      uint8 _value ;
    } ;
    class BaudCtrlA : public Io::Register
    {
    public:
      BaudCtrlA(IoXmegaUsart &port) : Register(port.Name() + "_BAUDCTRLA"), _port(port), _value(0x00) {}
      virtual uint8  Get() const  { return _value ; }
      virtual void   Set(uint8 v) { _value = v ; }
    private:
      IoXmegaUsart &_port ;
      uint8 _value ;
    } ;
    class BaudCtrlB : public Io::Register
    {
    public:
      BaudCtrlB(IoXmegaUsart &port) : Register(port.Name() + "_BAUDCTRLB"), _port(port), _value(0x00) {}
      virtual uint8  Get() const  { return _value ; }
      virtual void   Set(uint8 v) { _value = v ; }
    private:
      IoXmegaUsart &_port ;
      uint8 _value ;
    } ;
    
    IoXmegaUsart(const std::string &name) : _name(name), _rxPos(0) {}
    const std::string& Name() { return _name ; }
    virtual uint8 Rx() const ;
    virtual bool RxAvail() const { return _rxPos < _rx.size() ; }
    virtual void Tx(uint8 v) const ;
    virtual void Add(const std::vector<uint8> &data) ;
    
  private:
    std::string _name ;
    mutable std::vector<uint8> _rx ;
    mutable size_t _rxPos ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoEeprom
  ////////////////////////////////////////////////////////////////////////////////
  
  class IoEeprom : public Io
  {
  public:
    class EEARH : public Io::Register
    {
    public:
      EEARH(IoEeprom &eeprom) : Register("EEARH"), _eeprom(eeprom) {} ;
      virtual uint8  Get() const  { return _eeprom.GetAddrHi() ; }
      virtual void   Set(uint8 v) { _eeprom.SetAddrHi(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EEARL : public Io::Register
    {
    public:
      EEARL(IoEeprom &eeprom) : Register("EEARL"), _eeprom(eeprom) {} ;
      virtual uint8  Get() const  { return _eeprom.GetAddrLo() ; }
      virtual void   Set(uint8 v) { _eeprom.SetAddrLo(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EEDR : public Io::Register
    {
    public:
      EEDR(IoEeprom &eeprom) : Register("EEDR"), _eeprom(eeprom) {} ;
      virtual uint8  Get() const  { return _eeprom.GetData() ; }
      virtual void   Set(uint8 v) { _eeprom.SetData(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EECR : public Io::Register
    {
    public:
      EECR(IoEeprom &eeprom) : Register("EECR"), _eeprom(eeprom) {} ;
      virtual uint8  Get() const  { return _eeprom.GetControl() ; }
      virtual void   Set(uint8 v) { _eeprom.SetControl(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;

    IoEeprom(Mcu &mcu, bool hasEepm = true) : _mcu(mcu), _hasEepm(hasEepm), _addr(0), _data(0), _control(0), _activeTicks(0), _writeBusyTicks(0), _readBusyTicks(0) {}

    uint16 GetAddr() const     { return _addr      ; }
    void   SetAddr(uint16 v)   ;
    uint8  GetAddrHi() const   { return _addr >> 1 ; }
    void   SetAddrHi(uint8 v)  { SetAddr(((uint16)v << 8) | (_addr & 0x00ff)) ; }
    uint8  GetAddrLo() const   { return _addr >> 0 ; }
    void   SetAddrLo(uint8 v)  { SetAddr(((uint16)v << 0) | (_addr & 0xff00)) ; }
    uint8  GetData() const     { return _data      ; }
    void   SetData(uint8 v)    ;
    uint8  GetControl() const  ;
    void   SetControl(uint8 v) ;
    
  private:
    static const uint8 kEEPM  = 0b00110000 ;
    static const uint8 kEERIE = 0b00001000 ;
    static const uint8 kEEMPE = 0b00000100 ;
    static const uint8 kEEPE  = 0b00000010 ;
    static const uint8 kEERE  = 0b00000001 ;
    
    Mcu    &_mcu ;
    bool   _hasEepm ;
    uint16 _addr ;
    uint8  _data ;
    mutable uint8  _control ;
    uint32 _activeTicks ;
    uint32 _writeBusyTicks ;
    uint32 _readBusyTicks ;
  } ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
