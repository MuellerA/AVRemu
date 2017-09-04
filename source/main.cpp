////////////////////////////////////////////////////////////////////////////////
// avrEmu.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <algorithm>
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
  avr.SetProgram(0, prog) ;
  printf("prog size: %ld\n", prog.size()) ;

  while (avr.PC() < prog.size())
  {
    std::string disasm = avr.Disasm() ;
    printf("%s\n", disasm.c_str()) ;
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
    avr.SetProgram(0, prog) ;
    printf("prog size: %ld\n", prog.size()) ;

    while (avr.PC() < prog.size())
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

int usage(const char *name)
{
  fprintf(stderr, "usage: %s -h\n", name) ;
  fprintf(stderr, "          this help\n") ;
  fprintf(stderr, "       %s [-mcu <mcu>] <avr-bin>\n", name) ;
  fprintf(stderr, "          <mcu> is one of 'ATmega48PA', 'ATmega88PA', 'ATmega168PA', 'ATmega328P'\n") ;
  fprintf(stderr, "          <avr-bin> is the binary file\n") ;
  return 1 ;
}

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
      return usage(argv[0]) ;
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
  if      (!strcmp(mcuType, "ATany"      )) mcu = new AVR::ATmega48PA()  ;
  else if (!strcmp(mcuType, "ATmega48PA" )) mcu = new AVR::ATmega88PA()  ;
  else if (!strcmp(mcuType, "ATmega88PA" )) mcu = new AVR::ATmega88PA()  ;
  else if (!strcmp(mcuType, "ATmega168PA")) mcu = new AVR::ATmega168PA() ;
  else if (!strcmp(mcuType, "ATmega328P" )) mcu = new AVR::ATmega328P()  ;
  else return usage(argv[0]) ;
  
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
  mcu->SetProgram(0, prog) ;
  printf("prog size: %ld\n", prog.size()) ;

  while (mcu->PC() < prog.size())
  {
    std::string disasm = mcu->Disasm() ;
    printf("%s\n", disasm.c_str()) ;
  }

  fclose(f) ;
  return 0 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
