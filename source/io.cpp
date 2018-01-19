////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // IoRegisterNotImplemented
  ////////////////////////////////////////////////////////////////////////////////
  
  uint8  IoRegisterNotImplemented::Get() const
  {
    if (!_errorMsgIssued)
    {
      fprintf(stderr, "not implemented IO %s\n", _name.c_str()) ;
      _errorMsgIssued = true ;
    }
    return _value ;
  }
  void  IoRegisterNotImplemented::Set(uint8 v)
  {
    if (!_errorMsgIssued)
    {
      fprintf(stderr, "not implemented IO %s\n", _name.c_str()) ;
      _errorMsgIssued = true ;
    }
    _value = v ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoXmegaUsart
  ////////////////////////////////////////////////////////////////////////////////

  uint8 IoXmegaUsart::Data::Get() const
  {
    return _port.Rx() ;
  }
  void IoXmegaUsart::Data::Set(uint8 v)
  {
    _port.Tx(v) ;
  }
  
  uint8 IoXmegaUsart::Rx() const
  {
    if (_rxPos < _rx.size())
    {
      unsigned char c = (unsigned char) _rx[_rxPos++] ;
      fprintf(stdout, "%s Rx %02x", _name.c_str(), c) ;
      if ((' ' < c) && (c < '~'))
        fprintf(stdout, " %c", c) ;
      fprintf(stdout, "\n") ;
      return c ;
    }
    _rx.clear() ;
    _rxPos = 0 ;
    return 0 ;
  }
  void IoXmegaUsart::Tx(uint8 c) const
  {
    fprintf(stdout, "%s Tx %02x", _name.c_str(), c) ;
    if ((' ' < c) && (c < '~'))
      fprintf(stdout, " %c", c) ;
    fprintf(stdout, "\n") ;
  }
  void IoXmegaUsart::Add(const std::vector<uint8> &data)
  {
    _rx.insert(std::end(_rx), std::begin(data), std::end(data));
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  // IoEeprom
  ////////////////////////////////////////////////////////////////////////////////
  
  void IoEeprom::SetAddr(uint16 v)
  {
    if (v >= _mcu.EepromSize())
      v &= _mcu.EepromSize() - 1 ;
    _addr = v ;
  }

  void IoEeprom::SetData(uint8 v)
  {
    if ((_writeBusyTicks < _mcu.Ticks()) && (_readBusyTicks < _mcu.Ticks()))
      _data = v ;
  }
  
  uint8 IoEeprom::GetControl() const
  {
    if (_readBusyTicks < _mcu.Ticks())
      _control &= ~kEERE ;
    if (_writeBusyTicks < _mcu.Ticks())
      _control &= ~kEEPE ;
    if (_activeTicks < _mcu.Ticks())
      _control &= ~kEEMPE ;
    return _control ;
  }
  
  void IoEeprom::SetControl(uint8 v)
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