////////////////////////////////////////////////////////////////////////////////
// execute.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "avr.h"

class Command ;

namespace AVR
{
  class Execute
  {
  public:
    Execute(AVR::Mcu &mcu) ;
    ~Execute() ;

    void Loop() ;
    void Do(const std::string &cmd) ;
    
    void Quit()           { _quit = true ; }
    bool IsQuit() const   { return _quit ; }
    void SigInt()         { _sigInt = true ; }
    bool IsSigInt() const { return _sigInt ; }
    void MacroQuit(bool quit) { _macroQuit = quit ; }
    bool MacroQuit() const    { return _macroQuit ; }

    const std::vector<::Command*>& Commands() const { return _commands ; }
    ::Command* LastCommand() const { return _lastCommand ; }
    
  private:
    Mcu &_mcu ;
    std::vector<::Command*> _commands ;
  
    bool _quit ;
    bool _sigInt ;
    bool _macroQuit ;
    ::Command *_lastCommand ;
  } ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
