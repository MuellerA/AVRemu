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
      virtual uint8_t  Get() const    = 0 ;
      virtual void     Set(uint8_t v) = 0 ;
      virtual uint8_t  Init() const { return 0x00 ; } // bootup value
      virtual void     Add(const std::vector<uint8_t> &data) { ; }
      
    protected:
      std::string _name ;
    } ;

  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoRegisterValue
  ////////////////////////////////////////////////////////////////////////////////

  class IoRegisterValue : public Io::Register
  {
  public:
    IoRegisterValue(const std::string &name, uint8_t &value) : Register(name), _value(value) {}
    
    virtual uint8_t  Get() const    { return _value ; }
    virtual void     Set(uint8_t v) { _value = v    ; }
  private:
    uint8_t &_value ;
  } ;
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoRegisterNotImplemented
  ////////////////////////////////////////////////////////////////////////////////
  
  class IoRegisterNotImplemented : public Io::Register
  {
  public:
    IoRegisterNotImplemented(const std::string &name, uint8_t init = 0) : Register(name), _value(init), _errorMsgIssued(false) {}

    virtual uint8_t  Get() const  ;
    virtual void     Set(uint8_t v) ;
  private:
    uint8_t       _value ;
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
      virtual uint8_t Get() const  ;
      virtual void    Set(uint8_t v) ;
      virtual void    Add(const std::vector<uint8_t> &data) { _port.Add(data) ; }
      
    private:
      IoXmegaUsart &_port ;
    } ;
    class Status : public Io::Register
    {
    public:
      Status(IoXmegaUsart &port) : Register(port.Name() + "_STATUS"), _port(port), _value(0x20) {}
      virtual uint8_t Get() const    { return (_port.RxAvail() ? 0x80 : 0x00) | 0x40 | 0x20 ; }
      virtual void    Set(uint8_t v) { ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class CtrlA : public Io::Register
    {
    public:
      CtrlA(IoXmegaUsart &port) : Register(port.Name() + "_CTRLA"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return _value ; }
      virtual void    Set(uint8_t v) { _value = v & 0x3f ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class CtrlB : public Io::Register
    {
    public:
      CtrlB(IoXmegaUsart &port) : Register(port.Name() + "_CTRLB"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return _value ; }
      virtual void    Set(uint8_t v) { _value = v & 0x1f ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class CtrlC : public Io::Register
    {
    public:
      CtrlC(IoXmegaUsart &port) : Register(port.Name() + "_CTRLC"), _port(port), _value(0x02) {}
      virtual uint8_t Get() const    { return _value ; }
      virtual void    Set(uint8_t v) { _value = v ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class BaudCtrlA : public Io::Register
    {
    public:
      BaudCtrlA(IoXmegaUsart &port) : Register(port.Name() + "_BAUDCTRLA"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return _value ; }
      virtual void    Set(uint8_t v) { _value = v ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class BaudCtrlB : public Io::Register
    {
    public:
      BaudCtrlB(IoXmegaUsart &port) : Register(port.Name() + "_BAUDCTRLB"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return _value ; }
      virtual void    Set(uint8_t v) { _value = v ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    
    IoXmegaUsart(const std::string &name) : _name(name), _rxPos(0) {}
    const std::string& Name() { return _name ; }
    virtual uint8_t    Rx() const ;
    virtual bool       RxAvail() const { return _rxPos < _rx.size() ; }
    virtual void       Tx(uint8_t v) const ;
    virtual void       Add(const std::vector<uint8_t> &data) ;
    
  private:
    std::string _name ;
    mutable std::vector<uint8_t> _rx ;
    mutable uint32_t _rxPos ;
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
      virtual uint8_t Get() const    { return _eeprom.GetAddrHi() ; }
      virtual void    Set(uint8_t v) { _eeprom.SetAddrHi(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EEARL : public Io::Register
    {
    public:
      EEARL(IoEeprom &eeprom) : Register("EEARL"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return _eeprom.GetAddrLo() ; }
      virtual void    Set(uint8_t v) { _eeprom.SetAddrLo(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EEDR : public Io::Register
    {
    public:
      EEDR(IoEeprom &eeprom) : Register("EEDR"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return _eeprom.GetData() ; }
      virtual void    Set(uint8_t v) { _eeprom.SetData(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EECR : public Io::Register
    {
    public:
      EECR(IoEeprom &eeprom) : Register("EECR"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return _eeprom.GetControl() ; }
      virtual void    Set(uint8_t v) { _eeprom.SetControl(v) ; }
    private:
      IoEeprom &_eeprom ;
    } ;

    IoEeprom(Mcu &mcu, bool hasEepm = true) : _mcu(mcu), _hasEepm(hasEepm), _addr(0), _data(0), _control(0), _activeTicks(0), _writeBusyTicks(0), _readBusyTicks(0) {}

    uint16_t GetAddr() const       { return _addr      ; }
    void     SetAddr(uint16_t v)   ;
    uint8_t  GetAddrHi() const     { return _addr >> 1 ; }
    void     SetAddrHi(uint8_t v)  { SetAddr(((uint16_t)v << 8) | (_addr & 0x00ff)) ; }
    uint8_t  GetAddrLo() const     { return _addr >> 0 ; }
    void     SetAddrLo(uint8_t v)  { SetAddr(((uint16_t)v << 0) | (_addr & 0xff00)) ; }
    uint8_t  GetData() const       { return _data      ; }
    void     SetData(uint8_t v)    ;
    uint8_t  GetControl() const    ;
    void     SetControl(uint8_t v) ;
    
  private:
    static const uint8_t kEEPM  = 0b00110000 ;
    static const uint8_t kEERIE = 0b00001000 ;
    static const uint8_t kEEMPE = 0b00000100 ;
    static const uint8_t kEEPE  = 0b00000010 ;
    static const uint8_t kEERE  = 0b00000001 ;
    
    Mcu      &_mcu ;
    bool     _hasEepm ;
    uint16_t _addr ;
    uint8_t  _data ;
    mutable uint8_t  _control ;
    uint32_t _activeTicks ;
    uint32_t _writeBusyTicks ;
    uint32_t _readBusyTicks ;
  } ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
