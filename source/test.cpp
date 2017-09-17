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

int main()
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
// EOF
////////////////////////////////////////////////////////////////////////////////
