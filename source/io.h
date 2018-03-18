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
      Register(const Mcu &mcu, const std::string &name, bool ascii = false, bool notImplemented = false)
        : _mcu(mcu), _name(name), _ascii(ascii), _notImplemented(notImplemented) {}
      virtual ~Register() {} ;
      virtual const std::string& Name() const { return _name ; }
      virtual uint8_t  Get() const    = 0 ;
      virtual void     Set(uint8_t v) = 0 ;
      virtual uint8_t  Init() const { return 0x00 ; } // bootup value
      virtual void     Add(const std::vector<uint8_t> &data) { ; }
      
    protected:
      uint8_t VG(uint8_t v) const ;
      uint8_t VS(uint8_t v) const ;

      const Mcu &_mcu ;
      std::string _name ;
      bool _ascii ;
      bool _notImplemented ;
    } ;

  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoRegisterValue
  ////////////////////////////////////////////////////////////////////////////////

  class IoRegisterValue : public Io::Register
  {
  public:
    IoRegisterValue(const Mcu &mcu, const std::string &name, uint8_t &value) : Register(mcu, name), _value(value) {}
    
    virtual uint8_t  Get() const    { return VG(_value) ; }
    virtual void     Set(uint8_t v) { _value = VS(v)    ; }
  private:
    uint8_t &_value ;
  } ;
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoRegisterNotImplemented
  ////////////////////////////////////////////////////////////////////////////////
  
  class IoRegisterNotImplemented : public Io::Register
  {
  public:
    IoRegisterNotImplemented(const Mcu &mcu, const std::string &name, uint8_t init = 0) : Register(mcu, name, false, true), _value(init) {}

    virtual uint8_t  Get() const    { return VG(_value) ; }
    virtual void     Set(uint8_t v) { _value = VS(v)    ; }
  private:
    uint8_t       _value ;
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
      Data(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_DATA", true), _port(port) {}
      virtual uint8_t Get() const  ;
      virtual void    Set(uint8_t v) ;
      virtual void    Add(const std::vector<uint8_t> &data) { _port.Add(data) ; }
      
    private:
      IoXmegaUsart &_port ;
    } ;
    class Status : public Io::Register
    {
    public:
      Status(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_STATUS"), _port(port), _value(0x20) {}
      virtual uint8_t Get() const    { return VG((_port.RxAvail() ? 0x80 : 0x00) | 0x40 | 0x20) ; }
      virtual void    Set(uint8_t v) { VS(v) ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class CtrlA : public Io::Register
    {
    public:
      CtrlA(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_CTRLA"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return VG(_value) ; }
      virtual void    Set(uint8_t v) { _value = VS(v) & 0x3f ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class CtrlB : public Io::Register
    {
    public:
      CtrlB(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_CTRLB"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return VG(_value) ; }
      virtual void    Set(uint8_t v) { _value = VS(v) & 0x1f ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class CtrlC : public Io::Register
    {
    public:
      CtrlC(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_CTRLC"), _port(port), _value(0x02) {}
      virtual uint8_t Get() const    { return VG(_value) ; }
      virtual void    Set(uint8_t v) { _value = VS(v) ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class BaudCtrlA : public Io::Register
    {
    public:
      BaudCtrlA(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_BAUDCTRLA"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return VG(_value) ; }
      virtual void    Set(uint8_t v) { _value = VS(v) ; }
    private:
      IoXmegaUsart &_port ;
      uint8_t       _value ;
    } ;
    class BaudCtrlB : public Io::Register
    {
    public:
      BaudCtrlB(const Mcu &mcu, IoXmegaUsart &port) : Register(mcu, port.Name() + "_BAUDCTRLB"), _port(port), _value(0x00) {}
      virtual uint8_t Get() const    { return VG(_value) ; }
      virtual void    Set(uint8_t v) { _value = VS(v) ; }
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
  // IoXmegaCpu
  ////////////////////////////////////////////////////////////////////////////////

  class IoXmegaCpu : public Io
  {
  public:
    class Ccp : public Io::Register
    {
    public:
      Ccp(const Mcu &mcu, IoXmegaCpu &cpu) : Register(mcu, "CPU_CCP"), _cpu(cpu) {}
      virtual uint8_t Get() const    { return VG(_cpu.GetCcp()) ; }
      virtual void    Set(uint8_t v) { _cpu.SetCcp(VS(v))       ; }

    private:
      IoXmegaCpu &_cpu ;
    } ;

    IoXmegaCpu(Mcu &mcu) ;

    uint8_t GetCcp() const ;
    void SetCcp(uint8_t v) ;
    
  private:
    Mcu     &_mcu ;
    uint8_t  _value ;
    uint64_t _ticks ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaClock

  class IoXmegaClk : public Io
  {
  public:
    class RtcCtrl : public Io::Register
    {
    public:
      RtcCtrl(const Mcu &mcu, IoXmegaClk &clk) : Register(mcu, "CLK_RTCCTRL"), _clk(clk) {}
      virtual uint8_t Get() const    { return VG(_clk.GetRtcCtrl()) ; }
      virtual void    Set(uint8_t v) { _clk.SetRtcCtrl(VS(v))       ; }

    private:
      IoXmegaClk &_clk ;
    } ;

    IoXmegaClk(Mcu &mcu) ;

    uint8_t GetRtcCtrl() const ;
    void SetRtcCtrl(uint8_t v) ;
    uint8_t GetRtcEnable() const ;
    uint8_t GetRtcSrc() const ;
    uint32_t GetRtcFreq() const ;

  private:
    Mcu    &_mcu ;
    uint8_t  _rtcCtrl ;
    uint32_t _rtcFreq ;
  } ;
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaNvm
  ////////////////////////////////////////////////////////////////////////////////

  class IoXmegaNvm : public Io
  {
  public:
    class Addr0 : public Io::Register
    {
    public:
      Addr0(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_ADDR0"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetAddr0()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetAddr0(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;

    class Addr1 : public Io::Register
    {
    public:
      Addr1(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_ADDR1"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetAddr1()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetAddr1(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class Addr2 : public Io::Register
    {
    public:
      Addr2(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_ADDR2"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetAddr2()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetAddr2(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class Data0 : public Io::Register
    {
    public:
      Data0(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_DATA0"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetData0()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetData0(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class Data1 : public Io::Register
    {
    public:
      Data1(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_DATA1"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetData1()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetData1(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class Data2 : public Io::Register
    {
    public:
      Data2(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_DATA2"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetData2()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetData2(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class Cmd : public Io::Register
    {
    public:
      Cmd(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_CMD"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetCmd()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetCmd(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class CtrlA : public Io::Register
    {
    public:
      CtrlA(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_CTRLA"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetCtrlA()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetCtrlA(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class CtrlB : public Io::Register
    {
    public:
      CtrlB(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_CTRLB"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetCtrlB()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetCtrlB(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class IntCtrl : public Io::Register
    {
    public:
      IntCtrl(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_INTCTRL"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetIntCtrl()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetIntCtrl(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class Status : public Io::Register
    {
    public:
      Status(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_STATUS"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetStatus()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetStatus(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;
    
    class LockBits : public Io::Register
    {
    public:
      LockBits(const Mcu &mcu, IoXmegaNvm &nvm) : Register(mcu, "NVM_LockBits"), _nvm(nvm) {}
      virtual uint8_t Get() const    { return VG(_nvm.GetLockBits()) ; }
      virtual void    Set(uint8_t v) { _nvm.SetLockBits(VS(v))       ; }

    private:
      IoXmegaNvm &_nvm ;
    } ;

    enum class LpmType
    {
      Flash,
      UserSignature,        
      ProductionSignature,
    } ;
    
    IoXmegaNvm(Mcu &mcu, IoXmegaCpu &cpu) ;

    uint8_t  GetAddr0() ;
    void     SetAddr0(uint8_t v) ;
    uint8_t  GetAddr1() ;
    void     SetAddr1(uint8_t v) ;
    uint8_t  GetAddr2() ;
    void     SetAddr2(uint8_t v) ;
    uint32_t GetAddr() ;
    void     SetAddr(uint32_t v) ;

    uint8_t  GetData0() ;
    void     SetData0(uint8_t v) ;
    uint8_t  GetData1() ;
    void     SetData1(uint8_t v) ;
    uint8_t  GetData2() ;
    void     SetData2(uint8_t v) ;
    uint32_t GetData() ;
    void     SetData(uint32_t v) ;

    uint8_t GetCmd() ;
    void    SetCmd(uint8_t v) ;
    uint8_t GetCtrlA() ;
    void    SetCtrlA(uint8_t v) ;
    uint8_t GetCtrlB() ;
    void    SetCtrlB(uint8_t v) ;
    uint8_t GetIntCtrl() ;
    void    SetIntCtrl(uint8_t v) ;
    uint8_t GetStatus() ;
    void    SetStatus(uint8_t v) ;
    uint8_t GetLockBits() ;
    void    SetLockBits(uint8_t v) ;

    LpmType Lpm() const ;
    bool    EepromMapped() const ;
    
  private:
    Mcu        &_mcu ;
    IoXmegaCpu &_cpu ;
    
    uint32_t _addr ;
    uint32_t _data ;
    uint8_t  _cmd ;
    uint8_t  _ctrlB ;
    uint8_t  _intCtrl ;
    uint8_t  _lockBits ;

    LpmType  _lpm ;
  } ;

  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaRtc
  ////////////////////////////////////////////////////////////////////////////////

  class IoXmegaRtc : public Io
  {
  public:
    class Ctrl : public Io::Register
    {
    public:
      Ctrl(const Mcu &mcu, IoXmegaRtc &rtc) : Register(mcu, "RTC_CTRL"), _rtc(rtc) {}
      virtual uint8_t Get() const    { return VG(_rtc.GetPrescaler()) ; }
      virtual void    Set(uint8_t v) { _rtc.SetPrescaler(VS(v))       ; }

    private:
      IoXmegaRtc &_rtc ;
    } ;
    class Status : public Io::Register
    {
    public:
      Status(const Mcu &mcu, IoXmegaRtc &rtc) : Register(mcu, "RTC_STATUS"), _rtc(rtc) {}
      virtual uint8_t Get() const    { return VG(0) ; }
      virtual void    Set(uint8_t v) { VS(v)        ; }

    private:
      IoXmegaRtc &_rtc ;
    } ;

    class CntL : public Io::Register
    {
    public:
      CntL(const Mcu &mcu, IoXmegaRtc &rtc) : Register(mcu, "RTC_CNTL"), _rtc(rtc) {}
      virtual uint8_t Get() const    { return VG(_rtc.GetCntL()) ; }
      virtual void    Set(uint8_t v) { _rtc.SetCntL(VS(v))       ; }

    private:
      IoXmegaRtc &_rtc ;
    } ;
    class CntH : public Io::Register
    {
    public:
      CntH(const Mcu &mcu, IoXmegaRtc &rtc) : Register(mcu, "RTC_CNTH"), _rtc(rtc) {}
      virtual uint8_t Get() const    { return VG(_rtc.GetCntH()) ; }
      virtual void    Set(uint8_t v) { _rtc.SetCntH(VS(v))       ; }

    private:
      IoXmegaRtc &_rtc ;
    } ;
    class Temp : public Io::Register
    {
    public:
      Temp(const Mcu &mcu, IoXmegaRtc &rtc) : Register(mcu, "RTC_TEMP"), _rtc(rtc) {}
      virtual uint8_t Get() const    { return VG(_rtc.GetTemp()) ; }
      virtual void    Set(uint8_t v) { _rtc.SetTemp(VS(v))       ; }

    private:
      IoXmegaRtc &_rtc ;
    } ;

    IoXmegaRtc(Mcu &mcu, IoXmegaClk &clk) ;

    uint8_t GetPrescaler() const ;
    void    SetPrescaler(uint8_t v) ;
    uint8_t GetCntL() const ;
    void    SetCntL(uint8_t v) ;
    uint8_t GetCntH() const ;
    void    SetCntH(uint8_t v) ;
    uint8_t GetTemp() const ;
    void    SetTemp(uint8_t v) ;

  private:
    Mcu        &_mcu ;
    IoXmegaClk &_clk ;

    mutable uint64_t _ticks ;
    uint32_t _prescaler ;
    uint32_t _prescalerDiv ;
    mutable uint32_t _cnt ;
    mutable uint8_t  _tmp ;
  } ;
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoEeprom (tiny, mega)
  ////////////////////////////////////////////////////////////////////////////////
  
  class IoEeprom : public Io
  {
  public:
    class EEARH : public Io::Register
    {
    public:
      EEARH(const Mcu &mcu, IoEeprom &eeprom) : Register(mcu, "EEARH"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return VG(_eeprom.GetAddrHi()) ; }
      virtual void    Set(uint8_t v) { _eeprom.SetAddrHi(VS(v))       ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EEARL : public Io::Register
    {
    public:
      EEARL(const Mcu &mcu, IoEeprom &eeprom) : Register(mcu, "EEARL"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return VG(_eeprom.GetAddrLo()) ; }
      virtual void    Set(uint8_t v) { _eeprom.SetAddrLo(VS(v))       ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EEDR : public Io::Register
    {
    public:
      EEDR(const Mcu &mcu, IoEeprom &eeprom) : Register(mcu, "EEDR"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return VG(_eeprom.GetData()) ; }
      virtual void    Set(uint8_t v) { _eeprom.SetData(VS(v))       ; }
    private:
      IoEeprom &_eeprom ;
    } ;
    class EECR : public Io::Register
    {
    public:
      EECR(const Mcu &mcu, IoEeprom &eeprom) : Register(mcu, "EECR"), _eeprom(eeprom) {} ;
      virtual uint8_t Get() const    { return VG(_eeprom.GetControl()) ; }
      virtual void    Set(uint8_t v) { _eeprom.SetControl(VS(v))       ; }
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
