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
    for (unsigned int iCmd = 0 ; iCmd < 0x10000 ; ++iCmd)
    {
      prog.push_back(iCmd) ;

      if (((iCmd & AVR::instrJMP .Mask()) == AVR::instrJMP .Pattern()) ||
          ((iCmd & AVR::instrCALL.Mask()) == AVR::instrCALL.Pattern()) ||
          ((iCmd & AVR::instrLDS .Mask()) == AVR::instrLDS .Pattern()) ||
          ((iCmd & AVR::instrSTS .Mask()) == AVR::instrSTS .Pattern()))
        prog.push_back(0x8888) ;
    }

    avr.PC() = 0 ;
    size_t nCommand = avr.SetProgram(0, prog) ;
    printf("prog size: %ld\n", prog.size()) ;

   FILE *fo = fopen("all.bin", "wb") ;
    if (fo)
    {
      fwrite(prog.data(), prog.size(), sizeof(AVR::Command), fo) ;
      fclose(fo) ;
    }

    while (avr.PC() < nCommand)
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
