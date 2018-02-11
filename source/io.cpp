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
    if (_ascii || (_mcu.Verbose() && VerboseType::Io))
    {
      if (_notImplemented)
        fprintf(stdout, "not implemented ") ;
      fprintf(stdout, "IO %s read at %05x: %02x", _name.c_str(), _mcu.PC(), v) ;
      if (_ascii && (' ' < v) && (v <= '~'))
        fprintf(stdout, " %c", v) ;
      fprintf(stdout, "\n") ;
    }
    return v ;
  }
  
  uint8_t Io::Register::VS(uint8_t v) const
  {
    if (_ascii || (_mcu.Verbose() && VerboseType::Io))
    {
      if (_notImplemented)
        fprintf(stdout, "not implemented ") ;
      fprintf(stdout, "IO %s write at %05x: %02x", _name.c_str(), _mcu.PC(), v) ;
      if (_ascii && (' ' < v) && (v <= '~'))
        fprintf(stdout, " %c", v) ;
      fprintf(stdout, "\n") ;
    }
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
      if (EepromMapped())
        return ;
      
      return ;
      
    case 0x35: // Erase and write EEPROM page
      if (EepromMapped())
        return ;
      
      return ;

    default:
      fprintf(stderr, "unsupported NVM command at %05x %02x\n", _mcu.PC(), v) ;
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
      // todo
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
      fprintf(stderr, "EEPROM interrupt not supported\n") ;

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
        fprintf(stderr, "EEPROM illegal bit combination EEMPE|EEPE|EERE %02x", v&(kEEMPE|kEEPE|kEERE)) ;
      }
    }
    
    _control = v ;
  }
  
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
