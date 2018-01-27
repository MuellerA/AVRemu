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
    
    void Quit()   { _quit = true ; }
    bool IsQuit() { return _quit ; }

    const std::vector<::Command*>& Commands() const { return _commands ; }
    ::Command* LastCommand() const { return _lastCommand ; }
    
  private:
    Mcu &_mcu ;
    std::vector<::Command*> _commands ;
  
    bool _quit ;
    ::Command *_lastCommand ;
  } ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
