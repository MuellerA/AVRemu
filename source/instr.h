////////////////////////////////////////////////////////////////////////////////
// instr.h
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "avr.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // Instruction Classes
  ////////////////////////////////////////////////////////////////////////////////

#define INSTR(name)                                                          \
  class Instr##name : public Instruction                                     \
  {                                                                          \
  public:                                                                    \
    Instr##name() ;                                                          \
    virtual ~Instr##name() ;                                                 \
    virtual uint8       Ticks  (Mcu &mcu, Command cmd) const ;               \
    virtual void        Execute(Mcu &mcu, Command cmd) const ;               \
    virtual std::string Disasm (Mcu &mcu, Command cmd) const ;               \
    virtual XrefType    Xref   (Mcu &mcu, Command cmd, uint32 &addr) const ; \
  } ;                                                                        \
  extern Instr##name instr##name

  ////////////////////////////////////////////////////////////////////////////////
  // Arithmetic and Logic Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTR(ADD) ;
  INSTR(ADC) ;
  INSTR(ADIW) ;
  INSTR(SUB) ;
  INSTR(SUBI) ;
  INSTR(SBC) ;
  INSTR(SBCI) ;
  INSTR(SBIW) ;
  INSTR(AND) ;
  INSTR(ANDI) ;
  INSTR(OR) ;
  INSTR(ORI) ;
  INSTR(EOR) ;
  INSTR(COM) ;
  INSTR(NEG) ;
  //INSTR(SBR) ; => ORI r, i
  //INSTR(CBR) ; => ANDI r, i
  INSTR(INC) ;
  INSTR(DEC) ;
  //INSTR(TST) ; => AND r, r
  //INSTR(CLR) => EOR r, r
  //INSTR(SER) => LDI r, 0xff
  INSTR(MUL) ;
  INSTR(MULS) ;
  INSTR(MULSU) ;
  INSTR(FMUL) ;
  INSTR(FMULS) ;
  INSTR(FMULSU) ;
  INSTR(DES) ;

  ////////////////////////////////////////////////////////////////////////////////
  // Branch Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTR(RJMP) ;
  INSTR(IJMP) ;
  INSTR(EIJMP) ;
  INSTR(JMP) ;
  INSTR(RCALL) ;
  INSTR(ICALL) ;
  INSTR(EICALL) ;
  INSTR(CALL) ;
  INSTR(RET) ;
  INSTR(RETI) ;
  INSTR(CPSE) ;
  INSTR(CP) ;
  INSTR(CPC) ;
  INSTR(CPI) ;
  INSTR(SBRC) ;
  INSTR(SBRS) ;
  INSTR(SBIC) ;
  INSTR(SBIS) ;
  INSTR(BRBS) ;
  INSTR(BRBC) ;
  //INSTR(BREQ) ; => BRBS
  //INSTR(BRNE) ; => BRBC
  //INSTR(BRCS) ; => BRBS
  //INSTR(BRCC) ; => BRBC
  //INSTR(BRSH) ; => BRCC
  //INSTR(BRLO) ; => BRCS
  //INSTR(BRMI) ; => BRBS
  //INSTR(BRPL) ; => BRBC
  //INSTR(BRGE) ; => BRBS
  //INSTR(BRLT) ; => BRBC
  //INSTR(BRHS) ; => BRBS
  //INSTR(BRHC) ; => BRBC
  //INSTR(BRTS) ; => BRBS
  //INSTR(BRTC) ; => BRBC
  //INSTR(BRVS) ; => BRBS
  //INSTR(BRVC) ; => BRBC
  //INSTR(BRIE) ; => BRBS
  //INSTR(BRID) ; => BRBC

  ////////////////////////////////////////////////////////////////////////////////
  // Data Transfer Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTR(MOV) ;
  INSTR(MOVW) ;
  INSTR(LDI) ;
  INSTR(LDS) ;
  INSTR(LDx1) ;
  INSTR(LDx2) ;
  INSTR(LDx3) ;
  INSTR(LDy1) ;
  INSTR(LDy2) ;
  INSTR(LDy3) ;
  INSTR(LDy4) ;
  INSTR(LDz1) ;
  INSTR(LDz2) ;
  INSTR(LDz3) ;
  INSTR(LDz4) ;
  INSTR(STS) ;
  INSTR(STx1) ;
  INSTR(STx2) ;
  INSTR(STx3) ;
  INSTR(STy1) ;
  INSTR(STy2) ;
  INSTR(STy3) ;
  INSTR(STy4) ;
  INSTR(STz1) ;
  INSTR(STz2) ;
  INSTR(STz3) ;
  INSTR(STz4) ;
  INSTR(LPM1) ;
  INSTR(LPM2) ;
  INSTR(LPM3) ;
  INSTR(ELPM1) ;
  INSTR(ELPM2) ;
  INSTR(ELPM3) ;
  INSTR(SPM1) ;
  INSTR(SPM2) ;
  INSTR(IN) ;
  INSTR(OUT) ;
  INSTR(PUSH) ;
  INSTR(POP) ;
  INSTR(XCH) ;
  INSTR(LAS) ;
  INSTR(LAC) ;
  INSTR(LAT) ;

  ////////////////////////////////////////////////////////////////////////////////
  // Bit and Bit-test Instructions
  ////////////////////////////////////////////////////////////////////////////////

  //INSTR(LSL) ; => ADD r,r
  INSTR(LSR) ;
  //INSTR(ROL) ; => ADC r,r
  INSTR(ROR) ;
  INSTR(ASR) ;
  INSTR(SWAP) ;
  INSTR(BSET) ;
  INSTR(BCLR) ;
  INSTR(SBI) ;
  INSTR(CBI) ;
  INSTR(BST) ;
  INSTR(BLD) ;
  //INSTR(SEC) ; => BSET
  //INSTR(CLC) ; => BCLR
  //INSTR(SEN) ; => BSET
  //INSTR(CLN) ; => BCLR
  //INSTR(SEZ) ; => BSET
  //INSTR(CLZ) ; => BCLR
  //INSTR(SEI) ; => BSET
  //INSTR(CLI) ; => BCLR
  //INSTR(SES) ; => BSET
  //INSTR(CLS) ; => BCLR
  //INSTR(SEV) ; => BSET
  //INSTR(CLV) ; => BCLR
  //INSTR(SET) ; => BSET
  //INSTR(CLT) ; => BCLR
  //INSTR(SEH) ; => BSET
  //INSTR(CLH) ; => BCLR

  ////////////////////////////////////////////////////////////////////////////////
  // MCU Control Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTR(BREAK) ;
  INSTR(NOP) ;
  INSTR(SLEEP) ;
  INSTR(WDR) ;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
