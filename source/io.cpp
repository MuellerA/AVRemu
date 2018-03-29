////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // Io::Register
  ////////////////////////////////////////////////////////////////////////////////

  uint8_t Io::Register::VG(uint8_t v) const
  {
    char buff[1024] ;
    char *ptr = buff ;
    
    if (_notImplemented)
      ptr += sprintf(ptr, "not implemented ") ;
    ptr += sprintf(ptr, "IO %s read at %05x: %02x", _name.c_str(), _mcu.PC(), v) ;
    if ((' ' < v) && (v <= '~'))
      ptr += sprintf(ptr, " %c", v) ;
    ptr += sprintf(ptr, "\n") ;
    _mcu.Verbose(VerboseType::Io, buff) ;

    return v ;
  }
  
  uint8_t Io::Register::VS(uint8_t v) const
  {
    char buff[1024] ;
    char *ptr = buff ;
    
    if (_notImplemented)
      ptr += sprintf(ptr, "not implemented ") ;
    ptr += sprintf(ptr, "IO %s write at %05x: %02x", _name.c_str(), _mcu.PC(), v) ;
    if ((' ' < v) && (v <= '~'))
      ptr += sprintf(ptr, " %c", v) ;
    ptr += sprintf(ptr, "\n") ;
    _mcu.Verbose(VerboseType::Io, buff) ;
    
    return v  ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaUsart
  ////////////////////////////////////////////////////////////////////////////////

  uint8_t IoXmegaUsart::Data::Get() const
  {
    return VG(_port.Rx()) ;
  }
  void IoXmegaUsart::Data::Set(uint8_t v)
  {
    _port.Tx(VS(v)) ;
  }
  
  uint8_t IoXmegaUsart::Rx() const
  {
    if (_rxPos < _rx.size())
      return (unsigned char) _rx[_rxPos++] ;

    _rx.clear() ;
    _rxPos = 0 ;
    return 0 ;
  }
  void IoXmegaUsart::Tx(uint8_t c) const
  {
  }
  void IoXmegaUsart::Add(const std::vector<uint8_t> &data)
  {
    _rx.insert(std::end(_rx), std::begin(data), std::end(data));
  }

  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaCpu
  ////////////////////////////////////////////////////////////////////////////////

  IoXmegaCpu::IoXmegaCpu(Mcu &mcu) : _mcu(mcu), _value(0), _ticks(0)
  {
  }

  uint8_t IoXmegaCpu::GetCcp() const
  {
    uint8_t v = (_ticks + 4 > _mcu.Ticks()) ? _value : 0x00 ;
    return v ;
  }
  
  void IoXmegaCpu::SetCcp(uint8_t v)
  {
    _ticks = _mcu.Ticks() ;
    switch (v)
    {
    case 0xd8: _value = 0x01 ; return ;
    case 0x9d: _value = 0x02 ; return ;
    }
  }  

  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaClk
  ////////////////////////////////////////////////////////////////////////////////

  IoXmegaClk::IoXmegaClk(Mcu &mcu) : _mcu(mcu), _rtcCtrl(0), _rtcFreq(0)
  {
  }

  uint8_t IoXmegaClk::GetRtcCtrl() const
  {
    return _rtcCtrl ;
  }

  void IoXmegaClk::SetRtcCtrl(uint8_t v)
  {
    _rtcCtrl = v & 0x0f ;

    switch (_rtcCtrl)
    {
    case 0b0001: _rtcFreq =  1000 ; break ;
    case 0b0011: _rtcFreq =  1024 ; break ;
    case 0b0101: _rtcFreq =  1024 ; break ;
    case 0b1011: _rtcFreq = 32768 ; break ;
    case 0b1101: _rtcFreq = 32768 ; break ;
    case 0b1111: _rtcFreq = 32768 ; break ; //external clock
    default:     _rtcFreq =     0 ; break ;
    }
  }

  uint8_t IoXmegaClk::GetRtcEnable() const
  {
    return (_rtcCtrl >> 0) & 0x01 ;
  } ;
  
  uint8_t IoXmegaClk::GetRtcSrc() const
  {
    return (_rtcCtrl >> 1) & 0x07 ;
  }

  uint32_t IoXmegaClk::GetRtcFreq() const
  {
    return _rtcFreq ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaNvm
  ////////////////////////////////////////////////////////////////////////////////

  IoXmegaNvm::IoXmegaNvm(Mcu &mcu, IoXmegaCpu &cpu)
    : _mcu(mcu), _cpu(cpu), _lpm(LpmType::Flash)
  {
  }
  
  uint8_t  IoXmegaNvm::GetAddr0()
  {
    return _addr >>  0 ;
  }

  void     IoXmegaNvm::SetAddr0(uint8_t v)
  {
    _addr &= ~(0xff <<  0) ;
    _addr |= v      <<  0  ;
  }

  uint8_t  IoXmegaNvm::GetAddr1()
  {
    return _addr >>  8 ;
  }

  void     IoXmegaNvm::SetAddr1(uint8_t v)
  {
    _addr &= ~(0xff <<  8) ;
    _addr |= v      <<  8  ;
  }

  uint8_t  IoXmegaNvm::GetAddr2()
  {
    return _addr >> 16 ;
  }

  void     IoXmegaNvm::SetAddr2(uint8_t v)
  {
    _addr &= ~(0xff << 16) ;
    _addr |= v      << 16  ;
  }

  uint32_t IoXmegaNvm::GetAddr()
  {
    return _addr ;
  }

  void     IoXmegaNvm::SetAddr(uint32_t v)
  {
    _addr = v ;
  }

  uint8_t  IoXmegaNvm::GetData0()
  {
    return _data >>  0 ;
  }

  void     IoXmegaNvm::SetData0(uint8_t v)
  {
    _data &= ~(0xff <<  0) ;
    _data |= v      <<  0  ;
  }

  uint8_t  IoXmegaNvm::GetData1()
  {
    return _data >>  8 ;
  }

  void     IoXmegaNvm::SetData1(uint8_t v)
  {
    _data &= ~(0xff <<  8) ;
    _data |= v      <<  8  ;
  }

  uint8_t  IoXmegaNvm::GetData2()
  {
    return _data >> 16 ;
  }

  void     IoXmegaNvm::SetData2(uint8_t v)
  {
    _data &= ~(0xff << 16) ;
    _data |= v      << 16  ;
  }

  uint32_t IoXmegaNvm::GetData()
  {
    return _data ;
  }

  void     IoXmegaNvm::SetData(uint32_t v)
  {
    _data = v ;
  }


  uint8_t IoXmegaNvm::GetCmd()
  {
    return _cmd ;
  }

  void    IoXmegaNvm::SetCmd(uint8_t v)
  {
    _cmd = v & 0x7f ;
    switch (_cmd)
    {
    case 0x00: _lpm = LpmType::Flash               ; return ;
    case 0x01: _lpm = LpmType::UserSignature       ; return ;
    case 0x02: _lpm = LpmType::ProductionSignature ; return ;

    case 0x33: // Load EEPROM page buffer
    case 0x35: // Erase and write EEPROM page
      return ;

    default:
      {
        char buff[1024] ;
        char *ptr = buff ;
        ptr += sprintf(ptr, "unsupported NVM command at %05x %02x\n", _mcu.PC(), v) ;
        _mcu.Verbose(VerboseType::Io, buff) ;
      }
      return ;
    }
  }

  uint8_t IoXmegaNvm::GetCtrlA()
  {
    return 0 ;
  }

  void    IoXmegaNvm::SetCtrlA(uint8_t v)
  {
    if (v & 1)
    {
      switch (_cmd)
      {
      case 0x33:  // Load EEPROM page buffer
        return ;
        
      case 0x35: // Erase and write EEPROM page
        if (!(_cpu.GetCcp() & 0x01))
        {
          char buff[1024] ;
          char *ptr = buff ;
          ptr += sprintf(ptr, "unset CCP on Erase and write EEPROM page at %05x\n", _mcu.PC()) ;
          _mcu.Verbose(VerboseType::Io, buff) ;
            
          return ;
        }
        
        _mcu.Eeprom(_addr, _data, false) ;
        return ;        
      }
    }
  }

  uint8_t IoXmegaNvm::GetCtrlB()
  {
    return _ctrlB ;
  }

  void    IoXmegaNvm::SetCtrlB(uint8_t v)
  {
    _ctrlB = v & 0x0f ;
  }

  uint8_t IoXmegaNvm::GetIntCtrl()
  {
    return _intCtrl ;
  }

  void    IoXmegaNvm::SetIntCtrl(uint8_t v)
  {
    _intCtrl = v & 0x0f ;
  }

  uint8_t IoXmegaNvm::GetStatus()
  {
    return 0 ;
  }

  void    IoXmegaNvm::SetStatus(uint8_t v)
  {
    // nothing
  }

  uint8_t IoXmegaNvm::GetLockBits()
  {
    return _lockBits ;
  }

  void    IoXmegaNvm::SetLockBits(uint8_t v)
  {
    _lockBits = v ;
  }

  IoXmegaNvm::LpmType IoXmegaNvm::Lpm() const
  {
    return _lpm ;
  }
  
  bool IoXmegaNvm::EepromMapped() const
  {
    return _ctrlB & 0x08 ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaRtc
  ////////////////////////////////////////////////////////////////////////////////

  IoXmegaRtc::IoXmegaRtc(Mcu &mcu, IoXmegaClk &clk) : _mcu(mcu), _clk(clk), _ticks(0), _prescaler(0), _prescalerDiv(1), _cnt(0), _tmp(0)
  {
  }

  uint8_t IoXmegaRtc::GetCntL() const
  {
    uint64_t ticks = _mcu.Ticks() * _clk.GetRtcFreq() / 32000000 ;
    
    if (_clk.GetRtcEnable() && _prescaler)
    {
      _cnt += (ticks - _ticks) / _prescalerDiv ;
      _ticks = ticks ;
    }
    _tmp = (_cnt >> 8) & 0xff ;
    return (_cnt >> 0) & 0xff ;
  }
  
  uint8_t IoXmegaRtc::GetPrescaler() const
  {
    return _prescaler ;
  }
  
  void    IoXmegaRtc::SetPrescaler(uint8_t v)
  {
    _prescaler = v & 0x07 ;
    switch (_prescaler)
    {
    default: break ;
    case 1: _prescalerDiv =    1 ; break ;
    case 2: _prescalerDiv =    2 ; break ;
    case 3: _prescalerDiv =    8 ; break ;
    case 4: _prescalerDiv =   16 ; break ;
    case 5: _prescalerDiv =   64 ; break ;
    case 6: _prescalerDiv =  256 ; break ;
    case 7: _prescalerDiv = 1024 ; break ;
    }
  }
  void    IoXmegaRtc::SetCntL(uint8_t v)
  {
    _tmp = v ;
  }
  
  uint8_t IoXmegaRtc::GetCntH() const
  {
    return _tmp ;
  }
  
  void    IoXmegaRtc::SetCntH(uint8_t v)
  {
    _cnt = ((uint32_t)v << 8) | _tmp ;
    _ticks = _mcu.Ticks() ;
  }
  
  uint8_t IoXmegaRtc::GetTemp() const
  {
    return _tmp ;
  }
  
  void    IoXmegaRtc::SetTemp(uint8_t v)
  {
    _tmp = v ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoEeprom
  ////////////////////////////////////////////////////////////////////////////////
  
  void IoEeprom::SetAddr(uint16_t v)
  {
    if (v >= _mcu.EepromSize())
      v &= _mcu.EepromSize() - 1 ;
    _addr = v ;
  }

  void IoEeprom::SetData(uint8_t v)
  {
    if ((_writeBusyTicks < _mcu.Ticks()) && (_readBusyTicks < _mcu.Ticks()))
      _data = v ;
  }
  
  uint8_t IoEeprom::GetControl() const
  {
    if (_readBusyTicks < _mcu.Ticks())
      _control &= ~kEERE ;
    if (_writeBusyTicks < _mcu.Ticks())
      _control &= ~kEEPE ;
    if (_activeTicks < _mcu.Ticks())
      _control &= ~kEEMPE ;
    return _control ;
  }
  
  void IoEeprom::SetControl(uint8_t v)
  {
    v &= 0x3f ;
    
    if (v & kEERIE)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "EEPROM interrupt not supported\n") ;
      _mcu.Verbose(VerboseType::Eeprom, buff) ;
    }
    
    if ((_writeBusyTicks < _mcu.Ticks()) && (_readBusyTicks < _mcu.Ticks()))
    {
      switch (v & (kEEMPE | kEEPE | kEERE))
      {
      case kEEMPE:
        _activeTicks = _mcu.Ticks() + 4 ;
        break ;
      case kEEMPE|kEEPE:
      case kEEPE:
        if (_activeTicks >= _mcu.Ticks())
        {
          switch (v & kEEPM)
          {
          case 0x00000000: // erase & write
            _mcu.Eeprom(_addr, _data) ;
            _writeBusyTicks = _mcu.Ticks() + 34 ; // dummy - 3.4ms in real
            break ;
          case 0x00010000: // erase
            _mcu.Eeprom(_addr, 0xff) ;
            _writeBusyTicks = _mcu.Ticks() + 18 ; // dummy - 1.8ms in real
            break ;
          case 0x00100000: // write
            _mcu.Eeprom(_addr, _mcu.Eeprom(_addr) & _data) ;
            _writeBusyTicks = _mcu.Ticks() + 18 ; // dummy - 1.8ms in real
            break ;
          }
        }
        break ;
      case kEERE:
        _data = _mcu.Eeprom(_addr) ;
        _readBusyTicks = _mcu.Ticks() ; // +4
        break ;
      default:
        {
          char buff[80] ;
          snprintf(buff, sizeof(buff), "EEPROM illegal bit combination EEMPE|EEPE|EERE %02x", v&(kEEMPE|kEEPE|kEERE)) ;
          _mcu.Verbose(VerboseType::Eeprom, buff) ;
        }
        break ;
      }
    }
    
    _control = v ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoUsart (mega)
  ////////////////////////////////////////////////////////////////////////////////

  uint8_t IoUsart::UDRn::Get() const
  {
    return VG(_port.Rx()) ;
  }
  void IoUsart::UDRn::Set(uint8_t v)
  {
    _port.Tx(VS(v)) ;
  }

  uint8_t IoUsart::Rx() const
  {
    if (_rxPos < _rx.size())
      return (unsigned char) _rx[_rxPos++] ;

    _rx.clear() ;
    _rxPos = 0 ;
    return 0 ;
  }
  void IoUsart::Tx(uint8_t c) const
  {
  }
  void IoUsart::Add(const std::vector<uint8_t> &data)
  {
    _rx.insert(std::end(_rx), std::begin(data), std::end(data));
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
