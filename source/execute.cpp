////////////////////////////////////////////////////////////////////////////////
// execute.cpp
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
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
  std::string _command ;
  std::regex  _regex ;
  std::smatch _match ;
} ;

bool Command::Match(const std::string &command)
{
  _command = command ;
  return std::regex_match(_command, _match, _regex) ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandStep
////////////////////////////////////////////////////////////////////////////////
class CommandStep : public Command
{
public:
  CommandStep() : Command(R"XXX(\s*([sn])\s*(?:([\d]+)\s*)?)XXX")
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
  return strings { "s [count]        -- step in count instructions",
                   "n [count]        -- step over count instructions" } ;
}

bool CommandStep::Execute(AVR::Mcu &mcu)
{
  void (*prevIntHdl)(int) ;
  SigInt = false ;
  prevIntHdl = signal(SIGINT, SigIntHdl) ;
  
  const std::string &mode = _match[1] ;
  const std::string &countStr = _match[2] ;
  AVR::uint32 count = countStr.size() ? std::stoul(countStr) : 1 ;
  switch (mode[0])
  {
  case 's':
    for (AVR::uint32 i = 0 ; (i < count) && !SigInt ; ++i)
      mcu.Execute() ;
    break ;
  case 'n':
    for (AVR::uint32 i = 0 ; (i < count) && !SigInt ; ++i)
    {
      AVR::uint32 pc = mcu.PC() ;
      const AVR::Instruction *instr = mcu.Instr(pc) ;
      if (instr->IsCall())
      {
        pc += (instr->IsTwoWord()) ? 2 : 1 ;
        while ((mcu.PC() != pc) && !SigInt)
          mcu.Execute() ;
      }
      else
        mcu.Execute() ;
    }
    break ;
  }
  signal(SIGINT, prevIntHdl) ;

  mcu.Status() ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandGoto
////////////////////////////////////////////////////////////////////////////////
class CommandGoto : public Command
{
public:
  CommandGoto() : Command(R"XXX(\s*g\s*(?:(0x[0-9a-fA-F]+|[0-9]+)|([_0-9a-zA-Z]+))\s*)XXX")
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
  return strings { "g <addr>|<label> -- goto address/label" } ;
}

bool CommandGoto::Execute(AVR::Mcu &mcu)
{
  const std::string &addrLabel = _match[2] ;

  if (addrLabel.size())
  {
    const AVR::Mcu::Xref *xref = mcu.XrefByLabel(addrLabel) ;
    if (!xref)
    {
      std::cout << "illegal value" << std::endl ;
      return false ;
    }    
    mcu.PC() = xref->_addr ;
    return true ;
  }
  
  const std::string &addrStr = _match[1] ;
  AVR::uint32 addr = std::stoul(addrStr, nullptr, 0) ;
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
    "r<d>    = byte   -- set register",
    "d<addr> = byte   -- set data memory",
    "p<addr> = word   -- set program memory"
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
    mcu.Data(idx, val) ;
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
// CommandList
////////////////////////////////////////////////////////////////////////////////
class CommandList : public Command
{
public:
  CommandList() : Command(R"XXX(\s*l\s*(?:(0x[0-9a-fA-F]+|[0-9]+)\s*(?:(0x[0-9a-fA-F]+|[0-9]+)\s*)?)?)XXX") { }
  ~CommandList() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandList::Help() const
{
  return strings { "l [[<addr>] <count>] -- list source"} ;
}
bool CommandList::Execute(AVR::Mcu &mcu)
{
  AVR::uint32 pc0   = mcu.PC() ;
  AVR::uint32 addr  = pc0 ;
  AVR::uint32 count = 20 ;
  const std::string &m2 = _match[2] ;
  const std::string &m1 = _match[1] ;
  
  if (m2.size())
  {
    count = std::stoul(m2, nullptr, 0) ;
    addr  = std::stoul(m1, nullptr, 0) ;
  }
  else if (m1.size())
  {
    count = std::stoul(m1, nullptr, 0) ;
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
  CommandListLabels() : Command(R"XXX(\s*ll\s*)XXX") { }
  ~CommandListLabels() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandListLabels::Help() const
{
  return strings { "ll                   -- list labels"} ;
}
bool CommandListLabels::Execute(AVR::Mcu &mcu)
{
  for (const auto &iXref : mcu.XrefByLabel())
  {
    const AVR::Mcu::Xref *xref = iXref.second ;
    char buff[32] ;
    sprintf(buff, "[%05x] ", xref->_addr) ;
    std::cout << buff << xref->_label ;
    if (xref->_description.size())
      std::cout << " -- " << xref->_description.c_str() ;
    std::cout << std::endl ;
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
  return strings { "q                -- quit"} ;
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
  return strings { "<empty line>     -- repeat last command"} ;
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
  return strings { "?                -- help"} ;
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
      new CommandGoto(),
      new CommandAssign(),
      new CommandList(),
      new CommandListLabels(),
      new CommandQuit(quit),
      new CommandHelp(commands),
      new CommandUnknown(), // last!
    } ;

  std::cout << "type \"?\" for help" << std::endl ;
  
  std::string cmd ;
  
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

