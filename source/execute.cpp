////////////////////////////////////////////////////////////////////////////////
// execute.cpp
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <regex>
#include <signal.h>

#include "avr.h"
#include "instr.h"

////////////////////////////////////////////////////////////////////////////////

using strings = std::vector<std::string> ;

////////////////////////////////////////////////////////////////////////////////
// SigIntHdl
////////////////////////////////////////////////////////////////////////////////

static bool SigInt = false ;
static void SigIntHdl(int /*parameter*/)
{
  std::cout << std::endl << "Execution Interrupted" << std::endl << std::endl ;
  SigInt = true ;
}

////////////////////////////////////////////////////////////////////////////////
// Command
////////////////////////////////////////////////////////////////////////////////

class Command
{
public:
  Command(std::string regex) : _regex(regex, std::regex_constants::optimize) {}
  virtual ~Command() {}
  
  virtual strings  Help() const = 0 ;
  virtual bool     Match(const std::string &command) ;
  virtual bool     Execute(AVR::Mcu &mcu) = 0 ;

protected:

  bool Num(const std::string matchNum, AVR::uint32 &num)
  {
    num = std::stoul(matchNum, nullptr, 0) ;
    return true ;
  }
  
  bool Addr(const AVR::Mcu &mcu, const std::string &matchNum, const std::string &matchLbl, AVR::uint32 &addr)
  {
    if (matchLbl.size())
    {
      const AVR::Mcu::Xref *xref = mcu.XrefByLabel(matchLbl) ;
      if (!xref)
        return false ;

      addr = xref->_addr ;
      return true ;
    }

    addr = std::stoul(matchNum, nullptr, 0) ;
    return true ;
  }
  
  
protected:
  static const std::string _reNum  ;
  static const std::string _reAddr ;

  std::string _command ;
  std::regex  _regex ;
  std::smatch _match ;
} ;

bool Command::Match(const std::string &command)
{
  _command = command ;
  return std::regex_match(_command, _match, _regex) ;
}

const std::string Command::_reNum{ R"XXX((?:(0x[0-9a-fA-F]+|[0-9]+)))XXX" } ;
const std::string Command::_reAddr{ R"XXX((?:(0x[0-9a-fA-F]+|[0-9]+)|([_a-zA-Z][_0-9a-zA-Z]+)))XXX" } ;

////////////////////////////////////////////////////////////////////////////////
// CommandStep
////////////////////////////////////////////////////////////////////////////////
class CommandStep : public Command
{
public:
  CommandStep() : Command(R"XXX(\s*([sn])\s*(?:(0x[0-9a-fA-F]+|[\d]+)\s*)?)XXX")
  {
  }

