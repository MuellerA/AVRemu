////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <map>

#include "avr.h"
#include "instr.h"

extern void Execute(AVR::Mcu &mcu) ;

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
  fprintf(stderr, "usage: %s [-d] [-x] [-m <mcu>] <avr-bin>\n", name) ;
  fprintf(stderr, "       %s -h\n", name) ;
  fprintf(stderr, "use '-h' for a full list of MCUs\n") ;
  return 1 ;
}

int usageFull(const char *name)
{
  fprintf(stderr, "usage: %s [-d] [-x] [-m <mcu>] <avr-bin>\n", name) ;
  fprintf(stderr, "       %s -h\n", name) ;
  fprintf(stderr, "parameter:\n") ;
  fprintf(stderr, "   -m <mcu>    MCU type, see below\n") ;
  fprintf(stderr, "   -d          disassemble file\n") ;
  fprintf(stderr, "   -x          execute file\n") ;
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
  const char *mcuType = "ATany" ;
  for (iArg = 1 ; iArg < argc ; ++iArg)
  {
    if (*argv[iArg] != '-')
      break ;
    else if (!strcmp(argv[iArg], "-h"))
      return usageFull(argv[0]) ;
    else if (!strcmp(argv[iArg], "-d"))
      disasm = true ;
    else if (!strcmp(argv[iArg], "-x"))
      execute = true ;
    else if (!strcmp(argv[iArg], "-m"))
    {
      if (iArg >= argc-1)
        return usage(argv[0]) ;
      mcuType = argv[++iArg] ;
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
    fprintf(stderr, "read file \"%s\" failed\n", argv[1]) ;
    return 1 ;
  }

  while (true)
  {
    AVR::Command cmds[0x100] ;
    size_t nCmd = fread(cmds, sizeof(AVR::Command), 0x100, f) ;
    if (!nCmd)
      break ;
    prog.insert(prog.end(), cmds, cmds + nCmd) ;
  }
  fclose(f) ;

  mcu->PC() = 0 ;
  size_t nCommand = mcu->SetProgram(0, prog) ;
  printf("prog size:   %zd\n", prog.size()) ;
  printf("loaded size: %zd\n", nCommand) ;

  size_t progEnd = nCommand % mcu->Program().size() ;

  if (disasm || !execute)
  {
    while (mcu->PC() < nCommand)
    {
      std::string disasm = mcu->Disasm() ;
      printf("%s\n", disasm.c_str()) ;
      if (mcu->PC() == progEnd)
        break ;
    }
  }
  if (execute)
  {
    Execute(*mcu) ;
  }

  delete mcu ;
  
  return 0 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
