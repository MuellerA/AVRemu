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
  inline void xxxxxxxxxxxxRRRR(Command cmd, Command &r)
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
  bool Xref_xxxxxxxxxxxxxxxx(Mcu &mcu, uint32 &addr)
  {
    addr = mcu.PC() + 1 ;
    return true ;
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

  std::string Disasm_xxxxxxxxDDDDRRRR(const Instruction &instr, Command cmd)
  {
    Command r, d ;
    xxxxxxxxRRRRxxxx2(cmd, d) ;
    xxxxxxxxxxxxRRRR(cmd, r) ;
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
    char buff[1024] ;
    sprintf(buff, "%-6s %d\t\t; 0x%05x %s", instr.Mnemonic().c_str(), (int16)k, (uint32)(mcu.PC()) + (int16)k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  bool Xref_xxxxKKKKKKKKKKKK(const Instruction &instr, const Mcu &mcu, Command cmd, uint32 &addr)
  {
    Command k ;
    xxxxKKKKKKKKKKKK(cmd, k) ;
    addr = (uint32)(mcu.PC()) + (int16)k ;
    return true ;
  }

  std::string Disasm_xxxxxxxKKKKKxxxKk16(const Instruction &instr, Mcu &mcu, Command cmd)
  {
    Command k ;
    xxxxxxxKKKKKxxxK(cmd, k) ;
    uint32 addr = (((uint32)k) << 16) + mcu.ProgramNext() ;
    char buff[1024] ;
    sprintf(buff, "%-6s 0x%05x\t\t; %s", instr.Mnemonic().c_str(), addr, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  bool Xref_xxxxxxxKKKKKxxxKk16(const Instruction &instr, Mcu &mcu, Command cmd, uint32 &addr)
  {
    Command k ;
    xxxxxxxKKKKKxxxK(cmd, k) ;
    addr = (((uint32)k) << 16) + mcu.ProgramNext() ;
    return true ;
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
    mcu.PC() ; // todo disasm io register name
    char buff[1024] ;
    sprintf(buff, "%-6s 0x%02x, %d\t\t; %s", instr.Mnemonic().c_str(), a, b, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxxKKKKKKKSSS_BRBS(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command k, s ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    xxxxxxxxxxxxxSSS(cmd, s) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %d\t\t; 0x%05x %s", sToBRBS(s), (int16)k, (uint32)(mcu.PC()) + (int16)k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxxKKKKKKKSSS_BRBC(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command k, s ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    xxxxxxxxxxxxxSSS(cmd, s) ;
    char buff[1024] ;
    sprintf(buff, "%-6s %d\t\t; 0x%05x %s", sToBRBC(s), (int16)k, (uint32)(mcu.PC()) + (int16)k, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  bool Xref_xxxxxxKKKKKKKxxx(const Instruction &instr, const Mcu &mcu, Command cmd, uint32 &addr)
  {
    Command k ;
    xxxxxxKKKKKKKxxx(cmd, k) ;
    addr = (uint32)(mcu.PC()) + (int16)k ;
    return true ;
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
    sprintf(buff, "%-6s r%d, 0x%04x\t\t; %s", instr.Mnemonic().c_str(), d, addr, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxxxRRRRRxxxxk16(const Instruction &instr, Mcu &mcu, Command cmd)
  {
    Command r ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    uint32 addr = mcu.ProgramNext() ;
    char buff[1024] ;
    sprintf(buff, "%-6s 0x%04x, r%d\t\t; %s", instr.Mnemonic().c_str(), addr, r, instr.Description().c_str()) ;
    return std::string(buff) ;
  }

  std::string Disasm_xxxxxAADDDDDAAAA(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command d, a ;
    xxxxxAAxxxxxAAAA(cmd, a) ;
    xxxxxxxRRRRRxxxx(cmd, d) ;
    char buff[1024] ;
    sprintf(buff, "%-6s r%d, 0x%02x\t\t; %s", instr.Mnemonic().c_str(), d, a, instr.Description().c_str()) ;
    return std::string(buff) ;
  }
  std::string Disasm_xxxxxAARRRRRAAAA(const Instruction &instr, const Mcu &mcu, Command cmd)
  {
    Command r, a ;
    xxxxxAAxxxxxAAAA(cmd, a) ;
    xxxxxxxRRRRRxxxx(cmd, r) ;
    char buff[1024] ;
    sprintf(buff, "%-6s 0x%02x, r%d\t\t; %s", instr.Mnemonic().c_str(), a, r, instr.Description().c_str()) ;
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

  std::size_t InstrADD::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrADD::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrADD::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ADC
  InstrADC::InstrADC() : Instruction(0b0001110000000000, 0b1111110000000000, "ADC", "Add with Carry")
  {
  }

  InstrADC::~InstrADC()
  {
  }

  std::size_t InstrADC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrADC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrADC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ADIW
  InstrADIW::InstrADIW() : Instruction(0b1001011000000000, 0b1111111100000000, "ADIW", "Add Immediate to Word")
  {
  }

  InstrADIW::~InstrADIW()
  {
  }

  std::size_t InstrADIW::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrADIW::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxKKDDKKKK(*this, cmd) ;
  }
  bool InstrADIW::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SUB
  InstrSUB::InstrSUB() : Instruction(0b0001100000000000, 0b1111110000000000, "SUB", "Subtract without Carry")
  {
  }

  InstrSUB::~InstrSUB()
  {
  }

  std::size_t InstrSUB::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSUB::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrSUB::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SUBI
  InstrSUBI::InstrSUBI() : Instruction(0b0101000000000000, 0b1111000000000000, "SUBI", "Subtract Immediate")
  {
  }

  InstrSUBI::~InstrSUBI()
  {
  }

  std::size_t InstrSUBI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSUBI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  bool InstrSUBI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBC
  InstrSBC::InstrSBC() : Instruction(0b0000100000000000, 0b1111110000000000, "SBC", "Subtract with Carry")
  {
  }

  InstrSBC::~InstrSBC()
  {
  }

  std::size_t InstrSBC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrSBC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBCI
  InstrSBCI::InstrSBCI() : Instruction(0b0100000000000000, 0b1111000000000000, "SBCI", "Subtract Immediate with Carry")
  {
  }

  InstrSBCI::~InstrSBCI()
  {
  }

  std::size_t InstrSBCI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBCI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  bool InstrSBCI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBIW
  InstrSBIW::InstrSBIW() : Instruction(0b1001011100000000, 0b1111111100000000, "SBIW", "Subtract Immediate from Word")
  {
  }

  InstrSBIW::~InstrSBIW()
  {
  }

  std::size_t InstrSBIW::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBIW::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxKKDDKKKK(*this, cmd) ;
  }
  bool InstrSBIW::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // AND
  InstrAND::InstrAND() : Instruction(0b0010000000000000, 0b1111110000000000, "AND", "Logical AND")
  {
  }

  InstrAND::~InstrAND()
  {
  }

  std::size_t InstrAND::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrAND::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrAND::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ANDI
  InstrANDI::InstrANDI() : Instruction(0b0111000000000000, 0b1111000000000000, "ANDI", "Logical AND with Immediate")
  {
  }

  InstrANDI::~InstrANDI()
  {
  }

  std::size_t InstrANDI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrANDI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  bool InstrANDI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // OR
  InstrOR::InstrOR() : Instruction(0b0010100000000000, 0b1111110000000000, "OR", "Logical OR")
  {
  }

  InstrOR::~InstrOR()
  {
  }

  std::size_t InstrOR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrOR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrOR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ORI
  InstrORI::InstrORI() : Instruction(0b0110000000000000, 0b1111000000000000, "ORI", "Logical OR with Immediate")
  {
  }

  InstrORI::~InstrORI()
  {
  }

  std::size_t InstrORI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrORI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  bool InstrORI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // EOR
  InstrEOR::InstrEOR() : Instruction(0b0010010000000000, 0b1111110000000000, "EOR", "Exclusive OR")
  {
  }

  InstrEOR::~InstrEOR()
  {
  }

  std::size_t InstrEOR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrEOR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrEOR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // COM
  InstrCOM::InstrCOM() : Instruction(0b1001010000000000, 0b1111111000001111, "COM", "One's Complement")
  {
  }

  InstrCOM::~InstrCOM()
  {
  }

  std::size_t InstrCOM::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCOM::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrCOM::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // NEG
  InstrNEG::InstrNEG() : Instruction(0b1001010000000001, 0b1111111000001111, "NEG", "Two's Complement")
  {
  }

  InstrNEG::~InstrNEG()
  {
  }

  std::size_t InstrNEG::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrNEG::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrNEG::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // INC
  InstrINC::InstrINC() : Instruction(0b1001010000000011, 0b1111111000001111, "INC", "Increment")
  {
  }

  InstrINC::~InstrINC()
  {
  }

  std::size_t InstrINC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrINC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrINC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // DEC
  InstrDEC::InstrDEC() : Instruction(0b1001010000001010, 0b1111111000001111, "DEC", "Decrement")
  {
  }

  InstrDEC::~InstrDEC()
  {
  }

  std::size_t InstrDEC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrDEC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrDEC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MUL
  InstrMUL::InstrMUL() : Instruction(0b1001110000000000, 0b1111110000000000, "MUL", "Multiply Unsigned")
  {
  }

  InstrMUL::~InstrMUL()
  {
  }

  std::size_t InstrMUL::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrMUL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrMUL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MULS
  InstrMULS::InstrMULS() : Instruction(0b0000001000000000, 0b1111111100000000, "MULS", "Multiply Signed")
  {
  }

  InstrMULS::~InstrMULS()
  {
  }

  std::size_t InstrMULS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrMULS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxDDDDRRRR(*this, cmd) ;
  }
  bool InstrMULS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MULSU
  InstrMULSU::InstrMULSU() : Instruction(0b0000001100000000, 0b1111111110001000, "MULSU", "Multiply Signed with Unsigned")
  {
  }

  InstrMULSU::~InstrMULSU()
  {
  }

  std::size_t InstrMULSU::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrMULSU::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  bool InstrMULSU::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // FMUL
  InstrFMUL::InstrFMUL() : Instruction(0b0000001100001000, 0b1111111110001000, "FMUL", "Fractional Multiply Unsigned")
  {
  }

  InstrFMUL::~InstrFMUL()
  {
  }

  std::size_t InstrFMUL::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrFMUL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  bool InstrFMUL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // FMULS
  InstrFMULS::InstrFMULS() : Instruction(0b0000001110000000, 0b1111111110001000, "FMULS", "Fractional Multiply Signed")
  {
  }

  InstrFMULS::~InstrFMULS()
  {
  }

  std::size_t InstrFMULS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrFMULS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  bool InstrFMULS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // FMULSU
  InstrFMULSU::InstrFMULSU() : Instruction(0b0000001110001000, 0b1111111110001000, "FMULSU", "Fractional Multiply Signed with Unsigned")
  {
  }

  InstrFMULSU::~InstrFMULSU()
  {
  }

  std::size_t InstrFMULSU::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrFMULSU::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxDDDxRRR(*this, cmd) ;
  }
  bool InstrFMULSU::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // DES
  InstrDES::InstrDES() : Instruction(0b1001010000001011, 0b1111111100001111, "DES", "Data Encryption Standard")
  {
  }

  InstrDES::~InstrDES()
  {
  }

  std::size_t InstrDES::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrDES::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxKKKKxxxx(*this, cmd) ;
  }
  bool InstrDES::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RJMP
  InstrRJMP::InstrRJMP() : Instruction(0b1100000000000000, 0b1111000000000000, "RJMP", "Relative Jump")
  {
  }

  InstrRJMP::~InstrRJMP()
  {
  }

  std::size_t InstrRJMP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrRJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKKKKKKKKK(*this, mcu, cmd) ;
  }
  bool InstrRJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxKKKKKKKKKKKK(*this, mcu, cmd, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // IJMP
  InstrIJMP::InstrIJMP() : Instruction(0b1001010000001001, 0b1111111111111111, "IJMP", "Indirect Jump")
  {
  }

  InstrIJMP::~InstrIJMP()
  {
  }

  std::size_t InstrIJMP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrIJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrIJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // EIJMP
  InstrEIJMP::InstrEIJMP() : Instruction(0b1001010000011001, 0b1111111111111111, "EIJMP", "Extended Indirect Jump")
  {
  }

  InstrEIJMP::~InstrEIJMP()
  {
  }

  std::size_t InstrEIJMP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrEIJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrEIJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // JMP
  InstrJMP::InstrJMP() : Instruction(0b1001010000001100, 0b1111111000001110, "JMP", "Jump")
  {
  }

  InstrJMP::~InstrJMP()
  {
  }

  std::size_t InstrJMP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrJMP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd) ;
  }
  bool InstrJMP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RCALL
  InstrRCALL::InstrRCALL() : Instruction(0b1101000000000000, 0b1111000000000000, "RCALL", "Relative Call to Subroutine")
  {
  }

  InstrRCALL::~InstrRCALL()
  {
  }

  std::size_t InstrRCALL::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrRCALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKKKKKKKKK(*this, mcu, cmd) ;
  }
  bool InstrRCALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxKKKKKKKKKKKK(*this, mcu, cmd, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ICALL
  InstrICALL::InstrICALL() : Instruction(0b1001010100001001, 0b1111111111111111, "ICALL", "Indirect Call to Subroutine")
  {
  }

  InstrICALL::~InstrICALL()
  {
  }

  std::size_t InstrICALL::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrICALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrICALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // EICALL
  InstrEICALL::InstrEICALL() : Instruction(0b1001010100011001, 0b1111111111111111, "EICALL", "Extended Indirect Call to Subroutine")
  {
  }

  InstrEICALL::~InstrEICALL()
  {
  }

  std::size_t InstrEICALL::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrEICALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrEICALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CALL
  InstrCALL::InstrCALL() : Instruction(0b1001010000001110, 0b1111111000001110, "CALL", "Call to Subroutine")
  {
  }

  InstrCALL::~InstrCALL()
  {
  }

  std::size_t InstrCALL::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCALL::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd) ;
  }
  bool InstrCALL::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxKKKKKxxxKk16(*this, mcu, cmd, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RET
  InstrRET::InstrRET() : Instruction(0b1001010100001000, 0b1111111111111111, "RET", "Return from Subroutine")
  {
  }

  InstrRET::~InstrRET()
  {
  }

  std::size_t InstrRET::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrRET::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrRET::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // RETI
  InstrRETI::InstrRETI() : Instruction(0b1001010100011000, 0b1111111111111111, "RETI", "Return from Interrupt")
  {
  }

  InstrRETI::~InstrRETI()
  {
  }

  std::size_t InstrRETI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrRETI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrRETI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CPSE
  InstrCPSE::InstrCPSE() : Instruction(0b0001000000000000, 0b1111110000000000, "CPSE", "Compare Skip if Equal")
  {
  }

  InstrCPSE::~InstrCPSE()
  {
  }

  std::size_t InstrCPSE::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCPSE::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrCPSE::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxxxxxxxxxx(mcu, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CP
  InstrCP::InstrCP() : Instruction(0b0001010000000000, 0b1111110000000000, "CP", "Compare")
  {
  }

  InstrCP::~InstrCP()
  {
  }

  std::size_t InstrCP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrCP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CPC
  InstrCPC::InstrCPC() : Instruction(0b0000010000000000, 0b1111110000000000, "CPC", "Compare with Carry")
  {
  }

  InstrCPC::~InstrCPC()
  {
  }

  std::size_t InstrCPC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCPC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrCPC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CPI
  InstrCPI::InstrCPI() : Instruction(0b0011000000000000, 0b1111000000000000, "CPI", "Compare with Immediate")
  {
  }

  InstrCPI::~InstrCPI()
  {
  }

  std::size_t InstrCPI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCPI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  bool InstrCPI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBRC
  InstrSBRC::InstrSBRC() : Instruction(0b1111110000000000, 0b1111111000001000, "SBRC", "Skip if Bit in Register is Cleared")
  {
  }

  InstrSBRC::~InstrSBRC()
  {
  }

  std::size_t InstrSBRC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBRC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxBBB(*this, cmd) ;
  }
  bool InstrSBRC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxxxxxxxxxx(mcu, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBRS
  InstrSBRS::InstrSBRS() : Instruction(0b1111111000000000, 0b1111111000001000, "SBRS", "Skip if Bit in Register is Set")
  {
  }

  InstrSBRS::~InstrSBRS()
  {
  }

  std::size_t InstrSBRS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBRS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxBBB(*this, cmd) ;
  }
  bool InstrSBRS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxxxxxxxxxx(mcu, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBIC
  InstrSBIC::InstrSBIC() : Instruction(0b1001100100000000, 0b1111111100000000, "SBIC", "Skip if Bit in I/O Register is Cleared")
  {
  }

  InstrSBIC::~InstrSBIC()
  {
  }

  std::size_t InstrSBIC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBIC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  bool InstrSBIC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxxxxxxxxxx(mcu, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBIS
  InstrSBIS::InstrSBIS() : Instruction(0b1001101100000000, 0b1111111100000000, "SBIS", "Skip if Bit in I/O Registerster is Set")
  {
  }

  InstrSBIS::~InstrSBIS()
  {
  }

  std::size_t InstrSBIS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBIS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  bool InstrSBIS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxxxxxxxxxxx(mcu, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BRBS
  InstrBRBS::InstrBRBS() : Instruction(0b1111000000000000, 0b1111110000000000, "BRBS", "Branch if Bit in SREG is Set")
  {
  }

  InstrBRBS::~InstrBRBS()
  {
  }

  std::size_t InstrBRBS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBRBS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxKKKKKKKSSS_BRBS(*this, mcu, cmd) ;
  }
  bool InstrBRBS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxKKKKKKKxxx(*this, mcu, cmd, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BRBC
  InstrBRBC::InstrBRBC() : Instruction(0b1111010000000000, 0b1111110000000000, "BRBC", "Branch if Bit in SREG is Cleared")
  {
  }

  InstrBRBC::~InstrBRBC()
  {
  }

  std::size_t InstrBRBC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBRBC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxKKKKKKKSSS_BRBC(*this, mcu, cmd) ;
  }
  bool InstrBRBC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return Xref_xxxxxxKKKKKKKxxx(*this, mcu, cmd, addr) ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MOV
  InstrMOV::InstrMOV() : Instruction(0b0010110000000000, 0b1111110000000000, "MOV", "Copy Register")
  {
  }

  InstrMOV::~InstrMOV()
  {
  }

  std::size_t InstrMOV::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrMOV::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxRDDDDDRRRR(*this, cmd) ;
  }
  bool InstrMOV::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // MOVW
  InstrMOVW::InstrMOVW() : Instruction(0b0000000100000000, 0b1111111100000000, "MOVW", "Copy Register Word")
  {
  }

  InstrMOVW::~InstrMOVW()
  {
  }

  std::size_t InstrMOVW::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrMOVW::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxDDDDRRRR(*this, cmd) ;
  }
  bool InstrMOVW::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDI
  InstrLDI::InstrLDI() : Instruction(0b1110000000000000, 0b1111000000000000, "LDI", "Load Immediate")
  {
  }

  InstrLDI::~InstrLDI()
  {
  }

  std::size_t InstrLDI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxKKKKDDDDKKKK(*this, cmd) ;
  }
  bool InstrLDI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDS
  InstrLDS::InstrLDS() : Instruction(0b1001000000000000, 0b1111111000001111, "LDS", "Load Direct from Data Space")
  {
  }

  InstrLDS::~InstrLDS()
  {
  }

  std::size_t InstrLDS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxxk16(*this, mcu, cmd) ;
  }
  bool InstrLDS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDx1
  InstrLDx1::InstrLDx1() : Instruction(0b1001000000001100, 0b1111111000001111, "LD", "Load Indirect from Data Space using X")
  {
  }

  InstrLDx1::~InstrLDx1()
  {
  }

  std::size_t InstrLDx1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDx1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "X") ;
  }
  bool InstrLDx1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDx2
  InstrLDx2::InstrLDx2() : Instruction(0b1001000000001101, 0b1111111000001111, "LD", "Load Indirect from Data Space using X")
  {
  }

  InstrLDx2::~InstrLDx2()
  {
  }

  std::size_t InstrLDx2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDx2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "X+") ;
  }
  bool InstrLDx2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDx3
  InstrLDx3::InstrLDx3() : Instruction(0b1001000000001110, 0b1111111000001111, "LD", "Load Indirect from Data Space using X")
  {
  }

  InstrLDx3::~InstrLDx3()
  {
  }

  std::size_t InstrLDx3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDx3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "-X") ;
  }
  bool InstrLDx3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy1
  InstrLDy1::InstrLDy1() : Instruction(0b1000000000001000, 0b1111111000001111, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy1::~InstrLDy1()
  {
  }

  std::size_t InstrLDy1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDy1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Y") ;
  }
  bool InstrLDy1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy2
  InstrLDy2::InstrLDy2() : Instruction(0b1001000000001001, 0b1111111000001111, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy2::~InstrLDy2()
  {
  }

  std::size_t InstrLDy2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDy2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Y+") ;
  }
  bool InstrLDy2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy3
  InstrLDy3::InstrLDy3() : Instruction(0b1001000000001010, 0b1111111000001111, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy3::~InstrLDy3()
  {
  }

  std::size_t InstrLDy3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDy3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "-Y") ;
  }
  bool InstrLDy3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDy4
  InstrLDy4::InstrLDy4() : Instruction(0b1000000000001000, 0b1101001000001000, "LD", "Load Indirect from Data Space using Y")
  {
  }

  InstrLDy4::~InstrLDy4()
  {
  }

  std::size_t InstrLDy4::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDy4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxDDDDDxQQQ(*this, mcu, cmd, "Y") ;
  }
  bool InstrLDy4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz1
  InstrLDz1::InstrLDz1() : Instruction(0b1000000000000000, 0b1111111000001111, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz1::~InstrLDz1()
  {
  }

  std::size_t InstrLDz1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDz1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z") ;
  }
  bool InstrLDz1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz2
  InstrLDz2::InstrLDz2() : Instruction(0b1001000000000001, 0b1111111000001111, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz2::~InstrLDz2()
  {
  }

  std::size_t InstrLDz2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDz2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z+") ;
  }
  bool InstrLDz2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz3
  InstrLDz3::InstrLDz3() : Instruction(0b1001000000000010, 0b1111111000001111, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz3::~InstrLDz3()
  {
  }

  std::size_t InstrLDz3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDz3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "-Z") ;
  }
  bool InstrLDz3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LDz4
  InstrLDz4::InstrLDz4() : Instruction(0b1000000000000000, 0b1101001000001000, "LD", "Load Indirect from Data Space using Z")
  {
  }

  InstrLDz4::~InstrLDz4()
  {
  }

  std::size_t InstrLDz4::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLDz4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxDDDDDxQQQ(*this, mcu, cmd, "Z") ;
  }
  bool InstrLDz4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STS
  InstrSTS::InstrSTS() : Instruction(0b1001001000000000, 0b1111111000001111, "STS", "Store Direct to Data Space")
  {
  }

  InstrSTS::~InstrSTS()
  {
  }

  std::size_t InstrSTS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxxk16(*this, mcu, cmd) ;
  }
  bool InstrSTS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STx1
  InstrSTx1::InstrSTx1() : Instruction(0b1001001000001100, 0b1111111000001111, "ST", "Store Indirect to Data Space using X")
  {
  }

  InstrSTx1::~InstrSTx1()
  {
  }

  std::size_t InstrSTx1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTx1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "X") ;
  }
  bool InstrSTx1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STx2
  InstrSTx2::InstrSTx2() : Instruction(0b1001001000001101, 0b1111111000001111, "ST", "Store Indirect to Data Space using X")
  {
  }

  InstrSTx2::~InstrSTx2()
  {
  }

  std::size_t InstrSTx2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTx2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "X+") ;
  }
  bool InstrSTx2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STx3
  InstrSTx3::InstrSTx3() : Instruction(0b1001001000001110, 0b1111111000001111, "ST", "Store Indirect to Data Space using X")
  {
  }

  InstrSTx3::~InstrSTx3()
  {
  }

  std::size_t InstrSTx3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTx3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "-X") ;
  }
  bool InstrSTx3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy1
  InstrSTy1::InstrSTy1() : Instruction(0b1000001000001000, 0b1111111000001111, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy1::~InstrSTy1()
  {
  }

  std::size_t InstrSTy1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTy1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Y") ;
  }
  bool InstrSTy1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy2
  InstrSTy2::InstrSTy2() : Instruction(0b1001001000001001, 0b1111111000001111, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy2::~InstrSTy2()
  {
  }

  std::size_t InstrSTy2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTy2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Y+") ;
  }
  bool InstrSTy2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy3
  InstrSTy3::InstrSTy3() : Instruction(0b1001001000001010, 0b1111111000001111, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy3::~InstrSTy3()
  {
  }

  std::size_t InstrSTy3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTy3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "-Y") ;
  }
  bool InstrSTy3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STy4
  InstrSTy4::InstrSTy4() : Instruction(0b1000001000001000, 0b1101001000001000, "ST", "Store Indirect to Data Space using Y")
  {
  }

  InstrSTy4::~InstrSTy4()
  {
  }

  std::size_t InstrSTy4::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTy4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxRRRRRxQQQ(*this, mcu, cmd, "Y") ;
  }
  bool InstrSTy4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz1
  InstrSTz1::InstrSTz1() : Instruction(0b1000001000000000, 0b1111111000001111, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz1::~InstrSTz1()
  {
  }

  std::size_t InstrSTz1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTz1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Z") ;
  }
  bool InstrSTz1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz2
  InstrSTz2::InstrSTz2() : Instruction(0b1001001000000001, 0b1111111000001111, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz2::~InstrSTz2()
  {
  }

  std::size_t InstrSTz2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTz2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "Z+") ;
  }
  bool InstrSTz2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz3
  InstrSTz3::InstrSTz3() : Instruction(0b1001001000000010, 0b1111111000001111, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz3::~InstrSTz3()
  {
  }

  std::size_t InstrSTz3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTz3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxRRRRRxxxx(*this, mcu, cmd, "-Z") ;
  }
  bool InstrSTz3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // STz4
  InstrSTz4::InstrSTz4() : Instruction(0b1000001000000000, 0b1101001000001000, "ST", "Store Indirect to Data Space using Z")
  {
  }

  InstrSTz4::~InstrSTz4()
  {
  }

  std::size_t InstrSTz4::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSTz4::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxQxQQxRRRRRxQQQ(*this, mcu, cmd, "Z") ;
  }
  bool InstrSTz4::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LPM1
  InstrLPM1::InstrLPM1() : Instruction(0b1001010111001000, 0b1111111111111111, "LPM", "Load Program Memory")
  {
  }

  InstrLPM1::~InstrLPM1()
  {
  }

  std::size_t InstrLPM1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLPM1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrLPM1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LPM2
  InstrLPM2::InstrLPM2() : Instruction(0b1001000000000100, 0b1111111000001111, "LPM", "Load Program Memory")
  {
  }

  InstrLPM2::~InstrLPM2()
  {
  }

  std::size_t InstrLPM2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLPM2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z") ;
  }
  bool InstrLPM2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LPM3
  InstrLPM3::InstrLPM3() : Instruction(0b1001000000000101, 0b1111111000001111, "LPM", "Load Program Memory")
  {
  }

  InstrLPM3::~InstrLPM3()
  {
  }

  std::size_t InstrLPM3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLPM3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z+") ;
  }
  bool InstrLPM3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ELPM1
  InstrELPM1::InstrELPM1() : Instruction(0b1001010111011000, 0b1111111111111111, "ELPM", "Extended Load Program Memory")
  {
  }

  InstrELPM1::~InstrELPM1()
  {
  }

  std::size_t InstrELPM1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrELPM1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrELPM1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ELPM2
  InstrELPM2::InstrELPM2() : Instruction(0b1001000000000110, 0b1111111000001111, "ELPM", "Extended Load Program Memory")
  {
  }

  InstrELPM2::~InstrELPM2()
  {
  }

  std::size_t InstrELPM2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrELPM2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z") ;
  }
  bool InstrELPM2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ELPM3
  InstrELPM3::InstrELPM3() : Instruction(0b1001000000000111, 0b1111111000001111, "ELPM", "Extended Load Program Memory")
  {
  }

  InstrELPM3::~InstrELPM3()
  {
  }

  std::size_t InstrELPM3::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrELPM3::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, mcu, cmd, "Z+") ;
  }
  bool InstrELPM3::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SPM1
  InstrSPM1::InstrSPM1() : Instruction(0b1001010111101000, 0b1111111111111111, "SPM", "Store Program Memory")
  {
  }

  InstrSPM1::~InstrSPM1()
  {
  }

  std::size_t InstrSPM1::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSPM1::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrSPM1::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SPM2
  InstrSPM2::InstrSPM2() : Instruction(0b1001010111111000, 0b1111111111111111, "SPM Z+", "Store Program Memory")
  {
  }

  InstrSPM2::~InstrSPM2()
  {
  }

  std::size_t InstrSPM2::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSPM2::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrSPM2::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ; // todo information
  }

  ////////////////////////////////////////////////////////////////////////////////
  // IN
  InstrIN::InstrIN() : Instruction(0b1011000000000000, 0b1111100000000000, "IN", "Load an I/O Location to Register")
  {
  }

  InstrIN::~InstrIN()
  {
  }

  std::size_t InstrIN::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrIN::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxAADDDDDAAAA(*this, mcu, cmd) ;
  }
  bool InstrIN::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // OUT
  InstrOUT::InstrOUT() : Instruction(0b1011100000000000, 0b1111100000000000, "OUT", "Store Register to I/O Location")
  {
  }

  InstrOUT::~InstrOUT()
  {
  }

  std::size_t InstrOUT::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrOUT::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxAARRRRRAAAA(*this, mcu, cmd) ;
  }
  bool InstrOUT::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // PUSH
  InstrPUSH::InstrPUSH() : Instruction(0b1001001000001111, 0b1111111000001111, "PUSH", "Push Register on Stack")
  {
  }

  InstrPUSH::~InstrPUSH()
  {
  }

  std::size_t InstrPUSH::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrPUSH::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrPUSH::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // POP
  InstrPOP::InstrPOP() : Instruction(0b1001000000001111, 0b1111111000001111, "POP", "Pop Register from Stack")
  {
  }

  InstrPOP::~InstrPOP()
  {
  }

  std::size_t InstrPOP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrPOP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrPOP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // XCH
  InstrXCH::InstrXCH() : Instruction(0b1001001000000100, 0b1111111000001111, "XCH", "Exchange Indirect Register and Data Space")
  {
  }

  InstrXCH::~InstrXCH()
  {
  }

  std::size_t InstrXCH::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrXCH::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrXCH::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LAS
  InstrLAS::InstrLAS() : Instruction(0b1001001000000101, 0b1111111000001111, "LAS", "Load and Set Indirect Register and Data Space")
  {
  }

  InstrLAS::~InstrLAS()
  {
  }

  std::size_t InstrLAS::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLAS::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrLAS::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LAC
  InstrLAC::InstrLAC() : Instruction(0b1001001000000110, 0b1111111000001111, "LAC", "Load and Clear Indirect Register and Data Space")
  {
  }

  InstrLAC::~InstrLAC()
  {
  }

  std::size_t InstrLAC::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLAC::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrLAC::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LAT
  InstrLAT::InstrLAT() : Instruction(0b1001001000000111, 0b1111111000001111, "LAT", "Load and Toggle Indirect Register and Data Space")
  {
  }

  InstrLAT::~InstrLAT()
  {
  }

  std::size_t InstrLAT::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLAT::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrLAT::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // LSR
  InstrLSR::InstrLSR() : Instruction(0b1001010000000110, 0b1111111000001111, "LSR", "Logical Shift Right")
  {
  }

  InstrLSR::~InstrLSR()
  {
  }

  std::size_t InstrLSR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrLSR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrLSR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ROR
  InstrROR::InstrROR() : Instruction(0b1001010000000111, 0b1111111000001111, "ROR", "Rotate Right through Carry")
  {
  }

  InstrROR::~InstrROR()
  {
  }

  std::size_t InstrROR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrROR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrROR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // ASR
  InstrASR::InstrASR() : Instruction(0b1001010000000101, 0b1111111000001111, "ASR", "Arithmetic Shift Right")
  {
  }

  InstrASR::~InstrASR()
  {
  }

  std::size_t InstrASR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrASR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrASR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SWAP
  InstrSWAP::InstrSWAP() : Instruction(0b1001010000000010, 0b1111111000001111, "SWAP", "Swap Nibbles")
  {
  }

  InstrSWAP::~InstrSWAP()
  {
  }

  std::size_t InstrSWAP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSWAP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxxxx(*this, cmd) ;
  }
  bool InstrSWAP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BSET
  InstrBSET::InstrBSET() : Instruction(0b1001010000001000, 0b1111111110001111, "BSET", "Bit Set in SREG")
  {
  }

  InstrBSET::~InstrBSET()
  {
  }

  std::size_t InstrBSET::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBSET::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxSSSxxxx_BSET(*this, cmd) ;
  }
  bool InstrBSET::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BCLR
  InstrBCLR::InstrBCLR() : Instruction(0b1001010010001000, 0b1111111110001111, "BCLR", "Bit Clear in SREG")
  {
  }

  InstrBCLR::~InstrBCLR()
  {
  }

  std::size_t InstrBCLR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBCLR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxSSSxxxx_BCLR(*this, cmd) ;
  }
  bool InstrBCLR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SBI
  InstrSBI::InstrSBI() : Instruction(0b1001101000000000, 0b1111111100000000, "SBI", "Set Bit in I/O Register")
  {
  }

  InstrSBI::~InstrSBI()
  {
  }

  std::size_t InstrSBI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSBI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  bool InstrSBI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // CBI
  InstrCBI::InstrCBI() : Instruction(0b1001100000000000, 0b1111111100000000, "CBI", "Clear Bit in I/O Register")
  {
  }

  InstrCBI::~InstrCBI()
  {
  }

  std::size_t InstrCBI::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrCBI::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxAAAAABBB(*this, mcu, cmd) ;
  }
  bool InstrCBI::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BST
  InstrBST::InstrBST() : Instruction(0b1111101000000000, 0b1111111000001000, "BST", "Bit Store from Bit in Register to T Flag in SREG")
  {
  }

  InstrBST::~InstrBST()
  {
  }

  std::size_t InstrBST::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBST::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxBBB(*this, cmd) ;
  }
  bool InstrBST::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BLD
  InstrBLD::InstrBLD() : Instruction(0b1111100000000000, 0b1111111000001000, "BLD", "Bit Load from T Falg in SREG to Bit  in Register")
  {
  }

  InstrBLD::~InstrBLD()
  {
  }

  std::size_t InstrBLD::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBLD::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxDDDDDxBBB(*this, cmd) ;
  }
  bool InstrBLD::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // BREAK
  InstrBREAK::InstrBREAK() : Instruction(0b1001010110011000, 0b1111111111111111, "BREAK", "Break")
  {
  }

  InstrBREAK::~InstrBREAK()
  {
  }

  std::size_t InstrBREAK::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrBREAK::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrBREAK::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // NOP
  InstrNOP::InstrNOP() : Instruction(0b0000000000000000, 0b1111111111111111, "NOP", "No operation")
  {
  }

  InstrNOP::~InstrNOP()
  {
  }

  std::size_t InstrNOP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrNOP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrNOP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // SLEEP
  InstrSLEEP::InstrSLEEP() : Instruction(0b1001010110001000, 0b1111111111111111, "SLEEP", "Sleep")
  {
  }

  InstrSLEEP::~InstrSLEEP()
  {
  }

  std::size_t InstrSLEEP::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrSLEEP::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrSLEEP::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // WDR
  InstrWDR::InstrWDR() : Instruction(0b1001010110101000, 0b1111111111111111, "WDR", "Watchdog Reset")
  {
  }

  InstrWDR::~InstrWDR()
  {
  }

  std::size_t InstrWDR::Execute(Mcu &mcu, Command cmd) const
  {
    return 1 ;
  }
  std::string InstrWDR::Disasm(Mcu &mcu, Command cmd) const
  {
    return Disasm_xxxxxxxxxxxxxxxx(*this) ;
  }
  bool InstrWDR::Xref(Mcu &mcu, Command cmd, uint32 &addr) const
  {
    return false ;
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
