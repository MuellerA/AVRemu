////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <map>
#include <regex>
#include <fstream>

#include "avr.h"
#include "instr.h"

#include "execute.h"

////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::function<AVR::Mcu*()> > mcuFactory
{
  { "ATany",         []{ return new AVR::ATany()         ; } },
  { "ATmega48PA",    []{ return new AVR::ATmega48PA()    ; } },
  { "ATmega88PA",    []{ return new AVR::ATmega88PA()    ; } },
  { "ATmega168PA",   []{ return new AVR::ATmega168PA()   ; } },
  { "ATmega328P",    []{ return new AVR::ATmega328P()    ; } },
  { "ATmega8A",      []{ return new AVR::ATmega8A()      ; } },
  { "ATtiny24A",     []{ return new AVR::ATtiny24A()     ; } },
  { "ATtiny44A",     []{ return new AVR::ATtiny44A()     ; } },
  { "ATtiny84A",     []{ return new AVR::ATtiny84A()     ; } },
  { "ATtiny25",      []{ return new AVR::ATtiny25()      ; } },
  { "ATtiny45",      []{ return new AVR::ATtiny45()      ; } },
  { "ATtiny85",      []{ return new AVR::ATtiny85()      ; } },
  { "ATxmega128A4U", []{ return new AVR::ATxmega128A4U() ; } },
  { "ATxmega64A4U",  []{ return new AVR::ATxmega64A4U()  ; } },
  { "ATxmega32A4U",  []{ return new AVR::ATxmega32A4U()  ; } },
  { "ATxmega16A4U",  []{ return new AVR::ATxmega16A4U()  ; } },
} ;

////////////////////////////////////////////////////////////////////////////////

int usage(const char *name)
{
  fprintf(stderr, "usage: %s [-d] [-e] [-ee <macro>] [-m <mcu>] [-x <xref>] [-p <eeProm>] <avr-bin>\n", name) ;
  fprintf(stderr, "       %s -h\n", name) ;
  fprintf(stderr, "use '-h' for a full list of MCUs\n") ;
  return 1 ;
}

int usageFull(const char *name)
{
  fprintf(stderr, "usage: %s [-d] [-e] [-m <mcu>] [-x <xref>] [-p <eeProm>] <avr-bin>\n", name) ;
  fprintf(stderr, "       %s -h\n", name) ;
  fprintf(stderr, "parameter:\n") ;
  fprintf(stderr, "   -m <mcu>    MCU type, see below\n") ;
  fprintf(stderr, "   -d          disassemble file\n") ;
  fprintf(stderr, "   -e          execute file\n") ;
  fprintf(stderr, "   -ee <macro> run macro file <macro>.aem (implies -e)\n") ;
  fprintf(stderr, "   -x <xref>   read/write xref file\n") ;
  fprintf(stderr, "   -p <eeProm> binary file of EEPROM memory\n") ;
  fprintf(stderr, "   <avr-bin>   binary file to be disassembled / executed\n") ;
  fprintf(stderr, "   -h          this help\n") ;
  fprintf(stderr, "Supported MCU types:") ; 
  for (const auto &iFactory : mcuFactory)
  {
    fprintf(stderr, " %s", iFactory.first.c_str()) ;
  }
  fprintf(stderr, "\n") ;
  return 1 ;
}

////////////////////////////////////////////////////////////////////////////////

