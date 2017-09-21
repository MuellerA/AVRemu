////////////////////////////////////////////////////////////////////////////////
// instr.cpp
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <algorithm>

#include "instr.h"

namespace AVR
{
  ////////////////////////////////////////////////////////////////////////////////
  // Helper

  inline void xxxxxxxRRRRRxxxx(Command cmd, Command &r)
  {
    r = (cmd & 0b0000000111110000) >> 4 ;
  }
  inline void xxxxxxRxxxxxRRRR(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000001000000000) >> 5) | (cmd & 0b0000000000001111) ;
  }
  inline void xxxxxxxxKKxxKKKK(Command cmd, Command &k)
  {
    k = ((cmd & 0b0000000011000000) >> 2) | (cmd & 0b0000000000001111) ;
  }
  inline void xxxxxxxxxxRRxxxx(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000000000110000) >> 3) + 24 ;
  }
  inline void xxxxKKKKxxxxKKKK(Command cmd, Command &k)
  {
    k = ((cmd & 0b0000111100000000) >> 4) | (cmd & 0b0000000000001111) ;
  }
  inline void xxxxxxxxRRRRxxxx1(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000000011110000) >> 4) + 16 ;
  }
  inline void xxxxxxxxRRRRxxxx2(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000000011110000) >> 3) ; ;
  }
  inline void xxxxxxxxxxxxRRRR1(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000000000001111)) + 16 ;
  }
  inline void xxxxxxxxxxxxRRRR2(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000000000001111) << 1) ;
  }
  inline void xxxxxxxxxRRRxxxx(Command cmd, Command &r)
  {
    r = ((cmd & 0b0000000001110000) >> 4) + 16 ;
  }
  inline void xxxxxxxxxxxxxRRR(Command cmd, Command &r)
  {
    r = (cmd & 0b0000000000000111) + 16 ;
  }
  inline void xxxxxxxxKKKKxxxx(Command cmd, Command &k)
  {
    k = (cmd & 0b0000000011110000) >> 4 ;
  }
  inline void xxxxKKKKKKKKKKKK(Command cmd, Command &k)
  {
    k = (cmd & 0b0000111111111111) ;
    if (   k & 0b0000100000000000) // negative
      k    |= 0b1111000000000000 ;
  }
  inline void xxxxxxxKKKKKxxxK(Command cmd, Command &k)
  {
    k = ((cmd & 0b0000000111110000) >> 3) | (cmd & 0b0000000000000001) ;
  }
  inline void xxxxxxxxxxxxxBBB(Command cmd, Command &b)
  {
    b = (cmd & 0b0000000000000111) ;
  }
  inline void xxxxxxxxAAAAAxxx(Command cmd, Command &a)
  {
    a = (cmd & 0b0000000011111000) >> 3 ;
  }
  inline void xxxxxxKKKKKKKxxx(Command cmd, Command &k)
  {
    k = (cmd & 0b0000001111111000) >> 3 ;
    if (k & 0b0000000001000000) // negative
      k |= 0b1111111110000000 ;
  }
  inline void xxxxxxxxxxxxxSSS(Command cmd, Command &s)
  {
    s = (cmd & 0b0000000000000111) ;
  }
  inline void xxxxxxxxxSSSxxxx(Command cmd, Command &s)
  {
    s = (cmd & 0b0000000001110000) >> 4 ;
  }
  inline void xxxxxAAxxxxxAAAA(Command cmd, Command &a)
  {
    a = ((cmd & 0b0000011000000000) >> 5) | (cmd & 0b0000000000001111) ;
  }
  inline void xxQxQQxxxxxxxQQQ(Command cmd, Command &q)
  {
    q = ((cmd & 0b0010000000000000) >> 8) | ((cmd & 0b0000110000000000) >> 7) | ((cmd & 0b0000000000000111) >> 0) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  
  inline char sToFlag(Command s)
  {
    static char flags[] = "CZNVSHTI" ;
    return flags[s] ;
  }
  inline const char* sToBSET(Command s)
  {
    static const char *mnemonic[] = { "SEC", "SEZ", "SEN", "SEV", "SES", "SEH", "SET", "SEI" } ;
    return mnemonic[s] ;
  }
  inline const char* sToBCLR(Command s)
  {
    static const char *mnemonic[] = { "CLC", "CLZ", "CLN", "CLV", "CLS", "CLH", "CLT", "CLI" } ;
    return mnemonic[s] ;
  }
  inline const char* sToBRBS(Command s)
  {
    static const char *mnemonic[] = { "BRCS", "BREQ", "BRMI", "BRVS", "BRLT", "BRHS", "BRTS", "BRIE" } ;
    return mnemonic[s] ;
  }
  inline const char* sToBRBC(Command s)
  {
    static const char *mnemonic[] = { "BRCC", "BRNE", "BRPL", "BRVC", "BRGE", "BRHC", "BRTC", "BRID" } ;
    return mnemonic[s] ;
  }

  
  ////////////////////////////////////////////////////////////////////////////////
  
  std::string Disasm_xxxxxxxxxxxxxxxx(const Instruction &instr)
  {
    char buff[1024] ;
    sprintf(buff, "%-6s\t\t; %s", instr.Mnemonic().c_str(), instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxRDDDDDRRRR(const Instruction &instr, Command cmd)
  {
    Command r, d ;
    xxxxxxRxxxxxRRRR(cmd, r) ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, r%d\t\t; %s", instr.Mnemonic().c_str(), d, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxKKDDKKKK(const Instruction &instr, Command cmd)
  {
    Command k, d ;
    xxxxxxxxKKxxKKKK(cmd, k) ;
    xxxxxxxxxxRRxxxx(cmd, d) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, 0x%02x\t\t; %d %s", instr.Mnemonic().c_str(), d, k, k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxKKKKDDDDKKKK(const Instruction &instr, Command cmd)
  {
    Command k, d ;
    xxxxKKKKxxxxKKKK(cmd, k) ;
    xxxxxxxxRRRRxxxx1(cmd, d) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, 0x%02x\t\t; %d %s", instr.Mnemonic().c_str(), d, k, k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxDDDDDxxxx(const Instruction &instr, Command cmd)
  {
    Command d ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d\t\t; %s", instr.Mnemonic().c_str(), d, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxDDDDRRRR_MOVW(const Instruction &instr, Command cmd)
  {
    Command r, d ;
    xxxxxxxxRRRRxxxx2(cmd, d) ;
    xxxxxxxxxxxxRRRR2(cmd, r) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, r%d\t\t; %s", instr.Mnemonic().c_str(), d, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxDDDDRRRR_MULS(const Instruction &instr, Command cmd)
  {
    Command r, d ;
    xxxxxxxxRRRRxxxx1(cmd, d) ;
    xxxxxxxxxxxxRRRR1(cmd, r) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, r%d\t\t; %s", instr.Mnemonic().c_str(), d, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxxDDDxRRR(const Instruction &instr, Command cmd)
  {
    Command r, d ;
    xxxxxxxxxRRRxxxx(cmd, d) ;
    xxxxxxxxxxxxxRRR(cmd, r) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, r%d\t\t; %s", instr.Mnemonic().c_str(), d, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxKKKKxxxx(const Instruction &instr, Command cmd)
  {
    Command k ;
    xxxxxxxxKKKKxxxx(cmd, k) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %d\t\t; %s", instr.Mnemonic().c_str(), k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxKKKKKKKKKKKK(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command k ;
    xxxxKKKKKKKKKKKK(cmd, k) ;
    uint32 addr = (uint32)(mcu.PC()) + (int16)k ;
    char buff[1024] ;
    std::string label ;
    if (mcu.ProgAddrName(addr, label))
      sprintf(buff, "%-6s %s\t\t; %d 0x%05x %s", instr.Mnemonic().c_str(), label.c_str(), (int16)k, addr, instr.Description().c_str()) ;
    else
      sprintf(buff, "%-6s %d\t\t; 0x%05x %s", instr.Mnemonic().c_str(), (int16)k, addr, instr.Description().c_str()) ;      
    return std::string(buff) ;
  }
  void Xref_xxxxKKKKKKKKKKKK(const Instruction &instr, const Mcu &mcu, Command cmd, uint32 &addr)
  {
    Command k ;
    xxxxKKKKKKKKKKKK(cmd, k) ;
    addr = (uint32)(mcu.PC()) + (int16)k ;
  }

  std::string Disasm_xxxxxxxKKKKKxxxKk16(const Instruction &instr, Mcu &mcu, Command cmd)
  {
    Command k ;
    xxxxxxxKKKKKxxxK(cmd, k) ;
    uint32 addr = (((uint32)k) << 16) + mcu.ProgramNext() ;
    char buff[1024] ;
    std::string label ;
    if (mcu.ProgAddrName(addr, label))
      sprintf(buff, "%-6s %s\t\t; 0x%05x %s", instr.Mnemonic().c_str(), label.c_str(), addr, instr.Description().c_str()) ;
    else
      sprintf(buff, "%-6s 0x%05x\t\t; %s", instr.Mnemonic().c_str(), addr, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  void Xref_xxxxxxxKKKKKxxxKk16(const Instruction &instr, Mcu &mcu, Command cmd, uint32 &addr)
  {
    Command k ;
    xxxxxxxKKKKKxxxK(cmd, k) ;
    addr = (((uint32)k) << 16) + mcu.ProgramNext() ;
  }
  
  std::string Disasm_xxxxxxxRRRRRxBBB(const Instruction &instr, Command cmd)
  {
    Command r, b ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    xxxxxxxxxxxxxBBB(cmd, b) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, %d\t\t; %s", instr.Mnemonic().c_str(), r, b, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxAAAAABBB(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command a, b ;
    xxxxxxxxAAAAAxxx(cmd, a) ;
    xxxxxxxxxxxxxBBB(cmd, b) ;
    std::string ioRegName ;
    mcu.DataAddrName(a+0x20, ioRegName) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %s, %d\t\t; 0x%02x %s", instr.Mnemonic().c_str(), ioRegName.c_str(), b, a, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxKKKKKKKSSS_BRBS(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command k, s ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    xxxxxxxxxxxxxSSS(cmd, s) ;
    uint32 addr = (uint32)(mcu.PC()) + (int16)k ;
    char buff[1024] ;
    std::string label ;
    if (mcu.ProgAddrName(addr, label))
      sprintf(buff, "%-6s %s\t\t; %d 0x%05x %s", sToBRBS(s), label.c_str(), (int16)k, addr, instr.Description().c_str()) ;
    else
      sprintf(buff, "%-6s %d\t\t; 0x%05x %s", sToBRBS(s), (int16)k, addr, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxxKKKKKKKSSS_BRBC(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command k, s ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    xxxxxxxxxxxxxSSS(cmd, s) ;
    uint32 addr = (uint32)(mcu.PC()) + (int16)k ;
    char buff[1024] ;
    std::string label ;
    if (mcu.ProgAddrName(addr, label))
      sprintf(buff, "%-6s %s\t\t; %d 0x%05x %s", sToBRBC(s), label.c_str(), (int16)k, addr, instr.Description().c_str()) ;
    else
      sprintf(buff, "%-6s %d\t\t; 0x%05x %s", sToBRBC(s), (int16)k, addr, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  void Xref_xxxxxxKKKKKKKxxx(const Instruction &instr, const Mcu &mcu, Command cmd, uint32 &addr)
  {
    Command k ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    addr = (uint32)(mcu.PC()) + (int16)k ;
  }
  
  std::string Disasm_xxxxxxKKKKKKKxxx(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command k ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %d\t\t; 0x%05x %s", instr.Mnemonic().c_str(), (int16)k, (uint32)(mcu.PC()) + (int16)k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxxxSSSxxxx_BSET(const Instruction &instr, Command cmd)
  {
    Command s ;
    xxxxxxxxxSSSxxxx(cmd, s) ;
    char buff[1024] ;
    sprintf(buff, "%-6s\t\t; %s", sToBSET(s), instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxxxxxSSSxxxx_BCLR(const Instruction &instr, Command cmd)
  {
    Command s ;
    xxxxxxxxxSSSxxxx(cmd, s) ;
    char buff[1024] ;
    sprintf(buff, "%-6s\t\t; %s", sToBCLR(s), instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxDDDDDxBBB(const Instruction &instr, Command cmd)
  {
    Command d, b ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    xxxxxxxxxxxxxBBB(cmd, b) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, %d\t\t; %s", instr.Mnemonic().c_str(), d, b, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxDDDDDxxxxk16(const Instruction &instr, Mcu &mcu, Command cmd)
  {
    Command d ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    uint32 addr = mcu.ProgramNext() ;
    char buff[1024] ;
    std::string ioRegName ;
    if (mcu.DataAddrName(addr, ioRegName))
      sprintf(buff, "%-6s r%d, %s\t\t; 0x%04x %s", instr.Mnemonic().c_str(), d, ioRegName.c_str(), addr, instr.Description().c_str()) ;
    else
      sprintf(buff, "%-6s r%d, 0x%04x\t\t; %s", instr.Mnemonic().c_str(), d, addr, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxxxRRRRRxxxxk16(const Instruction &instr, Mcu &mcu, Command cmd)
  {
    Command r ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    uint32 addr = mcu.ProgramNext() ;
    char buff[1024] ;
    std::string ioRegName ;
    if (mcu.DataAddrName(addr, ioRegName))
      sprintf(buff, "%-6s %s, r%d\t\t; 0x%04x %s", instr.Mnemonic().c_str(), ioRegName.c_str(), r, addr, instr.Description().c_str()) ;
    else
      sprintf(buff, "%-6s 0x%04x, r%d\t\t; %s", instr.Mnemonic().c_str(), addr, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxAADDDDDAAAA(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command d, a ;
    xxxxxAAxxxxxAAAA(cmd, a) ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    char buff[1024] ;
    std::string ioRegName  ;
    mcu.DataAddrName(a+0x20, ioRegName) ;
    sprintf(buff, "%-6s r%d, %s\t\t; 0x%02x %s", instr.Mnemonic().c_str(), d, ioRegName.c_str(), a, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxAARRRRRAAAA(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command r, a ;
    xxxxxAAxxxxxAAAA(cmd, a) ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    char buff[1024] ;
    std::string ioRegName ;
    mcu.DataAddrName(a+0x20, ioRegName) ;
    sprintf(buff, "%-6s %s, r%d\t\t; 0x%02x %s", instr.Mnemonic().c_str(), ioRegName.c_str(), r, a, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxxDDDDDxxxx(const Instruction &instr, const Mcu &mcu, Command cmd, const char *offset)
  {
    Command d ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, %s\t\t; %s", instr.Mnemonic().c_str(), d, offset, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxxxRRRRRxxxx(const Instruction &instr, const Mcu &mcu, Command cmd, const char *offset)
  {
    Command r ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %s, r%d\t\t; %s", instr.Mnemonic().c_str(), offset, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string  Disasm_xxQxQQxDDDDDxQQQ(const Instruction &instr, const Mcu &mcu, Command cmd, const char *offset)
  {
    Command d, q ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    xxQxQQxxxxxxxQQQ(cmd, q) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, %s+%d\t\t; %s", instr.Mnemonic().c_str(), d, offset, q, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string  Disasm_xxQxQQxRRRRRxQQQ(const Instruction &instr, const Mcu &mcu, Command cmd, const char *offset)
  {
    Command r, q ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    xxQxQQxxxxxxxQQQ(cmd, q) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %s+%d, r%d\t\t; %s", instr.Mnemonic().c_str(), offset, q, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ADD
  InstrADD::InstrADD() : Instruction(0b0000110000000000, 0b1111110000000000, "ADD", "Add without Carry")
  {
  }

  InstrADD::~InstrADD()
  {
  }

  uint8 InstrADD::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrADD::Execute(Mcu &mcu, Command cmd) const
  {
    Command nr, nd ;
    xxxxxxRxxxxxRRRR(cmd, nr) ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11000000 ;
    uint8 rr = mcu.Reg(nr) ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = rd + rr ;
    uint8 rHC = rd & rr | rr & ~r | ~r & rd ;
    uint8 rV  = rd & rr & ~r | ~rd & ~rr & r ;
    if (rHC & (1<<3))
      sreg |= SREG::H ;
    if (rV & (1<<7))
      sreg |= SREG::V ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    if (rHC & (1<<7))
      sreg |= SREG::C ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrADD::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrADD::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ADC
  InstrADC::InstrADC() : Instruction(0b0001110000000000, 0b1111110000000000, "ADC", "Add with Carry")
  {
  }

  InstrADC::~InstrADC()
  {
  }

  uint8 InstrADC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrADC::Execute(Mcu &mcu, Command cmd) const
  {
    Command nr, nd ;
    xxxxxxRxxxxxRRRR(cmd, nr) ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11000000 ;
    uint8 rr = mcu.Reg(nr) ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = rd + rr + (sreg & (uint8)SREG::C) ;
    uint8 rHC = rd & rr | rr & ~r | ~r & rd ;
    uint8 rV  = rd & rr & ~r | ~rd & ~rr & r ;
    if (rHC & (1<<3))
      sreg |= SREG::H ;
    if (rV & (1<<7))
      sreg |= SREG::V ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    if (rHC & (1<<7))
      sreg |= SREG::C ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrADC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrADC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ADIW
  InstrADIW::InstrADIW() : Instruction(0b1001011000000000, 0b1111111100000000, "ADIW", "Add Immediate to Word")
  {
  }

  InstrADIW::~InstrADIW()
  {
  }

  uint8 InstrADIW::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrADIW::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrADIW::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxKKDDKKKK(*this, cmd) ;
  }
  XrefType InstrADIW::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SUB
  InstrSUB::InstrSUB() : Instruction(0b0001100000000000, 0b1111110000000000, "SUB", "Subtract without Carry")
  {
  }

  InstrSUB::~InstrSUB()
  {
  }

  uint8 InstrSUB::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrSUB::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSUB::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrSUB::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SUBI
  InstrSUBI::InstrSUBI() : Instruction(0b0101000000000000, 0b1111000000000000, "SUBI", "Subtract Immediate")
  {
  }

  InstrSUBI::~InstrSUBI()
  {
  }

  uint8 InstrSUBI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrSUBI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSUBI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  XrefType InstrSUBI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBC
  InstrSBC::InstrSBC() : Instruction(0b0000100000000000, 0b1111110000000000, "SBC", "Subtract with Carry")
  {
  }

  InstrSBC::~InstrSBC()
  {
  }

  uint8 InstrSBC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrSBC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrSBC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBCI
  InstrSBCI::InstrSBCI() : Instruction(0b0100000000000000, 0b1111000000000000, "SBCI", "Subtract Immediate with Carry")
  {
  }

  InstrSBCI::~InstrSBCI()
  {
  }

  uint8 InstrSBCI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrSBCI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBCI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  XrefType InstrSBCI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBIW
  InstrSBIW::InstrSBIW() : Instruction(0b1001011100000000, 0b1111111100000000, "SBIW", "Subtract Immediate from Word")
  {
  }

  InstrSBIW::~InstrSBIW()
  {
  }

  uint8 InstrSBIW::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrSBIW::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBIW::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxKKDDKKKK(*this, cmd) ;
  }
  XrefType InstrSBIW::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // AND
  InstrAND::InstrAND() : Instruction(0b0010000000000000, 0b1111110000000000, "AND", "Logical AND")
  {
  }

  InstrAND::~InstrAND()
  {
  }

  uint8 InstrAND::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrAND::Execute(Mcu &mcu, Command cmd) const
  {
    Command nr, nd ;
    xxxxxxRxxxxxRRRR(cmd, nr) ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11100001 ;
    uint8 rr = mcu.Reg(nr) ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = rd & rr ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrAND::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrAND::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ANDI
  InstrANDI::InstrANDI() : Instruction(0b0111000000000000, 0b1111000000000000, "ANDI", "Logical AND with Immediate")
  {
  }

  InstrANDI::~InstrANDI()
  {
  }

  uint8 InstrANDI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrANDI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrANDI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  XrefType InstrANDI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // OR
  InstrOR::InstrOR() : Instruction(0b0010100000000000, 0b1111110000000000, "OR", "Logical OR")
  {
  }

  InstrOR::~InstrOR()
  {
  }

  uint8 InstrOR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrOR::Execute(Mcu &mcu, Command cmd) const
  {
    Command nr, nd ;
    xxxxxxRxxxxxRRRR(cmd, nr) ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11100001 ;
    uint8 rr = mcu.Reg(nr) ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = rd | rr ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrOR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrOR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ORI
  InstrORI::InstrORI() : Instruction(0b0110000000000000, 0b1111000000000000, "ORI", "Logical OR with Immediate")
  {
  }

  InstrORI::~InstrORI()
  {
  }

  uint8 InstrORI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrORI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrORI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  XrefType InstrORI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // EOR
  InstrEOR::InstrEOR() : Instruction(0b0010010000000000, 0b1111110000000000, "EOR", "Exclusive OR")
  {
  }

  InstrEOR::~InstrEOR()
  {
  }

  uint8 InstrEOR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrEOR::Execute(Mcu &mcu, Command cmd) const
  {
    Command nr, nd ;
    xxxxxxRxxxxxRRRR(cmd, nr) ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11100001 ;
    uint8 rr = mcu.Reg(nr) ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = rd | rr ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrEOR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrEOR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // COM
  InstrCOM::InstrCOM() : Instruction(0b1001010000000000, 0b1111111000001111, "COM", "One's Complement")
  {
  }

  InstrCOM::~InstrCOM()
  {
  }

  uint8 InstrCOM::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrCOM::Execute(Mcu &mcu, Command cmd) const
  {
    Command nd ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11100000 ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = 0xff - rd ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    sreg |= SREG::C ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrCOM::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrCOM::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // NEG
  InstrNEG::InstrNEG() : Instruction(0b1001010000000001, 0b1111111000001111, "NEG", "Two's Complement")
  {
  }

  InstrNEG::~InstrNEG()
  {
  }

  uint8 InstrNEG::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrNEG::Execute(Mcu &mcu, Command cmd) const
  {
    Command nd ;
    xxxxxxxRRRRRxxxx(cmd, nd) ;

    uint8 sreg = mcu.SREG() & 0b11000000 ;
    uint8 rd = mcu.Reg(nd) ;
    uint8 r = 0 - rd ;
    uint8 rH = r | ~rd ;
    if (rH & (1<<3))
      sreg |= SREG::H ;
    if (r == 0x80)
      sreg |= SREG::V ;
    if (r & 0x80)
      sreg |= SREG::N ;
    if (r == 0x00)
      sreg |= SREG::Z ;
    if (r != 0x00)
      sreg |= SREG::C ;
    if ((sreg & SREG::N) ^ (sreg & SREG::V))
      sreg |= SREG::S ;
    mcu.Reg(nd, r) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrNEG::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrNEG::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // INC
  InstrINC::InstrINC() : Instruction(0b1001010000000011, 0b1111111000001111, "INC", "Increment")
  {
  }

  InstrINC::~InstrINC()
  {
  }

  uint8 InstrINC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrINC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrINC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrINC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // DEC
  InstrDEC::InstrDEC() : Instruction(0b1001010000001010, 0b1111111000001111, "DEC", "Decrement")
  {
  }

  InstrDEC::~InstrDEC()
  {
  }

  uint8 InstrDEC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrDEC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrDEC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrDEC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MUL
  InstrMUL::InstrMUL() : Instruction(0b1001110000000000, 0b1111110000000000, "MUL", "Multiply Unsigned")
  {
  }

  InstrMUL::~InstrMUL()
  {
  }

  uint8 InstrMUL::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrMUL::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrMUL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrMUL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MULS
  InstrMULS::InstrMULS() : Instruction(0b0000001000000000, 0b1111111100000000, "MULS", "Multiply Signed")
  {
  }

  InstrMULS::~InstrMULS()
  {
  }

  uint8 InstrMULS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrMULS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrMULS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxDDDDRRRR_MULS(*this, cmd) ;
  }
  XrefType InstrMULS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MULSU
  InstrMULSU::InstrMULSU() : Instruction(0b0000001100000000, 0b1111111110001000, "MULSU", "Multiply Signed with Unsigned")
  {
  }

  InstrMULSU::~InstrMULSU()
  {
  }

  uint8 InstrMULSU::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrMULSU::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrMULSU::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  XrefType InstrMULSU::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // FMUL
  InstrFMUL::InstrFMUL() : Instruction(0b0000001100001000, 0b1111111110001000, "FMUL", "Fractional Multiply Unsigned")
  {
  }

  InstrFMUL::~InstrFMUL()
  {
  }

  uint8 InstrFMUL::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrFMUL::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrFMUL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  XrefType InstrFMUL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // FMULS
  InstrFMULS::InstrFMULS() : Instruction(0b0000001110000000, 0b1111111110001000, "FMULS", "Fractional Multiply Signed")
  {
  }

  InstrFMULS::~InstrFMULS()
  {
  }

  uint8 InstrFMULS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrFMULS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrFMULS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  XrefType InstrFMULS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // FMULSU
  InstrFMULSU::InstrFMULSU() : Instruction(0b0000001110001000, 0b1111111110001000, "FMULSU", "Fractional Multiply Signed with Unsigned")
  {
  }

  InstrFMULSU::~InstrFMULSU()
  {
  }

  uint8 InstrFMULSU::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrFMULSU::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrFMULSU::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  XrefType InstrFMULSU::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // DES
  InstrDES::InstrDES() : Instruction(0b1001010000001011, 0b1111111100001111, "DES", "Data Encryption Standard")
  {
  }

  InstrDES::~InstrDES()
  {
  }

  uint8 InstrDES::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrDES::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrDES::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxKKKKxxxx(*this, cmd) ;
  }
  XrefType InstrDES::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RJMP
  InstrRJMP::InstrRJMP() : Instruction(0b1100000000000000, 0b1111000000000000, "RJMP", "Relative Jump")
  {
  }

  InstrRJMP::~InstrRJMP()
  {
  }

  uint8 InstrRJMP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrRJMP::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrRJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKKKKKKKKK(*this, mcu, cmd) ;
  }
  XrefType InstrRJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    Xref_xxxxKKKKKKKKKKKK(*this, mcu, cmd, addr) ;
    return XrefType::jmp ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // IJMP
  InstrIJMP::InstrIJMP() : Instruction(0b1001010000001001, 0b1111111111111111, "IJMP", "Indirect Jump")
  {
  }

  InstrIJMP::~InstrIJMP()
  {
  }

  uint8 InstrIJMP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrIJMP::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrIJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrIJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // EIJMP
  InstrEIJMP::InstrEIJMP() : Instruction(0b1001010000011001, 0b1111111111111111, "EIJMP", "Extended Indirect Jump")
  {
  }

  InstrEIJMP::~InstrEIJMP()
  {
  }

  uint8 InstrEIJMP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ;
  }
  void InstrEIJMP::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrEIJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrEIJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // JMP
  InstrJMP::InstrJMP() : Instruction(0b1001010000001100, 0b1111111000001110, "JMP", "Jump")
  {
  }

  InstrJMP::~InstrJMP()
  {
  }

  uint8 InstrJMP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrJMP::Execute(Mcu &mcu, Command cmd) const
  {
    Command k ;
    xxxxxxxKKKKKxxxK(cmd, k) ;
    mcu.PC() = (((uint32)k) << 16) + mcu.ProgramNext() ;
  }
  std::string InstrJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd) ;
  }
  XrefType InstrJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    Xref_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd, addr) ;
    return XrefType::jmp ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RCALL
  InstrRCALL::InstrRCALL() : Instruction(0b1101000000000000, 0b1111000000000000, "RCALL", "Relative Call to Subroutine")
  {
  }

  InstrRCALL::~InstrRCALL()
  {
  }

  uint8 InstrRCALL::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ; // todo
  }
  void InstrRCALL::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrRCALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKKKKKKKKK(*this, mcu, cmd) ;
  }
  XrefType InstrRCALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    Xref_xxxxKKKKKKKKKKKK(*this, mcu, cmd, addr) ;
    return XrefType::call ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ICALL
  InstrICALL::InstrICALL() : Instruction(0b1001010100001001, 0b1111111111111111, "ICALL", "Indirect Call to Subroutine")
  {
  }

  InstrICALL::~InstrICALL()
  {
  }

  uint8 InstrICALL::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ; // todo
  }
  void InstrICALL::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrICALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrICALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // EICALL
  InstrEICALL::InstrEICALL() : Instruction(0b1001010100011001, 0b1111111111111111, "EICALL", "Extended Indirect Call to Subroutine")
  {
  }

  InstrEICALL::~InstrEICALL()
  {
  }

  uint8 InstrEICALL::Ticks(Mcu &mcu, Command cmd) const
  {
    return 4 ; // todo
  }
  void InstrEICALL::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrEICALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrEICALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CALL
  InstrCALL::InstrCALL() : Instruction(0b1001010000001110, 0b1111111000001110, "CALL", "Call to Subroutine")
  {
  }

  InstrCALL::~InstrCALL()
  {
  }

  uint8 InstrCALL::Ticks(Mcu &mcu, Command cmd) const
  {
    return 4 ; // todo
  }
  void InstrCALL::Execute(Mcu &mcu, Command cmd) const
  {
    mcu.PushPC() ;
    Command k ;
    xxxxxxxKKKKKxxxK(cmd, k) ;
    mcu.PC() = (((uint32)k) << 16) + mcu.ProgramNext() ;
  }
  std::string InstrCALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd) ;
  }
  XrefType InstrCALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    Xref_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd, addr) ;
    return XrefType::call ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RET
  InstrRET::InstrRET() : Instruction(0b1001010100001000, 0b1111111111111111, "RET", "Return from Subroutine")
  {
  }

  InstrRET::~InstrRET()
  {
  }

  uint8 InstrRET::Ticks(Mcu &mcu, Command cmd) const
  {
    return 4 ; // todo
  }
  void InstrRET::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrRET::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrRET::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RETI
  InstrRETI::InstrRETI() : Instruction(0b1001010100011000, 0b1111111111111111, "RETI", "Return from Interrupt")
  {
  }

  InstrRETI::~InstrRETI()
  {
  }

  uint8 InstrRETI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 4 ; // todo
  }
  void InstrRETI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrRETI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrRETI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CPSE
  InstrCPSE::InstrCPSE() : Instruction(0b0001000000000000, 0b1111110000000000, "CPSE", "Compare Skip if Equal")
  {
  }

  InstrCPSE::~InstrCPSE()
  {
  }

  uint8 InstrCPSE::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrCPSE::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrCPSE::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrCPSE::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CP
  InstrCP::InstrCP() : Instruction(0b0001010000000000, 0b1111110000000000, "CP", "Compare")
  {
  }

  InstrCP::~InstrCP()
  {
  }

  uint8 InstrCP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrCP::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrCP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrCP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CPC
  InstrCPC::InstrCPC() : Instruction(0b0000010000000000, 0b1111110000000000, "CPC", "Compare with Carry")
  {
  }

  InstrCPC::~InstrCPC()
  {
  }

  uint8 InstrCPC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrCPC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrCPC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrCPC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CPI
  InstrCPI::InstrCPI() : Instruction(0b0011000000000000, 0b1111000000000000, "CPI", "Compare with Immediate")
  {
  }

  InstrCPI::~InstrCPI()
  {
  }

  uint8 InstrCPI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrCPI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrCPI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  XrefType InstrCPI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBRC
  InstrSBRC::InstrSBRC() : Instruction(0b1111110000000000, 0b1111111000001000, "SBRC", "Skip if Bit in Register is Cleared")
  {
  }

  InstrSBRC::~InstrSBRC()
  {
  }

  uint8 InstrSBRC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrSBRC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBRC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxBBB(*this, cmd) ;
  }
  XrefType InstrSBRC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBRS
  InstrSBRS::InstrSBRS() : Instruction(0b1111111000000000, 0b1111111000001000, "SBRS", "Skip if Bit in Register is Set")
  {
  }

  InstrSBRS::~InstrSBRS()
  {
  }

  uint8 InstrSBRS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrSBRS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBRS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxBBB(*this, cmd) ;
  }
  XrefType InstrSBRS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBIC
  InstrSBIC::InstrSBIC() : Instruction(0b1001100100000000, 0b1111111100000000, "SBIC", "Skip if Bit in I/O Register is Cleared")
  {
  }

  InstrSBIC::~InstrSBIC()
  {
  }

  uint8 InstrSBIC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrSBIC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBIC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  XrefType InstrSBIC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBIS
  InstrSBIS::InstrSBIS() : Instruction(0b1001101100000000, 0b1111111100000000, "SBIS", "Skip if Bit in I/O Registerster is Set")
  {
  }

  InstrSBIS::~InstrSBIS()
  {
  }

  uint8 InstrSBIS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrSBIS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBIS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  XrefType InstrSBIS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BRBS
  InstrBRBS::InstrBRBS() : Instruction(0b1111000000000000, 0b1111110000000000, "BRBS", "Branch if Bit in SREG is Set")
  {
  }

  InstrBRBS::~InstrBRBS()
  {
  }

  uint8 InstrBRBS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrBRBS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrBRBS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxKKKKKKKSSS_BRBS(*this, mcu, cmd) ;
  }
  XrefType InstrBRBS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    Xref_xxxxxxKKKKKKKxxx(*this, mcu, cmd, addr) ;
    return XrefType::jmp ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BRBC
  InstrBRBC::InstrBRBC() : Instruction(0b1111010000000000, 0b1111110000000000, "BRBC", "Branch if Bit in SREG is Cleared")
  {
  }

  InstrBRBC::~InstrBRBC()
  {
  }

  uint8 InstrBRBC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrBRBC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrBRBC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxKKKKKKKSSS_BRBC(*this, mcu, cmd) ;
  }
  XrefType InstrBRBC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    Xref_xxxxxxKKKKKKKxxx(*this, mcu, cmd, addr) ;
    return XrefType::jmp ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MOV
  InstrMOV::InstrMOV() : Instruction(0b0010110000000000, 0b1111110000000000, "MOV", "Copy Register")
  {
  }

  InstrMOV::~InstrMOV()
  {
  }

  uint8 InstrMOV::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrMOV::Execute(Mcu &mcu, Command cmd) const
  {
    Command r, d ;
    xxxxxxRxxxxxRRRR(cmd, r) ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    mcu.Reg(d, mcu.Reg(r)) ;
  }
  std::string InstrMOV::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  XrefType InstrMOV::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MOVW
  InstrMOVW::InstrMOVW() : Instruction(0b0000000100000000, 0b1111111100000000, "MOVW", "Copy Register Word")
  {
  }

  InstrMOVW::~InstrMOVW()
  {
  }

  uint8 InstrMOVW::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrMOVW::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrMOVW::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxDDDDRRRR_MOVW(*this, cmd) ;
  }
  XrefType InstrMOVW::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDI
  InstrLDI::InstrLDI() : Instruction(0b1110000000000000, 0b1111000000000000, "LDI", "Load Immediate")
  {
  }

  InstrLDI::~InstrLDI()
  {
  }

  uint8 InstrLDI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrLDI::Execute(Mcu &mcu, Command cmd) const
  {
    Command k, d ;
    xxxxKKKKxxxxKKKK(cmd, k) ;
    xxxxxxxxRRRRxxxx1(cmd, d) ;
    mcu.Reg(d, (uint8)k) ;
  }
  std::string InstrLDI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  XrefType InstrLDI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDS
  InstrLDS::InstrLDS() : Instruction(0b1001000000000000, 0b1111111000001111, "LDS", "Load Direct from Data Space")
  {
  }

  InstrLDS::~InstrLDS()
  {
  }

  uint8 InstrLDS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrLDS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxxk16(*this, mcu, cmd) ;
  }
  XrefType InstrLDS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDx1
  InstrLDx1::InstrLDx1() : Instruction(0b1001000000001100, 0b1111111000001111, "LD", "Load Indirect from Data Space using X")
  {
  }

  InstrLDx1::~InstrLDx1()
  {
  }

  uint8 InstrLDx1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrLDx1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDx1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "X") ;
  }
  XrefType InstrLDx1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDx2
  InstrLDx2::InstrLDx2() : Instruction(0b1001000000001101, 0b1111111000001111, "LD", "Load Indirect from Data Space using X")
  {
  }

  InstrLDx2::~InstrLDx2()
  {
  }

  uint8 InstrLDx2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDx2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDx2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "X+") ;
  }
  XrefType InstrLDx2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDx3
  InstrLDx3::InstrLDx3() : Instruction(0b1001000000001110, 0b1111111000001111, "LD", "Load Indirect from Data Space using X")
  {
  }

  InstrLDx3::~InstrLDx3()
  {
  }

  uint8 InstrLDx3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDx3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDx3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "-X") ;
  }
  XrefType InstrLDx3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy1
  InstrLDy1::InstrLDy1() : Instruction(0b1000000000001000, 0b1111111000001111, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy1::~InstrLDy1()
  {
  }

  uint8 InstrLDy1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrLDy1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDy1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Y") ;
  }
  XrefType InstrLDy1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy2
  InstrLDy2::InstrLDy2() : Instruction(0b1001000000001001, 0b1111111000001111, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy2::~InstrLDy2()
  {
  }

  uint8 InstrLDy2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDy2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDy2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Y+") ;
  }
  XrefType InstrLDy2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy3
  InstrLDy3::InstrLDy3() : Instruction(0b1001000000001010, 0b1111111000001111, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy3::~InstrLDy3()
  {
  }

  uint8 InstrLDy3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDy3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDy3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "-Y") ;
  }
  XrefType InstrLDy3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy4
  InstrLDy4::InstrLDy4() : Instruction(0b1000000000001000, 0b1101001000001000, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy4::~InstrLDy4()
  {
  }

  uint8 InstrLDy4::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDy4::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDy4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxDDDDDxQQQ(*this, mcu, cmd, "Y") ;
  }
  XrefType InstrLDy4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz1
  InstrLDz1::InstrLDz1() : Instruction(0b1000000000000000, 0b1111111000001111, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz1::~InstrLDz1()
  {
  }

  uint8 InstrLDz1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrLDz1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDz1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z") ;
  }
  XrefType InstrLDz1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz2
  InstrLDz2::InstrLDz2() : Instruction(0b1001000000000001, 0b1111111000001111, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz2::~InstrLDz2()
  {
  }

  uint8 InstrLDz2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDz2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDz2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z+") ;
  }
  XrefType InstrLDz2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz3
  InstrLDz3::InstrLDz3() : Instruction(0b1001000000000010, 0b1111111000001111, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz3::~InstrLDz3()
  {
  }

  uint8 InstrLDz3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDz3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDz3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "-Z") ;
  }
  XrefType InstrLDz3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz4
  InstrLDz4::InstrLDz4() : Instruction(0b1000000000000000, 0b1101001000001000, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz4::~InstrLDz4()
  {
  }

  uint8 InstrLDz4::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrLDz4::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLDz4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxDDDDDxQQQ(*this, mcu, cmd, "Z") ;
  }
  XrefType InstrLDz4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STS
  InstrSTS::InstrSTS() : Instruction(0b1001001000000000, 0b1111111000001111, "STS", "Store Direct to Data Space")
  {
  }

  InstrSTS::~InstrSTS()
  {
  }

  uint8 InstrSTS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrSTS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxxk16(*this, mcu, cmd) ;
  }
  XrefType InstrSTS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STx1
  InstrSTx1::InstrSTx1() : Instruction(0b1001001000001100, 0b1111111000001111, "ST", "Store Indirect to Data Space using X")
  {
  }

  InstrSTx1::~InstrSTx1()
  {
  }

  uint8 InstrSTx1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTx1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTx1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "X") ;
  }
  XrefType InstrSTx1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STx2
  InstrSTx2::InstrSTx2() : Instruction(0b1001001000001101, 0b1111111000001111, "ST", "Store Indirect to Data Space using X")
  {
  }

  InstrSTx2::~InstrSTx2()
  {
  }

  uint8 InstrSTx2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTx2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTx2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "X+") ;
  }
  XrefType InstrSTx2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STx3
  InstrSTx3::InstrSTx3() : Instruction(0b1001001000001110, 0b1111111000001111, "ST", "Store Indirect to Data Space using X")
  {
  }

  InstrSTx3::~InstrSTx3()
  {
  }

  uint8 InstrSTx3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTx3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTx3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "-X") ;
  }
  XrefType InstrSTx3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy1
  InstrSTy1::InstrSTy1() : Instruction(0b1000001000001000, 0b1111111000001111, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy1::~InstrSTy1()
  {
  }

  uint8 InstrSTy1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTy1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTy1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Y") ;
  }
  XrefType InstrSTy1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy2
  InstrSTy2::InstrSTy2() : Instruction(0b1001001000001001, 0b1111111000001111, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy2::~InstrSTy2()
  {
  }

  uint8 InstrSTy2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTy2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTy2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Y+") ;
  }
  XrefType InstrSTy2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy3
  InstrSTy3::InstrSTy3() : Instruction(0b1001001000001010, 0b1111111000001111, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy3::~InstrSTy3()
  {
  }

  uint8 InstrSTy3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTy3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTy3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "-Y") ;
  }
  XrefType InstrSTy3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy4
  InstrSTy4::InstrSTy4() : Instruction(0b1000001000001000, 0b1101001000001000, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy4::~InstrSTy4()
  {
  }

  uint8 InstrSTy4::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTy4::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTy4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxRRRRRxQQQ(*this, mcu, cmd, "Y") ;
  }
  XrefType InstrSTy4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz1
  InstrSTz1::InstrSTz1() : Instruction(0b1000001000000000, 0b1111111000001111, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz1::~InstrSTz1()
  {
  }

  uint8 InstrSTz1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTz1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTz1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Z") ;
  }
  XrefType InstrSTz1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz2
  InstrSTz2::InstrSTz2() : Instruction(0b1001001000000001, 0b1111111000001111, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz2::~InstrSTz2()
  {
  }

  uint8 InstrSTz2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTz2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTz2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Z+") ;
  }
  XrefType InstrSTz2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz3
  InstrSTz3::InstrSTz3() : Instruction(0b1001001000000010, 0b1111111000001111, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz3::~InstrSTz3()
  {
  }

  uint8 InstrSTz3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTz3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTz3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "-Z") ;
  }
  XrefType InstrSTz3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz4
  InstrSTz4::InstrSTz4() : Instruction(0b1000001000000000, 0b1101001000001000, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz4::~InstrSTz4()
  {
  }

  uint8 InstrSTz4::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrSTz4::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSTz4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxRRRRRxQQQ(*this, mcu, cmd, "Z") ;
  }
  XrefType InstrSTz4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LPM1
  InstrLPM1::InstrLPM1() : Instruction(0b1001010111001000, 0b1111111111111111, "LPM", "Load Program Memory")
  {
  }

  InstrLPM1::~InstrLPM1()
  {
  }

  uint8 InstrLPM1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrLPM1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLPM1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrLPM1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LPM2
  InstrLPM2::InstrLPM2() : Instruction(0b1001000000000100, 0b1111111000001111, "LPM", "Load Program Memory")
  {
  }

  InstrLPM2::~InstrLPM2()
  {
  }

  uint8 InstrLPM2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrLPM2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLPM2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z") ;
  }
  XrefType InstrLPM2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LPM3
  InstrLPM3::InstrLPM3() : Instruction(0b1001000000000101, 0b1111111000001111, "LPM", "Load Program Memory")
  {
  }

  InstrLPM3::~InstrLPM3()
  {
  }

  uint8 InstrLPM3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrLPM3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLPM3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z+") ;
  }
  XrefType InstrLPM3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ELPM1
  InstrELPM1::InstrELPM1() : Instruction(0b1001010111011000, 0b1111111111111111, "ELPM", "Extended Load Program Memory")
  {
  }

  InstrELPM1::~InstrELPM1()
  {
  }

  uint8 InstrELPM1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrELPM1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrELPM1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrELPM1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ELPM2
  InstrELPM2::InstrELPM2() : Instruction(0b1001000000000110, 0b1111111000001111, "ELPM", "Extended Load Program Memory")
  {
  }

  InstrELPM2::~InstrELPM2()
  {
  }

  uint8 InstrELPM2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrELPM2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrELPM2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z") ;
  }
  XrefType InstrELPM2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ELPM3
  InstrELPM3::InstrELPM3() : Instruction(0b1001000000000111, 0b1111111000001111, "ELPM", "Extended Load Program Memory")
  {
  }

  InstrELPM3::~InstrELPM3()
  {
  }

  uint8 InstrELPM3::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ;
  }
  void InstrELPM3::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrELPM3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z+") ;
  }
  XrefType InstrELPM3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SPM1
  InstrSPM1::InstrSPM1() : Instruction(0b1001010111101000, 0b1111111111111111, "SPM", "Store Program Memory")
  {
  }

  InstrSPM1::~InstrSPM1()
  {
  }

  uint8 InstrSPM1::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ; // todo
  }
  void InstrSPM1::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSPM1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrSPM1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SPM2
  InstrSPM2::InstrSPM2() : Instruction(0b1001010111111000, 0b1111111111111111, "SPM Z+", "Store Program Memory")
  {
  }

  InstrSPM2::~InstrSPM2()
  {
  }

  uint8 InstrSPM2::Ticks(Mcu &mcu, Command cmd) const
  {
    return 3 ; // todo
  }
  void InstrSPM2::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSPM2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrSPM2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // IN
  InstrIN::InstrIN() : Instruction(0b1011000000000000, 0b1111100000000000, "IN", "Load an I/O Location to Register")
  {
  }

  InstrIN::~InstrIN()
  {
  }

  uint8 InstrIN::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrIN::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrIN::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxAADDDDDAAAA(*this, mcu, cmd) ;
  }
  XrefType InstrIN::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // OUT
  InstrOUT::InstrOUT() : Instruction(0b1011100000000000, 0b1111100000000000, "OUT", "Store Register to I/O Location")
  {
  }

  InstrOUT::~InstrOUT()
  {
  }

  uint8 InstrOUT::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrOUT::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrOUT::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxAARRRRRAAAA(*this, mcu, cmd) ;
  }
  XrefType InstrOUT::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // PUSH
  InstrPUSH::InstrPUSH() : Instruction(0b1001001000001111, 0b1111111000001111, "PUSH", "Push Register on Stack")
  {
  }

  InstrPUSH::~InstrPUSH()
  {
  }

  uint8 InstrPUSH::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrPUSH::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrPUSH::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrPUSH::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // POP
  InstrPOP::InstrPOP() : Instruction(0b1001000000001111, 0b1111111000001111, "POP", "Pop Register from Stack")
  {
  }

  InstrPOP::~InstrPOP()
  {
  }

  uint8 InstrPOP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 2 ; // todo
  }
  void InstrPOP::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrPOP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrPOP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // XCH
  InstrXCH::InstrXCH() : Instruction(0b1001001000000100, 0b1111111000001111, "XCH Z,", "Exchange Indirect Register and Data Space")
  {
  }

  InstrXCH::~InstrXCH()
  {
  }

  uint8 InstrXCH::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrXCH::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrXCH::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrXCH::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LAS
  InstrLAS::InstrLAS() : Instruction(0b1001001000000101, 0b1111111000001111, "LAS Z,", "Load and Set Indirect Register and Data Space")
  {
  }

  InstrLAS::~InstrLAS()
  {
  }

  uint8 InstrLAS::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrLAS::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLAS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrLAS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LAC
  InstrLAC::InstrLAC() : Instruction(0b1001001000000110, 0b1111111000001111, "LAC Z,", "Load and Clear Indirect Register and Data Space")
  {
  }

  InstrLAC::~InstrLAC()
  {
  }

  uint8 InstrLAC::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrLAC::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLAC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrLAC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LAT
  InstrLAT::InstrLAT() : Instruction(0b1001001000000111, 0b1111111000001111, "LAT Z,", "Load and Toggle Indirect Register and Data Space")
  {
  }

  InstrLAT::~InstrLAT()
  {
  }

  uint8 InstrLAT::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrLAT::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLAT::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrLAT::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LSR
  InstrLSR::InstrLSR() : Instruction(0b1001010000000110, 0b1111111000001111, "LSR", "Logical Shift Right")
  {
  }

  InstrLSR::~InstrLSR()
  {
  }

  uint8 InstrLSR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrLSR::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrLSR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrLSR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ROR
  InstrROR::InstrROR() : Instruction(0b1001010000000111, 0b1111111000001111, "ROR", "Rotate Right through Carry")
  {
  }

  InstrROR::~InstrROR()
  {
  }

  uint8 InstrROR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrROR::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrROR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrROR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ASR
  InstrASR::InstrASR() : Instruction(0b1001010000000101, 0b1111111000001111, "ASR", "Arithmetic Shift Right")
  {
  }

  InstrASR::~InstrASR()
  {
  }

  uint8 InstrASR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrASR::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrASR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrASR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SWAP
  InstrSWAP::InstrSWAP() : Instruction(0b1001010000000010, 0b1111111000001111, "SWAP", "Swap Nibbles")
  {
  }

  InstrSWAP::~InstrSWAP()
  {
  }

  uint8 InstrSWAP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrSWAP::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSWAP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  XrefType InstrSWAP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BSET
  InstrBSET::InstrBSET() : Instruction(0b1001010000001000, 0b1111111110001111, "BSET", "Bit Set in SREG")
  {
  }

  InstrBSET::~InstrBSET()
  {
  }

  uint8 InstrBSET::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrBSET::Execute(Mcu &mcu, Command cmd) const
  {
    Command s ;
    xxxxxxxxxSSSxxxx(cmd, s) ;

    uint8 sreg = mcu.SREG() ;
    sreg |= 1 << s ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrBSET::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxSSSxxxx_BSET(*this, cmd) ;
  }
  XrefType InstrBSET::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BCLR
  InstrBCLR::InstrBCLR() : Instruction(0b1001010010001000, 0b1111111110001111, "BCLR", "Bit Clear in SREG")
  {
  }

  InstrBCLR::~InstrBCLR()
  {
  }

  uint8 InstrBCLR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrBCLR::Execute(Mcu &mcu, Command cmd) const
  {
    Command s ;
    xxxxxxxxxSSSxxxx(cmd, s) ;

    uint8 sreg = mcu.SREG() ;
    sreg &= ~(1 << s) ;
    mcu.SREG()  = sreg ;
  }
  std::string InstrBCLR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxSSSxxxx_BCLR(*this, cmd) ;
  }
  XrefType InstrBCLR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBI
  InstrSBI::InstrSBI() : Instruction(0b1001101000000000, 0b1111111100000000, "SBI", "Set Bit in I/O Register")
  {
  }

  InstrSBI::~InstrSBI()
  {
  }

  uint8 InstrSBI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrSBI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrSBI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  XrefType InstrSBI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CBI
  InstrCBI::InstrCBI() : Instruction(0b1001100000000000, 0b1111111100000000, "CBI", "Clear Bit in I/O Register")
  {
  }

  InstrCBI::~InstrCBI()
  {
  }

  uint8 InstrCBI::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ; // todo
  }
  void InstrCBI::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrCBI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  XrefType InstrCBI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BST
  InstrBST::InstrBST() : Instruction(0b1111101000000000, 0b1111111000001000, "BST", "Bit Store from Bit in Register to T Flag in SREG")
  {
  }

  InstrBST::~InstrBST()
  {
  }

  uint8 InstrBST::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrBST::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrBST::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxBBB(*this, cmd) ;
  }
  XrefType InstrBST::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BLD
  InstrBLD::InstrBLD() : Instruction(0b1111100000000000, 0b1111111000001000, "BLD", "Bit Load from T Falg in SREG to Bit  in Register")
  {
  }

  InstrBLD::~InstrBLD()
  {
  }

  uint8 InstrBLD::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrBLD::Execute(Mcu &mcu, Command cmd) const
  {
    // todo exec
  }
  std::string InstrBLD::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxBBB(*this, cmd) ;
  }
  XrefType InstrBLD::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BREAK
  InstrBREAK::InstrBREAK() : Instruction(0b1001010110011000, 0b1111111111111111, "BREAK", "Break")
  {
  }

  InstrBREAK::~InstrBREAK()
  {
  }

  uint8 InstrBREAK::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrBREAK::Execute(Mcu &mcu, Command cmd) const
  {
    mcu.Break() ;
  }
  std::string InstrBREAK::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrBREAK::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // NOP
  InstrNOP::InstrNOP() : Instruction(0b0000000000000000, 0b1111111111111111, "NOP", "No operation")
  {
  }

  InstrNOP::~InstrNOP()
  {
  }

  uint8 InstrNOP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrNOP::Execute(Mcu &mcu, Command cmd) const
  {
  }
  std::string InstrNOP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrNOP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SLEEP
  InstrSLEEP::InstrSLEEP() : Instruction(0b1001010110001000, 0b1111111111111111, "SLEEP", "Sleep")
  {
  }

  InstrSLEEP::~InstrSLEEP()
  {
  }

  uint8 InstrSLEEP::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrSLEEP::Execute(Mcu &mcu, Command cmd) const
  {
    mcu.Sleep() ;
  }
  std::string InstrSLEEP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrSLEEP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // WDR
  InstrWDR::InstrWDR() : Instruction(0b1001010110101000, 0b1111111111111111, "WDR", "Watchdog Reset")
  {
  }

  InstrWDR::~InstrWDR()
  {
  }

  uint8 InstrWDR::Ticks(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  void InstrWDR::Execute(Mcu &mcu, Command cmd) const
  {
    mcu.WDR() ;
  }
  std::string InstrWDR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  XrefType InstrWDR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return XrefType::none ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Instruction Instances
  ////////////////////////////////////////////////////////////////////////////////

#define INSTRinst(name) Instr##name instr##name

  ////////////////////////////////////////////////////////////////////////////////
  // Arithmetic and Logic Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTRinst(ADD) ;
  INSTRinst(ADC) ;
  INSTRinst(ADIW) ;
  INSTRinst(SUB) ;
  INSTRinst(SUBI) ;
  INSTRinst(SBC) ;
  INSTRinst(SBCI) ;
  INSTRinst(SBIW) ;
  INSTRinst(AND) ;
  INSTRinst(ANDI) ;
  INSTRinst(OR) ;
  INSTRinst(ORI) ;
  INSTRinst(EOR) ;
  INSTRinst(COM) ;
  INSTRinst(NEG) ;
  //INSTRinst(SBR) ; => ORI r, i
  //INSTRinst(CBR) ; => ANDI r, i
  INSTRinst(INC) ;
  INSTRinst(DEC) ;
  //INSTRinst(TST) ; => AND r, r
  //INSTRinst(CLR) => EOR r, r
  //INSTRinst(SER) => LDI r, 0xff
  INSTRinst(MUL) ;
  INSTRinst(MULS) ;
  INSTRinst(MULSU) ;
  INSTRinst(FMUL) ;
  INSTRinst(FMULS) ;
  INSTRinst(FMULSU) ;
  INSTRinst(DES) ;

  ////////////////////////////////////////////////////////////////////////////////
  // Branch Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTRinst(RJMP) ;
  INSTRinst(IJMP) ;
  INSTRinst(EIJMP) ;
  INSTRinst(JMP) ;
  INSTRinst(RCALL) ;
  INSTRinst(ICALL) ;
  INSTRinst(EICALL) ;
  INSTRinst(CALL) ;
  INSTRinst(RET) ;
  INSTRinst(RETI) ;
  INSTRinst(CPSE) ;
  INSTRinst(CP) ;
  INSTRinst(CPC) ;
  INSTRinst(CPI) ;
  INSTRinst(SBRC) ;
  INSTRinst(SBRS) ;
  INSTRinst(SBIC) ;
  INSTRinst(SBIS) ;
  INSTRinst(BRBS) ;
  INSTRinst(BRBC) ;

  ////////////////////////////////////////////////////////////////////////////////
  // Data Transfer Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTRinst(MOV) ;
  INSTRinst(MOVW) ;
  INSTRinst(LDI) ;
  INSTRinst(LDS) ;
  INSTRinst(LDx1) ;
  INSTRinst(LDx2) ;
  INSTRinst(LDx3) ;
  INSTRinst(LDy1) ;
  INSTRinst(LDy2) ;
  INSTRinst(LDy3) ;
  INSTRinst(LDy4) ;
  INSTRinst(LDz1) ;
  INSTRinst(LDz2) ;
  INSTRinst(LDz3) ;
  INSTRinst(LDz4) ;
  INSTRinst(STS) ;
  INSTRinst(STx1) ;
  INSTRinst(STx2) ;
  INSTRinst(STx3) ;
  INSTRinst(STy1) ;
  INSTRinst(STy2) ;
  INSTRinst(STy3) ;
  INSTRinst(STy4) ;
  INSTRinst(STz1) ;
  INSTRinst(STz2) ;
  INSTRinst(STz3) ;
  INSTRinst(STz4) ;
  INSTRinst(LPM1) ;
  INSTRinst(LPM2) ;
  INSTRinst(LPM3) ;
  INSTRinst(ELPM1) ;
  INSTRinst(ELPM2) ;
  INSTRinst(ELPM3) ;
  INSTRinst(SPM1) ;
  INSTRinst(SPM2) ;
  INSTRinst(IN) ;
  INSTRinst(OUT) ;
  INSTRinst(PUSH) ;
  INSTRinst(POP) ;
  INSTRinst(XCH) ;
  INSTRinst(LAS) ;
  INSTRinst(LAC) ;
  INSTRinst(LAT) ;

  ////////////////////////////////////////////////////////////////////////////////
  // Bit and Bit-test Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTRinst(LSR) ;
  INSTRinst(ROR) ;
  INSTRinst(ASR) ;
  INSTRinst(SWAP) ;
  INSTRinst(BSET) ;
  INSTRinst(BCLR) ;
  INSTRinst(SBI) ;
  INSTRinst(CBI) ;
  INSTRinst(BST) ;
  INSTRinst(BLD) ;

  ////////////////////////////////////////////////////////////////////////////////
  // MCU Control Instructions
  ////////////////////////////////////////////////////////////////////////////////

  INSTRinst(BREAK) ;
  INSTRinst(NOP) ;
  INSTRinst(SLEEP) ;
  INSTRinst(WDR) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