  ~CommandStep()
  {
  }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandStep::Help() const
{
  return strings { "s [count]               -- step in count instructions",
                   "n [count]               -- step over count instructions" } ;
}

bool CommandStep::Execute(AVR::Mcu &mcu)
{
  void (*prevIntHdl)(int) ;
  SigInt = false ;
  prevIntHdl = signal(SIGINT, SigIntHdl) ;
  
  const std::string &mode = _match[1] ;
  const std::string &countStr = _match[2] ;
  AVR::uint32 count = countStr.size() ? std::stoul(countStr, nullptr, 0) : 1 ;
  switch (mode[0])
  {
  case 's':
    for (AVR::uint32 i = 0 ; (i < count) && !SigInt ; ++i)
    {
      mcu.Execute() ;
      if (mcu.IsBreakpoint())
        break ;
    }
    break ;
  case 'n':
    for (AVR::uint32 i = 0 ; (i < count) && !SigInt ; ++i)
    {
      AVR::uint32 pc = mcu.PC() ;
      const AVR::Instruction *instr = mcu.Instr(pc) ;
      if (instr->IsCall())
      {
        pc += (instr->IsTwoWord()) ? 2 : 1 ;
        while ((mcu.PC() != pc) && !mcu.IsBreakpoint(pc) && !SigInt)
        {
          mcu.Execute() ;
          if (mcu.IsBreakpoint())
            break ;
        }
      }
      else
      {
        mcu.Execute() ;
        if (mcu.IsBreakpoint())
          break ;
      }
      if (mcu.IsBreakpoint())
        break ;
    }
    break ;
  }
  signal(SIGINT, prevIntHdl) ;

  mcu.Status() ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandRun
////////////////////////////////////////////////////////////////////////////////
class CommandRun : public Command
{
public:
  CommandRun() : Command(R"XXX(\s*r\s*)XXX")
  {
  }

  ~CommandRun()
  {
  }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandRun::Help() const
{
  return strings { "r                       -- run" } ;
}

bool CommandRun::Execute(AVR::Mcu &mcu)
{
  void (*prevIntHdl)(int) ;
  SigInt = false ;
  prevIntHdl = signal(SIGINT, SigIntHdl) ;

  while (!SigInt)
  {
    mcu.Execute() ;
    if (mcu.IsBreakpoint())
      break ;
  }
  signal(SIGINT, prevIntHdl) ;

  mcu.Status() ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandBreakpoint
////////////////////////////////////////////////////////////////////////////////
class CommandBreakpoint : public Command
{
public:
  CommandBreakpoint() : Command(R"XXX(\s*b\s*([-+])\s*)XXX" + _reAddr + R"XXX(\s*)XXX")
  {
  }

  ~CommandBreakpoint()
  {
  }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandBreakpoint::Help() const
{
  return strings { "b + <addr>|<label>      -- add breakpoint",
                   "b - <addr>|<label>      -- remove breakpoint" } ;
}

bool CommandBreakpoint::Execute(AVR::Mcu &mcu)
{
  AVR::uint32 addr ;
  
  if (!Addr(mcu, _match[2], _match[3], addr))
  {
    std::cout << "illegal value" << std::endl ;
    return false ;
  }

  bool add = _match[1].str()[0] == '+' ;

  if (add)
    mcu.AddBreakpoint(addr) ;
  else
    mcu.DelBreakpoint(addr) ;

  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandListBreakpoints
////////////////////////////////////////////////////////////////////////////////

class CommandListBreakpoints : public Command
{
public:
  CommandListBreakpoints() : Command(R"XXX(\s*lb\s*)XXX") { }
  ~CommandListBreakpoints() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
  } ;

strings CommandListBreakpoints::Help() const
{
  return strings { "lb                      -- list breakpoints" } ;
}
bool CommandListBreakpoints::Execute(AVR::Mcu &mcu)
{
  for (auto addr : mcu.Breakpoints())
  {
    const AVR::Mcu::Xref *xref = mcu.XrefByAddr((AVR::uint32) addr) ;

    std::cout << std::hex << std::setfill('0') << std::setw(4) << addr ;
  
    if (xref)
      std::cout << "    " << xref->_label ;

    std::cout << std::endl << std::endl ;
  }

  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandGoto
////////////////////////////////////////////////////////////////////////////////
class CommandGoto : public Command
{
public:
  CommandGoto() : Command(R"XXX(\s*g\s*)XXX" + _reAddr + R"XXX(\s*)XXX")
  {
  }

  ~CommandGoto()
  {
  }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandGoto::Help() const
{
  return strings { "g <addr>|<label>        -- goto address/label" } ;
}

bool CommandGoto::Execute(AVR::Mcu &mcu)
{
  AVR::uint32 addr ;
  if (!Addr(mcu, _match[1], _match[2], addr))
  {
    std::cout << "illegal value" << std::endl ;
    return false ;
  }

  mcu.PC() = addr ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandAssign
////////////////////////////////////////////////////////////////////////////////
class CommandAssign : public Command
{
public:
  CommandAssign() : Command(R"XXX(\s*([rdp])\s*(0x[0-9a-fA-F]+|[0-9]+)\s*=\s*(0x[0-9a-fA-F]+|[0-9]+)\s*)XXX") { }
  ~CommandAssign() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandAssign::Help() const
{
  return strings
  {
    "r<d>    = byte          -- set register",
    "d<addr> = byte          -- set data memory",
    "p<addr> = word          -- set program memory"
  } ;
}

bool CommandAssign::Execute(AVR::Mcu &mcu)
{
  char typ = _match[1].str()[0] ;
  AVR::uint32 idx = std::stoul(_match[2], nullptr, 0) ;
  AVR::uint32 val = std::stoul(_match[3], nullptr, 0) ;

  switch (typ)
  {
  case 'r':
    if ((idx > 0x1f) || (val > 0xff))
    {
      std::cout << "illegal value" << std::endl ;
      return false ;
    }
    mcu.Reg(idx, val) ;
    break ;

  case 'd':
    if (val > 0xff)
    {
      std::cout << "illegal value" << std::endl ;
      return false ;
    }
    mcu.Data(idx, val, false) ;
    break ;
    
  case 'p':
    if (val > 0xffff)
    {
      std::cout << "illegal value" << std::endl ;
      return false ;
    }
    mcu.Prog(idx, val) ;
    break ;
  }

  mcu.Status() ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandRead
////////////////////////////////////////////////////////////////////////////////
class CommandRead : public Command
{
public:
  CommandRead() : Command(R"XXX(\d*([de])\s*\?\s*(0x[0-9a-fA-F]+|[0-9]+)\s*(0x[0-9a-fA-F]+|[0-9]+)?\s*)XXX") { }
  ~CommandRead() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandRead::Help() const
{
  return strings { "d ? <addr> <len>        -- read memory content" } ;
}

std::ostream& operator<<(std::ostream& os, AVR::uint8 v)
{
  os << std::setfill('0') << std::setw(sizeof(AVR::uint8)*2) << std::hex << (unsigned int)v ;
  return os ;
}

bool CommandRead::Execute(AVR::Mcu &mcu)
{
  const std::string &modeStr = _match[1] ;
  const std::string &addrStr = _match[2] ;
  const std::string &lenStr  = _match[3] ;

  char mode = modeStr[0] ;
  AVR::uint32 addr = std::stoul(addrStr, nullptr, 0) ;
  AVR::uint32 len = (lenStr.size()) ? std::stoul(lenStr, nullptr, 0) : 128 ;

  AVR::uint32 cnt = 0 ;
  for (AVR::uint32 iAddr = addr, eAddr = addr+len ; iAddr < eAddr ; )
  {
    std::cout << std::hex << std::setfill('0') << std::setw(4) << iAddr << ':' ;

    unsigned char data[16] ;
    for (cnt = 0 ; (cnt < 16) && (iAddr < eAddr) ; ++iAddr, ++cnt)
    {
      switch (mode)
      {
      case 'd': data[cnt] = mcu.Data  ((AVR::uint32)iAddr, (bool)false) ; break ;
      case 'e': data[cnt] = mcu.Eeprom((size_t)     iAddr, (bool)false) ; break ;
      }
    }

    for (unsigned int i = 0 ; i < 16 ; ++i)
    {
      if (i < cnt)
        std::cout << ' ' << data[i] ;
      else
        std::cout << "   " ;
      if ((i & 0x03) == 0x03) std::cout << ' ' ;
    }
    std::cout << "    " ;
    for (unsigned int i = 0 ; i < cnt ; ++i)
    {
      unsigned char ch = data[i] ;
      std::cout << (char)(((' ' <= ch) && (ch <= '~')) ? ch : '.') ;
    }

    std::cout << std::endl ;      
  }

  std::cout << std::endl ;

  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandList
////////////////////////////////////////////////////////////////////////////////
class CommandList : public Command
{
public:
  CommandList() : Command(R"XXX(\s*l\s*(?:)XXX" + _reNum + R"XXX(|(?:)XXX" + _reAddr + R"XXX((?:\s+)XXX" + _reNum + R"XXX()?))?\s*)XXX") { }
  ~CommandList() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandList::Help() const
{
  return strings { "l [[<addr>|<label>] <count>]    -- list source"} ;
}
bool CommandList::Execute(AVR::Mcu &mcu)
{
  AVR::uint32 pc0   = mcu.PC() ;
  AVR::uint32 addr  = pc0 ;
  AVR::uint32 count = 20 ;
  const std::string &mCnt2 = _match[1] ;
  const std::string &mNum = _match[2] ;
  const std::string &mLbl = _match[3] ;
  const std::string &mCnt1 = _match[4] ;

  if (mCnt2.size())
  {
    Num(mCnt2, count) ;
  }
  else if (mNum.size() || mLbl.size())
  {
    if (!Addr(mcu, mNum, mLbl, addr))
    {
      std::cout << "illegal value" << std::endl ;
      return false ;
    }
    if (mCnt1.size())
      Num(mCnt1, count) ;
  }

  mcu.PC() = addr ;
  for (AVR::uint32 iAddr = addr, eAddr = addr+count ; iAddr < eAddr ; ++iAddr)
    std::cout << mcu.Disasm() << std::endl ;
  std::cout << std::endl ;
  
  mcu.PC() = pc0 ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandListLabels
////////////////////////////////////////////////////////////////////////////////
class CommandListLabels : public Command
{
public:
  CommandListLabels() : Command(R"XXX(\s*ll\s*([_0-9a-zA-Z]+)?\s*)XXX") { }
  ~CommandListLabels() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandListLabels::Help() const
{
  return strings { "ll [pattern]            -- list labels"} ;
}
bool CommandListLabels::Execute(AVR::Mcu &mcu)
{
  std::string pattern = _match[1] ;
  
  for (const auto &iXref : mcu.XrefByLabel())
  {
    const AVR::Mcu::Xref *xref = iXref.second ;
    if (!pattern.size() || (xref->_label.find(pattern) != std::string::npos))
    {
      char buff[32] ;
      sprintf(buff, "[%05x] ", xref->_addr) ;
      std::cout << buff << xref->_label ;
      if (xref->_description.size())
        std::cout << " -- " << xref->_description.c_str() ;
      std::cout << std::endl ;
    }
  }
  std::cout << std::endl ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandQuit
////////////////////////////////////////////////////////////////////////////////
class CommandQuit : public Command
{
public:
  CommandQuit(bool &quit) : Command(R"XXX(\s*q\s*)XXX"), _quit(quit) { }
  ~CommandQuit() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  bool &_quit ;
} ;

strings CommandQuit::Help() const
{
  return strings { "q                       -- quit"} ;
}
bool CommandQuit::Execute(AVR::Mcu &mcu)
{
  _quit = true ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandRepeat
////////////////////////////////////////////////////////////////////////////////
class CommandRepeat : public Command
{
public:
  CommandRepeat(Command *&lastCommand) : Command(R"XXX(\s*)XXX"), _lastCommand(lastCommand), _lastCommand0(nullptr) { }
  ~CommandRepeat() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  Command *&_lastCommand ;
  Command *_lastCommand0 ;
} ;

strings CommandRepeat::Help() const
{
  return strings { "<empty line>            -- repeat last command"} ;
}
bool CommandRepeat::Execute(AVR::Mcu &mcu)
{
  if ((_lastCommand == this) && _lastCommand0)
    return _lastCommand0->Execute(mcu) ;
  if (_lastCommand)
  {
    _lastCommand0 = _lastCommand ;
    return _lastCommand0->Execute(mcu) ;
  }
  return false ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandHelp
////////////////////////////////////////////////////////////////////////////////
class CommandHelp : public Command
{
public:
  CommandHelp(std::vector<Command*> &commands) : Command(R"XXX(\s*[?h]\s*)XXX"), _commands(commands) { }
  ~CommandHelp() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  std::vector<Command*> &_commands ;
} ;

strings CommandHelp::Help() const
{
  return strings { "h                       -- help",
                   "?                       -- help"} ;
}
bool CommandHelp::Execute(AVR::Mcu &mcu)
{
  std::cout << std::endl ;
  for (Command *command : _commands)
  {
    for (const auto &help : command->Help())
      std::cout << help << std::endl ;
  }
  std::cout << std::endl ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandUnknown
////////////////////////////////////////////////////////////////////////////////
class CommandUnknown : public Command
{
public:
  CommandUnknown() : Command(R"XXX(.*)XXX") { }
  ~CommandUnknown() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandUnknown::Help() const
{
  return strings { } ;
}
bool CommandUnknown::Execute(AVR::Mcu &mcu)
{
  std::cout << "unknown command \"" << _match[0] << "\"" << std::endl ;
  std::cout << "type \"?\" for help" << std::endl ;
  std::cout << std::endl ;
  
  return false ;
}

////////////////////////////////////////////////////////////////////////////////
// Execute
////////////////////////////////////////////////////////////////////////////////

void Execute(AVR::Mcu &mcu)
{
  bool quit = false ;
  Command *lastCommand = nullptr ;
  
  std::vector<Command*> commands =
    {
      new CommandRepeat(lastCommand), // first!
      new CommandStep(),
      new CommandRun(),
      new CommandBreakpoint(),
      new CommandGoto(),
      new CommandAssign(),
      new CommandRead(),
      new CommandList(),
      new CommandListLabels(),
      new CommandListBreakpoints(),
      new CommandQuit(quit),
      new CommandHelp(commands),
      new CommandUnknown(), // last!
    } ;

  std::cout << "type \"?\" for help" << std::endl ;
  std::cout << std::endl ;
  
  std::string cmd ;

 //Command* read=commands[6] ;
  
  while (!quit)
  {
    std::size_t pc0 = mcu.PC() ;
    std::cout << mcu.Disasm() << std::endl << "> " << std::flush ;
    mcu.PC() = pc0 ;
    std::getline(std::cin, cmd) ;

    for (Command *command : commands)
    {
      if (command->Match(cmd))
      {
        if (command->Execute(mcu))
          lastCommand = command ;
        else
          lastCommand = nullptr ;
 //read->Match("d ? 0x60 256") ;
 //read->Execute(mcu) ;
        break ;
      }
    }
  }

  for (auto command : commands)
    delete command ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

