////////////////////////////////////////////////////////////////////////////////
// filter.cpp
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <signal.h>

#include "avr.h"
#include "filter.h"

////////////////////////////////////////////////////////////////////////////////

namespace AVR
{
  Filter::Filter(const std::string &command, AVR::VerboseType vt) : _pid(-1), _command(command), _vt(vt), _toChild{-1, -1}, _toParent{-1, -1}
  {
    if ((pipe(_toChild) < 0) || (pipe(_toParent) < 0))
      return ;

    _pid = fork() ;

    if (_pid < 0) // error
    {
      close(_toChild [0]) ; close(_toChild [1]) ;
      close(_toParent[0]) ; close(_toParent[1]) ;
      _toChild[0] = _toChild[1] = 0 ;
      _toParent[0] = _toParent[1] = 0 ;
      return ;
    }

    if (_pid == 0) // child
    {
      setpgid(0, 0) ;
      
      close(_toChild[1]) ;
      close(_toParent[0]) ;

      dup2(_toChild[0], STDIN_FILENO) ;
      close(_toChild[0]) ;

      dup2(_toParent[1], STDOUT_FILENO) ;
      close(_toParent[1]) ;

      execl("/bin/sh", "sh", "-c", _command.c_str(), (char*) 0) ;
    }

    if (_pid > 0) // parent
    {
      close(_toChild[0]) ;
      close(_toParent[1]) ;
    }
  }
  
  Filter::~Filter()
  {
    close(_toChild[1]) ;
    close(_toParent[0]) ;
    kill(_pid, SIGTERM) ;
  }

  bool Filter::operator()(const std::string toFilter, std::string &fromFilter)
  {
    unsigned char buff[1024] ;
    size_t len ;

    if ((_toChild[1] == -1) || (_toParent[0] == -1))
      return false ;
    
    if (write(_toChild[1], toFilter.c_str(), toFilter.size()) != (ssize_t) toFilter.size())
      return false ;

    fromFilter.clear() ;
    while (true)
    {
      len = read(_toParent[0], buff, sizeof(buff)) ;
      if (len < 0)
        return false ;
      if (len == 0)
      {
        close(_toChild[1]) ;
        close(_toParent[0]) ;
        _toChild[1] = _toParent[0] = -1 ;
        return false ;
      }
      fromFilter.append((char*)buff, len) ;
      if (len < sizeof(buff))
        break ;
    }

    while (fromFilter.size() && (fromFilter[fromFilter.size()-1] == '\n'))
    {
      fromFilter.resize(fromFilter.size() - 1) ;
    }
    
    return true ;
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
