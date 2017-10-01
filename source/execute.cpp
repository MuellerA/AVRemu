////////////////////////////////////////////////////////////////////////////////
// execute.cpp
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <regex>

#include "avr.h"
#include "instr.h"

////////////////////////////////////////////////////////////////////////////////

using strings = std::vector<std::string> ;

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
  CommandStep() : Command(R"XXX(\s*s\s*(?:([\d]+)\s*)?)XXX")
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
  return strings { "s [count]       -- step count instructions" } ;
}

bool CommandStep::Execute(AVR::Mcu &mcu)
{
  const std::string &countStr = _match[1] ;
  AVR::uint32 count = countStr.size() ? std::stoul(countStr) : 1 ;
  for (AVR::uint32 i = 0 ; i < count ; ++i)
    mcu.Execute() ;
  mcu.Status() ;
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
    "r<d>    = byte  -- assign register",
    "d<addr> = byte  -- assign data memory",
    "p<addr> = word  -- assign program memory"
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
  return strings { "q               -- quit"} ;
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
  return strings { "<empty line>    -- repeat last command"} ;
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
  return strings { "?               -- help"} ;
}
bool CommandHelp::Execute(AVR::Mcu &mcu)
{
  for (Command *command : _commands)
  {
    for (const auto &help : command->Help())
      std::cout << help << std::endl ;
  }

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
      new CommandStep(),
      new CommandAssign(),
      new CommandQuit(quit),
      new CommandRepeat(lastCommand),
      new CommandHelp(commands),
      new CommandUnknown(), // last!
    } ;

  std::cout << "type \"?\" for help" << std::endl ;
  
  std::string cmd ;
  
  while (!quit)
  {
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

