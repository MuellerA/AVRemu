////////////////////////////////////////////////////////////////////////////////
// match.cpp
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

bool readFile(const char *filename, std::vector<AVR::Command> &commands)
{
  FILE *f = fopen(filename, "rb") ;
  if (!f)
  {
    fprintf(stderr, "read file \"%s\" failed\n", filename) ;
    return false ;
  }
  while (true)
  {
    AVR::Command cmds[0x100] ;
    uint32_t nCmd = fread(cmds, sizeof(AVR::Command), 0x100, f) ;
    if (!nCmd)
      break ;
    commands.insert(commands.end(), cmds, cmds + nCmd) ;
  }
  fclose(f) ;

  return true ;
}

////////////////////////////////////////////////////////////////////////////////

int compareCmd(std::vector<const AVR::Instruction*> &instructions, AVR::Command binCmd, AVR::Command matchCmd)
{
  const AVR::Instruction *binInst = instructions[binCmd] ;
  const AVR::Instruction *matchInst = instructions[matchCmd] ;

  if (!binInst && !matchInst)
    return 1 ;
      
  if (binInst != matchInst)
    return -1 ;

  // printf("c %s\n", binInst->Mnemonic().c_str()) ;
  
  if (binCmd == matchCmd)
    return binInst->Size() ;

  if ((binInst == &AVR::instrRJMP) ||
      (binInst == &AVR::instrRCALL))
  {
    return 1 ;
  }
  //else if ((binInst == &AVR::instrJMP) ||
  //         (binInst == &AVR::instrCALL))
  //{
  //  return 2 ;
  //}
  else if ((binInst == &AVR::instrBRBS) ||
           (binInst == &AVR::instrBRBC))
  {
    if ((binCmd & 0b1111110000000111) == (matchCmd & 0b1111110000000111))
      return 1 ;
  }
  else if (binInst == &AVR::instrLDI)
  {
    if ((binCmd & 0b1111000011110000) == (matchCmd & 0b1111000011110000))
      return 1 ;
  }
  //else if (binInst == &AVR::instrLDS)
  //{
  //  return 2 ;
  //}

  return -1 ;
}

////////////////////////////////////////////////////////////////////////////////

int compare(std::vector<const AVR::Instruction*> &instructions, std::vector<AVR::Command> bin, std::vector<AVR::Command> match)
{
  for (int iCmd0 = 0, eCmd0 = bin.size() - match.size() ; iCmd0 <= eCmd0 ; ++iCmd0)
  {
    // printf("a %d\n", iCmd0) ;
    int iCmd = 0, eCmd = match.size() ;
    while (iCmd < eCmd)
    {
      // printf("b %d\n", iCmd) ;
      AVR::Command binCmd = bin[iCmd0 + iCmd] ;
      AVR::Command matchCmd = match[iCmd] ;

      int inc = compareCmd(instructions, binCmd, matchCmd) ;
      if (inc <= 0)
        break ;

      iCmd += inc ;
    }

    if (iCmd == eCmd)
      return iCmd0 ;
  }

  return -1 ;
}

////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "usage: %s <avr-bin> <avr-match> ...\n", argv[0]) ;
    return 1 ;
  }

  const AVR::Instruction *instructions0[]
  {
    &AVR::instrADD, &AVR::instrADC, &AVR::instrADIW, &AVR::instrSUB, &AVR::instrSUBI, &AVR::instrSBC, &AVR::instrSBCI, &AVR::instrSBIW, &AVR::instrAND, &AVR::instrANDI,
    &AVR::instrOR, &AVR::instrORI, &AVR::instrEOR, &AVR::instrCOM, &AVR::instrNEG, &AVR::instrINC, &AVR::instrDEC, &AVR::instrMUL, &AVR::instrMULS, &AVR::instrMULSU,
    &AVR::instrFMUL, &AVR::instrFMULS, &AVR::instrFMULSU, &AVR::instrDES,

    &AVR::instrRJMP, &AVR::instrIJMP, &AVR::instrEIJMP, &AVR::instrJMP, &AVR::instrRCALL, &AVR::instrICALL, &AVR::instrEICALL, &AVR::instrCALL, &AVR::instrRET,
    &AVR::instrRETI, &AVR::instrCPSE, &AVR::instrCP, &AVR::instrCPC, &AVR::instrCPI, &AVR::instrSBRC, &AVR::instrSBRS, &AVR::instrSBIC, &AVR::instrSBIS,
    &AVR::instrBRBS, &AVR::instrBRBC,

    &AVR::instrMOV, &AVR::instrMOVW, &AVR::instrLDI, &AVR::instrLDS, &AVR::instrLDx1, &AVR::instrLDx2, &AVR::instrLDx3, &AVR::instrLDy1, &AVR::instrLDy2,
    &AVR::instrLDy3, &AVR::instrLDy4, &AVR::instrLDz1, &AVR::instrLDz2, &AVR::instrLDz3, &AVR::instrLDz4, &AVR::instrSTS, &AVR::instrSTx1, &AVR::instrSTx2,
    &AVR::instrSTx3, &AVR::instrSTy1, &AVR::instrSTy2, &AVR::instrSTy3, &AVR::instrSTy4, &AVR::instrSTz1, &AVR::instrSTz2, &AVR::instrSTz3, &AVR::instrSTz4,
    &AVR::instrLPM1, &AVR::instrLPM2, &AVR::instrLPM3, &AVR::instrELPM1, &AVR::instrELPM2, &AVR::instrELPM3, &AVR::instrSPM1, &AVR::instrSPM2, &AVR::instrIN,
    &AVR::instrOUT, &AVR::instrPUSH, &AVR::instrPOP, &AVR::instrXCH, &AVR::instrLAS, &AVR::instrLAC, &AVR::instrLAT,

    &AVR::instrLSR, &AVR::instrROR, &AVR::instrASR, &AVR::instrSWAP, &AVR::instrBSET, &AVR::instrBCLR, &AVR::instrSBI, &AVR::instrCBI, &AVR::instrBST,
    &AVR::instrBLD,

    &AVR::instrBREAK, &AVR::instrNOP, &AVR::instrSLEEP, &AVR::instrWDR,
  } ;

  std::vector<const AVR::Instruction*> instructions ;
  instructions.resize(0x10000) ;

  for (auto const *instr : instructions0)
  {
    for (uint32_t m = 0 ; m < 0x10000 ; ++m)
    {
      if (!(m & instr->Mask()))
      {
        AVR::Command cmd = instr->Pattern() | m ;
        instructions[cmd] = instr ;
      }
    }
  }

  std::vector<AVR::Command> bin ;
  if (!readFile(argv[1], bin))
    return 1 ;

  for (int iMatch = 2 ; iMatch < argc ; ++iMatch)
  {
    std::vector<AVR::Command> match ;
    if (!readFile(argv[iMatch], match))
      continue ;

    int pos = compare(instructions, bin, match) ;
    if (pos >= 0)
      fprintf(stdout, "%s(%05x) = %s\n", argv[1], pos, argv[iMatch]) ;
  }

  return 0 ;
}
