////////////////////////////////////////////////////////////////////////////////
// execute.cpp
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>
#include <algorithm>
#include <cstdlib>
#include <signal.h>

#include "avr.h"
#include "instr.h"
#include "execute.h"

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
// Util
////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, uint8_t v)
{
  os << std::setfill('0') << std::setw(sizeof(uint8_t)*2) << std::hex << (unsigned int)v ;
  return os ;
}

void Data(AVR::Mcu &mcu, uint32_t addr, uint32_t len, char mode)
{
  uint32_t cnt = 0 ;
  for (uint32_t iAddr = addr, eAddr = addr+len ; iAddr < eAddr ; )
  {
    std::cout << std::hex << std::setfill('0') << std::setw(4) << iAddr << ':' ;

    unsigned char data[16] ;
    for (cnt = 0 ; (cnt < 16) && (iAddr < eAddr) ; ++iAddr, ++cnt)
    {
      switch (mode)
      {
      case 'd': data[cnt] = mcu.Data  ((uint32_t)iAddr, (bool)false) ; break ;
      case 'e': data[cnt] = mcu.Eeprom((size_t)  iAddr, (bool)false) ; break ;
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
}  

void List(AVR::Mcu &mcu, uint32_t addr, uint32_t len)
{
  uint32_t pc0   = mcu.PC() ;
  mcu.PC() = addr ;
  for (uint32_t iAddr = addr, eAddr = addr + len ; iAddr < eAddr ; ++iAddr)
    std::cout << mcu.Disasm() << std::endl ;
  std::cout << std::endl ;
  
  mcu.PC() = pc0 ;
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

  bool Num(const std::string matchNum, uint32_t &num)
  {
    num = std::stoul(matchNum, nullptr, 0) ;
    return true ;
  }
  
  bool Addr(const AVR::Mcu &mcu, const std::string &matchNum, const std::string &matchLbl, uint32_t &addr)
  {
    if (matchLbl.size())
    {
      const AVR::Mcu::Xref *xref = mcu.XrefByLabel(matchLbl) ;
      if (!xref)
        return false ;

      addr = xref->Addr() ;
      return true ;
    }

    addr = std::stoul(matchNum, nullptr, 0) ;
    return true ;
  }
  
  bool AddrOpt(const AVR::Mcu &mcu, const std::string &matchNum, const std::string &matchLbl, uint32_t &addr, uint32_t defaultAddr)
  {
    if (matchLbl.size())
    {
      const AVR::Mcu::Xref *xref = mcu.XrefByLabel(matchLbl) ;
      if (!xref)
        return false ;

      addr = xref->Addr() ;
      return true ;
    }

    if (matchNum.size())
    {
      addr = std::stoul(matchNum, nullptr, 0) ;
      return true ;
    }

    addr = defaultAddr ;
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
  uint32_t count = countStr.size() ? std::stoul(countStr, nullptr, 0) : 1 ;
  switch (mode[0])
  {
  case 's':
    for (uint32_t i = 0 ; (i < count) && !SigInt ; ++i)
    {
      mcu.Execute() ;
      if (mcu.IsBreakpoint())
        break ;
    }
    break ;
  case 'n':
    for (uint32_t i = 0 ; (i < count) && !SigInt ; ++i)
    {
      uint32_t pc = mcu.PC() ;
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
  CommandRun() : Command(R"XXX(\s*r\s*(?:)XXX" + _reAddr + R"XXX()?\s*)XXX")
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
  return strings
  {
    "r                       -- run",
    "r <addr>|<label>        -- run to address",
  } ;
}

bool CommandRun::Execute(AVR::Mcu &mcu)
{
  void (*prevIntHdl)(int) ;
  SigInt = false ;
  prevIntHdl = signal(SIGINT, SigIntHdl) ;

  const std::string &m1 = _match[1] ;
  const std::string &m2 = _match[2] ;

  bool infinity = !m1.size() & !m2.size() ;
  
  uint32_t addr = 0 ;
  if (!infinity && !Addr(mcu, m1, m2, addr))
  {
    std::cout << "illegal value" << std::endl ;
    return false ;
  }
  
  while (!SigInt)
  {
    mcu.Execute() ;
    if (!infinity && (mcu.PC() == addr) ||
        mcu.IsBreakpoint())
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
  uint32_t addr ;
  
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
  CommandListBreakpoints() : Command(R"XXX(\s*b\s*\?\s*)XXX") { }
  ~CommandListBreakpoints() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
  } ;

strings CommandListBreakpoints::Help() const
{
  return strings { "b ?                     -- list breakpoints" } ;
}
bool CommandListBreakpoints::Execute(AVR::Mcu &mcu)
{
  for (auto addr : mcu.Breakpoints())
  {
    const AVR::Mcu::Xref *xref = mcu.XrefByAddr((uint32_t) addr) ;

    std::cout << std::hex << std::setfill('0') << std::setw(4) << addr ;
  
    if (xref)
      std::cout << "    " << xref->Label() ;

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
  return strings { "g <addr>|<label>        -- set PC to address" } ;
}

bool CommandGoto::Execute(AVR::Mcu &mcu)
{
  uint32_t addr ;
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
    "r<d>     = byte         -- set register",
    "d <addr> = byte         -- set data memory",
    "p <addr> = word         -- set program memory"
  } ;
}

bool CommandAssign::Execute(AVR::Mcu &mcu)
{
  char typ = _match[1].str()[0] ;
  uint32_t idx = std::stoul(_match[2], nullptr, 0) ;
  uint32_t val = std::stoul(_match[3], nullptr, 0) ;

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
  CommandRead() : Command(R"XXX(\s*([de])\s*(0x[0-9a-fA-F]+|[0-9]+)\s*\?\s*(0x[0-9a-fA-F]+|[0-9]+)?\s*)XXX") { }
  ~CommandRead() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandRead::Help() const
{
  return strings { "d <addr> ? [<len>]      -- read memory content" } ;
}

bool CommandRead::Execute(AVR::Mcu &mcu)
{
  const std::string &modeStr = _match[1] ;
  const std::string &addrStr = _match[2] ;
  const std::string &lenStr  = _match[3] ;

  char mode = modeStr[0] ;
  uint32_t addr = std::stoul(addrStr, nullptr, 0) ;
  uint32_t len = (lenStr.size()) ? std::stoul(lenStr, nullptr, 0) : 128 ;

  Data(mcu, addr, len, mode) ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandReadDataIndirect
////////////////////////////////////////////////////////////////////////////////
class CommandReadDataIndirect : public Command
{
public:
  CommandReadDataIndirect() : Command(R"XXX(\s*d\s*@\s*(X|Y|Z|SP|r[02468]|r[12][02468]|r30)\s*\?\s*(0x[0-9a-fA-F]+|[0-9]+)?\s*)XXX") { }
  ~CommandReadDataIndirect() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandReadDataIndirect::Help() const
{
  return strings { "d @ <X|Y|Z|SP|rXX> ? [<len>]   -- read memory content" } ;
}

bool CommandReadDataIndirect::Execute(AVR::Mcu &mcu)
{
  const std::string &addrStr = _match[1] ;
  const std::string &lenStr  = _match[2] ;

  uint32_t addr = 0 ;
  if      (addrStr == "X")  addr = mcu.RegW(26) ;
  else if (addrStr == "Y")  addr = mcu.RegW(28) ;
  else if (addrStr == "Z")  addr = mcu.RegW(30) ;
  else if (addrStr == "SP") addr = mcu.GetSP() ;
  else                      addr = mcu.RegW(std::stoul(addrStr.substr(1), nullptr, 10)) ;
  
  uint32_t len = (lenStr.size()) ? std::stoul(lenStr, nullptr, 0) : 128 ;

  Data(mcu, addr, len, 'd') ;

  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandList
////////////////////////////////////////////////////////////////////////////////
class CommandList : public Command
{
public:
  CommandList() : Command(R"XXX(\s*p\s*)XXX" + _reAddr + R"XXX(?\s*\?\s*)XXX" + _reNum + R"XXX(?\s*)XXX") {}
  ~CommandList() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandList::Help() const
{
  return strings { "p [<addr>|<label>] ? [<count>] -- list source"} ;
}
bool CommandList::Execute(AVR::Mcu &mcu)
{
  uint32_t pc0   = mcu.PC() ;
  uint32_t addr  = pc0 ;
  uint32_t count = 20 ;
  const std::string &mNum = _match[1] ;
  const std::string &mLbl = _match[2] ;
  const std::string &mCnt = _match[3] ;

  if (!AddrOpt(mcu, mNum, mLbl, addr, mcu.PC()))
  {
    std::cout << "illegal value" << std::endl ;
    return false ;
  } 

  if (mCnt.size())
    Num(mCnt, count) ;

  List(mcu, addr, count) ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandListIndirect
////////////////////////////////////////////////////////////////////////////////
class CommandListIndirect : public Command
{
public:
  CommandListIndirect() : Command(R"XXX(\s*p\s@\s*(X|Y|Z|r[02468]|r[12][02468]|r30)\s*\?\s*)XXX" + _reNum + R"XXX(?\s*)XXX") {}
  ~CommandListIndirect() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandListIndirect::Help() const
{
  return strings { "p @ <X|Y|Z|rXX> ? [<count>]    -- list source"} ;
}

bool CommandListIndirect::Execute(AVR::Mcu &mcu)
{
  const std::string &mAddr = _match[1] ;
  const std::string &mCnt  = _match[2] ;

  uint32_t addr = 0 ;
  if      (mAddr == "X")  addr = mcu.RegW(26) ;
  else if (mAddr == "Y")  addr = mcu.RegW(28) ;
  else if (mAddr == "Z")  addr = mcu.RegW(30) ;
  else                    addr = mcu.RegW(std::stoul(mAddr.substr(1), nullptr, 10)) ;

  uint32_t len = (mCnt.size()) ? std::stoul(mCnt, nullptr, 0) : 128 ;

  List(mcu, addr, len) ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandListSymbols
////////////////////////////////////////////////////////////////////////////////
class CommandListSymbols : public Command
{
public:
  CommandListSymbols() : Command(R"XXX(\s*ls\s*([_0-9a-zA-Z]+)?\s*)XXX") { }
  ~CommandListSymbols() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;
} ;

strings CommandListSymbols::Help() const
{
  return strings { "ls [pattern]            -- list symbols"} ;
}
bool CommandListSymbols::Execute(AVR::Mcu &mcu)
{
  std::string pattern = _match[1] ;
  
  for (const auto &iXref : mcu.XrefByLabel())
  {
    const AVR::Mcu::Xref *xref = iXref.second ;
    if (!pattern.size() || (xref->Label().find(pattern) != std::string::npos))
    {
      char buff[32] ;
      sprintf(buff, "[%05x] ", xref->Addr()) ;
      std::cout << buff << xref->Label() ;
      if (xref->Description().size())
        std::cout << " -- " << xref->Description().c_str() ;
      std::cout << std::endl ;
    }
  }
  std::cout << std::endl ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandIoAddHex
////////////////////////////////////////////////////////////////////////////////
class CommandIoAddHex : public Command
{
public:
  CommandIoAddHex() : Command(R"XXX(\s*io\s*([_0-9a-zA-Z]+)\s*=((?:\s*)XXX" + _reNum + R"XXX()+)\s*)XXX") {}
  ~CommandIoAddHex() {}

  virtual strings Help() const ;
  virtual bool Execute(AVR::Mcu &mcu) ;
} ;

strings CommandIoAddHex::Help() const
{
  return strings { "io <name> = <hex>       -- set next io read values (hex)" } ;
}
bool CommandIoAddHex::Execute(AVR::Mcu &mcu)
{
  const std::string &name = _match[1] ;
  const std::string &hex  = _match[2] ;

  auto &io = mcu.Io() ;
  auto iIo = std::find_if(io.begin(), io.end(), [&name](const AVR::Io::Register *ioReg){ return ioReg && (ioReg->Name() == name) ; }) ;
  if (iIo == io.end())
  {
    std::cout << "illegal value" << std::endl ;
    return false ;
  }

  std::vector<uint8_t> data ;
  const char *str = hex.c_str() ;
  char *str_end = const_cast<char*>(str) ; ;

  while (*str)
  {
    uint32_t ul = strtoul(str, &str_end, 0) ;
    if (ul > 255)
    {
      std::cout << "illegal value " << ul << std::endl ;
      return false ;
    }
    data.push_back(ul) ;
    str = str_end ;
  }
  (*iIo)->Add(data) ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandIoAddAsc
////////////////////////////////////////////////////////////////////////////////
class CommandIoAddAsc : public Command
{
public:
  CommandIoAddAsc() : Command(R"XXX(\s*io\s*([_0-9a-zA-Z]+)\s=\s*"(.+)"\s*)XXX") {}
  ~CommandIoAddAsc() {}

  virtual strings Help() const ;
  virtual bool Execute(AVR::Mcu &mcu) ;
} ;

strings CommandIoAddAsc::Help() const
{
  return strings { "io <name> = \"<asc>\"     -- set next io read values (asc)" } ;
}
bool CommandIoAddAsc::Execute(AVR::Mcu &mcu)
{
  const std::string &name = _match[1] ;
  const std::string &asc  = _match[2] ;

  auto &io = mcu.Io() ;
  auto iIo = std::find_if(io.begin(), io.end(), [&name](const AVR::Io::Register *ioReg){ return ioReg && (ioReg->Name() == name) ; }) ;
  if (iIo == io.end())
  {
    std::cout << "illegal value" << std::endl ;
    return false ;
  }

  std::vector<uint8_t> data ;
  data.reserve(asc.size()) ;
  for (auto c : asc)
    data.push_back((uint8_t)c) ;
  (*iIo)->Add(data) ;
  
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandTrace
////////////////////////////////////////////////////////////////////////////////
class CommandTrace : public Command
{
public:
  CommandTrace() : Command{R"XXX(\s*t\s+(?:(?:on\s+([-_/.0-9a-zA-Z]+))?(?:\s+)XXX" + _reAddr + R"XXX()?|off)\s*)XXX"} {}
  ~CommandTrace() {}

  virtual strings Help() const ;
  virtual bool Execute(AVR::Mcu &mcu) ;
} ;

strings CommandTrace::Help() const
{
  return strings
  {
    "t on <name> [addr]      -- log to trace file until addr is reached (default 0x00000)",
    "t off                   -- close trace file",
  } ;
}
bool CommandTrace::Execute(AVR::Mcu &mcu)
{
  const std::string &name = _match[1] ;
  const std::string &num  = _match[2] ;
  const std::string &lbl  = _match[3] ;

  if (name.size())
  {
    uint32_t addr ;
    if (!AddrOpt(mcu, num, lbl, addr, 0x0000))
    {
      std::cout << "illegal value" << std::endl ;
      return false ;
    }
    mcu.TraceOn(name, addr) ;
  }
  else
  {
    mcu.TraceOff() ;
  }
  
  return false ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandMacro
////////////////////////////////////////////////////////////////////////////////
class CommandMacro : public Command
{
public:
  CommandMacro(AVR::Execute &exec) : Command{R"XXX(\s*m\s+([-_/.0-9a-zA-Z]+)\s*)XXX"}, _exec{exec}, _reEmpty{R"XXX(\s*(?:#.*)?)XXX"} {}
  ~CommandMacro() {}

  virtual strings Help() const ;
  virtual bool Execute(AVR::Mcu &mcu) ;

private:
  AVR::Execute &_exec ;
  std::regex _reEmpty ;
} ;

strings CommandMacro::Help() const
{
  return strings { "m <name>                -- run macro file <name>.aem" } ;
}
bool CommandMacro::Execute(AVR::Mcu &mcu)
{
  std::smatch match ;
  const std::string &macro = _match[1] ;

  std::ifstream ifs ;

  ifs.open((macro + ".aem").c_str()) ;
  if (ifs.fail())
  {
    std::cout << "failed to read macro file " << macro << std::endl ;
    return false ;
  }

  _exec.MacroQuit(false) ;
  while (!ifs.eof() && !_exec.IsQuit() && !_exec.MacroQuit())
  {
    std::string cmd ;
    std::getline(ifs, cmd) ;

    if (std::regex_match(cmd, match, _reEmpty))
      continue ;
    
    std::cout << std::endl << cmd << std::endl ;
    _exec.Do(cmd) ;
  }
  _exec.MacroQuit(false) ;

  return false ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandMacroQuit
////////////////////////////////////////////////////////////////////////////////
class CommandMacroQuit : public Command
{
public:
  CommandMacroQuit(AVR::Execute &exec) : Command(R"XXX(\s*mq\s*)XXX"), _exec(exec) { }
  ~CommandMacroQuit() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  AVR::Execute &_exec ;
} ;

strings CommandMacroQuit::Help() const
{
  return strings { "mq                      -- quit macro execution"} ;
}
bool CommandMacroQuit::Execute(AVR::Mcu &mcu)
{
  _exec.MacroQuit(true) ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandEcho
////////////////////////////////////////////////////////////////////////////////
class CommandEcho : public Command
{
public:
  CommandEcho() : Command{R"XXX(\s*\$\s*(.+?)\s*)XXX"} {}
  ~CommandEcho() {}

  virtual strings Help() const ;
  virtual bool Execute(AVR::Mcu &mcu) ;
} ;

strings CommandEcho::Help() const
{
  return strings { "$ <text>                -- write text to output / only useful in macros" } ;
}
bool CommandEcho::Execute(AVR::Mcu &mcu)
{
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandQuit
////////////////////////////////////////////////////////////////////////////////
class CommandQuit : public Command
{
public:
  CommandQuit(AVR::Execute &exec) : Command(R"XXX(\s*q\s*)XXX"), _exec(exec) { }
  ~CommandQuit() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  AVR::Execute &_exec ;
} ;

strings CommandQuit::Help() const
{
  return strings { "q                       -- quit"} ;
}
bool CommandQuit::Execute(AVR::Mcu &mcu)
{
  _exec.Quit() ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// CommandRepeat
////////////////////////////////////////////////////////////////////////////////
class CommandRepeat : public Command
{
public:
  CommandRepeat(AVR::Execute &exec) : Command(R"XXX(\s*)XXX"), _exec(exec), _lastCommand0(nullptr) { }
  ~CommandRepeat() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  AVR::Execute& _exec ;
  Command *_lastCommand0 ;
} ;

strings CommandRepeat::Help() const
{
  return strings { "<empty line>            -- repeat last command"} ;
}
bool CommandRepeat::Execute(AVR::Mcu &mcu)
{
  if ((_exec.LastCommand() == this) && _lastCommand0)
    return _lastCommand0->Execute(mcu) ;
  if (_exec.LastCommand())
  {
    _lastCommand0 = _exec.LastCommand() ;
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
  CommandHelp(AVR::Execute &exec) : Command(R"XXX(\s*[?h]\s*)XXX"), _exec(exec) { }
  ~CommandHelp() { }

  virtual strings Help() const ;
  virtual bool    Execute(AVR::Mcu &mcu) ;

private:
  AVR::Execute &_exec ;
} ;

strings CommandHelp::Help() const
{
  return strings { "h                       -- help",
                   "?                       -- help"} ;
}
bool CommandHelp::Execute(AVR::Mcu &mcu)
{
  std::cout << std::endl ;
  for (const Command *command : _exec.Commands())
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

namespace AVR
{

  Execute::Execute(AVR::Mcu &mcu) :
    _mcu{mcu},
    _commands
    {
      new CommandRepeat(*this), // first!
      new CommandStep(),
      new CommandRun(),
      new CommandGoto(),
      new CommandBreakpoint(),
      new CommandListBreakpoints(),
      new CommandRead(),
      new CommandReadDataIndirect(),
      new CommandList(),
      new CommandListIndirect(),
      new CommandAssign(),
      new CommandListSymbols(),
      new CommandIoAddHex(),
      new CommandIoAddAsc(),
      new CommandMacro(*this),
      new CommandMacroQuit(*this),
      new CommandTrace(),
      new CommandEcho(),
      new CommandQuit(*this),
      new CommandHelp(*this),
      new CommandUnknown(), // last!
    },
    _quit{false},
    _lastCommand{nullptr}
  {
    std::cout << "type \"?\" for help" << std::endl ;
    std::cout << std::endl ;
  }

  Execute::~Execute()
  {
    for (auto command : _commands)
      delete command ;
  }

  void Execute::Loop()
  {
    std::string cmd ;

    while (!_quit)
    {
      std::size_t pc0 = _mcu.PC() ;
      std::cout << _mcu.Disasm() << std::endl << "> " << std::flush ;
      _mcu.PC() = pc0 ;
      std::getline(std::cin, cmd) ;

      Do(cmd) ;
    }
  }

  void Execute::Do(const std::string &cmd)
  {
    for (::Command *command : _commands)
    {
      if (command->Match(cmd))
      {
        if (command->Execute(_mcu))
          _lastCommand = command ;
        else
          _lastCommand = nullptr ;
        break ;
      }
    }
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
