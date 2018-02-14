////////////////////////////////////////////////////////////////////////////////
// filter.h
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"

namespace AVR
{
  class Filter
  {
  public:
    Filter(const std::string &command, VerboseType vt = VerboseType::All) ;
    ~Filter() ;

    bool operator()(const std::string toFilter, std::string &fromFilter) ;
    VerboseType Verbose() const { return _vt ; }

    pid_t Pid() const { return _pid ; }
    const std::string &Command() const { return _command ; }
    
  private:
    pid_t _pid ;
    const std::string _command ;
    VerboseType _vt ;
    int _toChild[2];
    int _toParent[2];
  } ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