void ParseXrefFile(AVR::Mcu &mcu, const std::string &xrefFileName)
{
  std::ifstream in ;
  in.open(xrefFileName) ;
  if (in.fail())
  {
    fprintf(stderr, "open file \"%s\" failed\n", xrefFileName.c_str()) ;
    return ;
  }

  std::regex reLine(R"XXX(([jcd])\s+(0x[0-9a-fA-F]+|[0-9]+)\s+([-_:*.a-zA-Z0-9]+)(?:\s+(.*?))?\s*)XXX", std::regex_constants::optimize) ;
  std::regex reEmpty(R"XXX(\s*(?:#.*)?)XXX") ;
  std::smatch match ;
  std::string line ;
  while (!in.eof())
  {
    std::getline(in, line) ;
    if (std::regex_match(line, match, reLine))
    {
      const std::string &typeStr = match[1] ;
      const std::string &addrStr = match[2] ;
      const std::string &label   = match[3] ;
      const std::string &desc    = match[4] ;

      AVR::XrefType type = AVR::XrefType::none ;
      switch (typeStr[0])
      {
      case 'j': type = AVR::XrefType::jmp  ; break ;
      case 'c': type = AVR::XrefType::call ; break ;
      case 'd': type = AVR::XrefType::data ; break ;
      }
    
      uint32_t addr = std::stoul(addrStr, nullptr, 0) ;

      mcu.XrefAdd(AVR::Mcu::Xref(addr, type, label, desc)) ;
    }
    else if (!std::regex_match(line, match, reEmpty))
    {
      fprintf(stderr, "unknown line \"%s\"\n", line.c_str()) ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    usage(argv[0]) ;
    return 1 ;
  }

  int iArg ;
  bool disasm = false ;
  bool execute = false ;
  std::string mcuType = "ATany" ;
  std::string xrefFileName ;
  std::string eepromFileName ;
  std::string macroFileName ;
  
  for (iArg = 1 ; iArg < argc ; ++iArg)
  {
    if (*argv[iArg] != '-')
      break ;
    else if (!strcmp(argv[iArg], "-h"))
      return usageFull(argv[0]) ;
    else if (!strcmp(argv[iArg], "-d"))
      disasm = true ;
    else if (!strcmp(argv[iArg], "-e"))
      execute = true ;
    else if (!strcmp(argv[iArg], "-ee"))
    {
      if (iArg >= argc-1)
        return usage(argv[0]) ;
      execute = true ;
      macroFileName = argv[++iArg] ;
    }
    else if (!strcmp(argv[iArg], "-m"))
    {
      if (iArg >= argc-1)
        return usage(argv[0]) ;
      mcuType = argv[++iArg] ;
    }
    else if (!strcmp(argv[iArg], "-x"))
    {
      if (iArg >= argc-1)
        return usage(argv[0]) ;
      xrefFileName = argv[++iArg] ;
    }
    else if (!strcmp(argv[iArg], "-p"))
    {
      if (iArg >= argc-1)
        return usage(argv[0]) ;
      eepromFileName = argv[++iArg] ;
    }
    else
      return usage(argv[0]) ;
  }
  if (iArg != argc -1)
    return usage(argv[0]) ;

  AVR::Mcu *mcu ;
  auto iFactory = mcuFactory.find(mcuType) ;
  if (iFactory == mcuFactory.end())
    return usage(argv[0]) ;

  mcu = iFactory->second() ;
  std::vector<AVR::Command> prog ;
  prog.reserve(0x20000) ;

  FILE *f = fopen(argv[iArg], "rb") ;
  if (!f)
  {
    fprintf(stderr, "read file \"%s\" failed\n", argv[iArg]) ;
    return 1 ;
  }
  while (true)
  {
    AVR::Command cmds[0x100] ;
    uint32_t nCmd = fread(cmds, sizeof(AVR::Command), 0x100, f) ;
    if (!nCmd)
      break ;
    prog.insert(prog.end(), cmds, cmds + nCmd) ;
  }
  fclose(f) ;
  
  if (eepromFileName.size())
  {
    std::vector<uint8_t> eeprom ;
    eeprom.reserve(mcu->EepromSize()) ;
    FILE *ee = fopen(eepromFileName.c_str(), "rb") ;
    if (!ee)
    {
      fprintf(stderr, "read file \"%s\" failed\n", eepromFileName.c_str()) ;
      return 1 ;
    }
    while (true)
    {
      uint8_t bytes[0x100] ;
      uint32_t nByte = fread(bytes, sizeof(uint8_t), 0x100, ee) ;
      if (!nByte)
        break ;
      eeprom.insert(eeprom.end(), bytes, bytes + nByte) ;
    }
    fclose(ee) ;
    mcu->SetEeprom(0, eeprom) ;
  }
  
  if (xrefFileName.size())
  {
    ParseXrefFile(*mcu, xrefFileName) ;
  }
  
  mcu->PC() = 0 ;
  uint32_t nCommand = mcu->SetFlash(0, prog) ;
  printf("prog size:   %zd\n", prog.size()) ;
  printf("loaded size: %d\n" , nCommand) ;

  uint32_t progEnd = nCommand % mcu->Flash().size() ;

  if (disasm || !execute)
  {
    const AVR::Instruction *instr  = nullptr ;
    
    while (mcu->PC() < nCommand)
    {
      const AVR::Mcu::Xref *xref = mcu->XrefByAddr(mcu->PC()) ;
      if (xref && static_cast<uint32_t>(xref->Type() & AVR::XrefType::call) &&
          ((instr == &AVR::instrRET) || (instr == &AVR::instrRETI) ||
           (instr == &AVR::instrJMP) || (instr == &AVR::instrRJMP) ||
           (instr == &AVR::instrIJMP) || (instr == &AVR::instrEIJMP)))
        printf("\n////////////////////////////////////////////////////////////////////////////////\n\n") ;
      instr = mcu->Instr(mcu->PC()) ;
      
      std::string disasm = mcu->Disasm() ;
      printf("%s\n", disasm.c_str()) ;
      if (mcu->PC() == progEnd)
        break ;
    }
  }
  if (execute)
  {
    AVR::Execute exec(*mcu) ;
    if (!macroFileName.empty())
      exec.Do(std::string("m ") + macroFileName) ;
    exec.Loop() ;
  }

  delete mcu ;
  
  return 0 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
