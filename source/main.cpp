////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <algorithm>
#include <functional>
#include <map>
#include <string.h>

#include "avr.h"
#include "instr.h"

////////////////////////////////////////////////////////////////////////////////
// test
////////////////////////////////////////////////////////////////////////////////

int test()
{
  AVR::ATany avr ;

  std::vector<AVR::Command> prog ;
  prog.reserve(0x20000) ;

  {
    for (auto *instr : avr.Instructions())
    {
      if (instr)
      {
        bool is2word = false ;

        is2word |= instr->Pattern() == 0b1001010000001100 ; // JMP
        is2word |= instr->Pattern() == 0b1001010000001110 ; // JMP
        is2word |= instr->Pattern() == 0b1001000000000000 ; // LDS
        is2word |= instr->Pattern() == 0b1001001000000000 ; // STS

        AVR::Command min = instr->Pattern() & instr->Mask() ;
        prog.push_back(min) ;
        if (is2word) prog.push_back(0x0000) ;

        AVR::Command max = instr->Pattern() + ~instr->Mask() ;
        prog.push_back(max) ;
        if (is2word) prog.push_back(0xffff) ;
      }
    }

    avr.PC() = 0 ;
    size_t nCommand = avr.SetProgram(0, prog) ;
    printf("prog size: %ld\n", prog.size()) ;

    for (size_t iCommand = 0 ; iCommand < nCommand ; ++iCommand)
    {
      std::string disasm = avr.Disasm() ;
      printf("%s\n", disasm.c_str()) ;
    }
  }

  printf("================================================================================\n") ;

  FILE *f = fopen("flash328.orig.bin", "rb") ;
  if (f)
  {
    prog.clear() ;
    while (true)
    {
      AVR::Command cmds[0x100] ;
      size_t nCmd = fread(cmds, sizeof(AVR::Command), 0x100, f) ;
      if (!nCmd)
        break ;
      prog.insert(prog.end(), cmds, cmds + nCmd) ;
    }
    avr.PC() = 0 ;
    size_t nCommand = avr.SetProgram(0, prog) ;
    printf("prog size: %ld\n", prog.size()) ;

    for (size_t iCommand = 0 ; iCommand < nCommand ; ++iCommand)
    {
      std::string disasm = avr.Disasm() ;
      printf("%s\n", disasm.c_str()) ;
    }
  }

  return 0 ;
}

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
  fprintf(stderr, "usage: %s [-mcu <mcu>] <avr-bin>\n", name) ;
  fprintf(stderr, "       %s -h\n", name) ;
  fprintf(stderr, "use '-h' for a full list of MCUs\n") ;
  return 1 ;
}

int usageFull(const char *name)
{
  fprintf(stderr, "usage: %s [-mcu <mcu>] <avr-bin>\n", name) ;
  fprintf(stderr, "       %s -h\n", name) ;
  fprintf(stderr, "parameter:\n") ;
  fprintf(stderr, "   -mcu <mcu>  MCU type, see below\n") ;
  fprintf(stderr, "   <avr-bin>   binary file to be disassembled\n") ;
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
  const char *mcuType = "ATany" ;
  for (iArg = 1 ; iArg < argc ; ++iArg)
  {
    if (*argv[iArg] != '-')
      break ;
    else if (!strcmp(argv[iArg], "-h"))
      return usageFull(argv[0]) ;
    else if (!strcmp(argv[iArg], "-mcu"))
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
  
  FILE *f = fopen(argv[iArg], "rb") ;
  if (!f)
  {
    fprintf(stderr, "read file \"%s\" failed\n", argv[1]) ;
    return 1 ;
  }

  std::vector<AVR::Command> prog ;
  prog.reserve(0x20000) ;

  while (true)
  {
    AVR::Command cmds[0x100] ;
    size_t nCmd = fread(cmds, sizeof(AVR::Command), 0x100, f) ;
    if (!nCmd)
      break ;
    prog.insert(prog.end(), cmds, cmds + nCmd) ;
  }
  mcu->PC() = 0 ;
  size_t nCommand = mcu->SetProgram(0, prog) ;
  printf("prog size:   %ld\n", prog.size()) ;
  printf("loaded size: %ld\n", nCommand) ;

  size_t progEnd = nCommand % mcu->Program().size() ;
  
  while (mcu->PC() < nCommand)
  {
    std::string disasm = mcu->Disasm() ;
    printf("%s\n", disasm.c_str()) ;
    if (mcu->PC() == progEnd)
      break ;
  }

  fclose(f) ;

  delete mcu ;
  
  return 0 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
