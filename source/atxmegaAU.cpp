////////////////////////////////////////////////////////////////////////////////
// atxmegaAU.cpp
// ATxmega128A4U / 64A4U / 32A4U / 16A4U
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{
  uint8_t  ATxmegaAU::Data(uint32_t addr, bool resetOnError) const
  {
    if (addr < _ioSize)
    {
      return Io(addr) ;
    }
    else if (_nvm.EepromMapped() &&
             (0x1000 <= addr) && (addr < 0x2000))
    {
      return Eeprom((addr - 0x1000) % _eepromSize) ;
    }
    else if ((0x2000 <= addr) && (addr < (0x2000 + _ramSize)))
    {
      return _ram[addr - 0x2000] ;
    }

    char buff[80] ;
    snprintf(buff, sizeof(buff), "illegal data read at %05x: %04x\n", _pc, addr) ;
    Verbose(VerboseType::DataError, buff) ;
    //if (resetOnError)
    //  const_cast<Mcu*>(this)->_pc = 0 ;
    return 0xff ;
  }

  bool ATxmegaAU::Data(uint32_t addr, uint8_t &byte) const
  {
    if (addr < _ioSize)
    {
      return Io(addr, byte) ;
    }
    else if (_nvm.EepromMapped() &&
             (0x1000 <= addr) && (addr < 0x2000))
    {
      byte = Eeprom((addr - 0x1000) % _eepromSize) ;
      return true ;
    }
    else if ((0x2000 <= addr) && (addr < (0x2000 + _ramSize)))
    {
      byte = _ram[addr - 0x2000] ;
      return true ;
    }

    return false ;
  }
  
  void ATxmegaAU::Data(uint32_t addr, uint8_t value, bool resetOnError)
  {
    if (addr < _ioSize)
    {
      Io(addr, value) ;
      return ;
    }
//    else if (_nvm.EepromMapped() &&
//             (0x1000 <= addr) && (addr < 0x2000))
//    {
//      Eeprom((addr - 0x1000) % eepromSize, value) ;
//      return ;
//    }
    else if ((0x2000 <= addr) && (addr < (0x2000 + _ramSize)))
    {
      _ram[addr - 0x2000] = value ;
      return ;
    }

    char buff[80] ;
    snprintf(buff, sizeof(buff), "illegal data write at %05x: %04x %02x\n", _pc, addr, value) ;
    Verbose(VerboseType::DataError, buff) ;
    //if (resetOnError)
    //  _pc = 0 ;
  }

  Command  ATxmegaAU::Program(uint32_t addr) const
  {
    switch (_nvm.Lpm())
    {
    default:
      {
        if (addr >= _flashSize)
        {
          char buff[80] ;
          snprintf(buff, sizeof(buff), "invalid program memory read at %05x\n", addr) ;
          Verbose(VerboseType::ProgError, buff) ;
          return 0xffff ;
        }
        if (addr >= _loadedFlashSize)
        {
          char buff[80] ;
          snprintf(buff, sizeof(buff), "uninitialized program memory read at %05x: %05x\n", _pc, addr) ;
          Verbose(VerboseType::ProgError, buff) ;
          return 0x9508 ;
        }
        return _flash[addr] ;
      }
      break ;
    case IoXmegaNvm::LpmType::UserSignature:
      return UserSignature(addr) ;
    case IoXmegaNvm::LpmType::ProductionSignature:
      return ProductionSignature(addr) ;
    }
  }
  
  void ATxmegaAU::Program(uint32_t addr, Command cmd)
  {
    if (addr >= _flashSize)
    {
      char buff[80] ;
      snprintf(buff, sizeof(buff), "invalid program memory write at %05x: %05x %04x\n", _pc, addr, cmd) ;
      Verbose(VerboseType::ProgError, buff) ;
      return ;
    }
    _flash[addr] = cmd ;
  }

  bool ATxmegaAU::InRam(uint32_t addr) const
  {
    return (0x2000 <= addr) && (addr < (0x2000 + _ramSize)) ;
  }  

  void ATxmegaAU::RamRange(uint32_t &min, uint32_t &max) const
  {
    min = 0x2000 ;
    max = 0x2000 + _ramSize - 1 ;
  }
  
  ATxmegaAU::ATxmegaAU(const std::string &name, uint32_t flashSize, uint32_t ramSize, uint32_t eepromSize)
    : Mcu(name, flashSize, 0x1000, ramSize, eepromSize, 0x3fff),
      _cpu(*this), _nvm(*this, _cpu), _rtc(*this),
      _usartC0("USARTC0"), _usartC1("USARTC1"), _usartD0("USARTD0"), _usartD1("USARTD1"), _usartE0("USARTE0")
  {
    _isXMega = true ;

    const Instruction *instructions[]
    {
      &instrADD, &instrADC, &instrADIW, &instrSUB, &instrSUBI, &instrSBC, &instrSBCI, &instrSBIW, &instrAND, &instrANDI,
      &instrOR, &instrORI, &instrEOR, &instrCOM, &instrNEG, &instrINC, &instrDEC, &instrMUL, &instrMULS, &instrMULSU,
      &instrFMUL, &instrFMULS, &instrFMULSU, &instrDES,

      &instrRJMP, &instrIJMP, &instrEIJMP, &instrJMP, &instrRCALL, &instrICALL, &instrEICALL, &instrCALL, &instrRET,
      &instrRETI, &instrCPSE, &instrCP, &instrCPC, &instrCPI, &instrSBRC, &instrSBRS, &instrSBIC, &instrSBIS,
      &instrBRBS, &instrBRBC,

      &instrMOV, &instrMOVW, &instrLDI, &instrLDS, &instrLDx1, &instrLDx2, &instrLDx3, &instrLDy1, &instrLDy2,
      &instrLDy3, &instrLDy4, &instrLDz1, &instrLDz2, &instrLDz3, &instrLDz4, &instrSTS, &instrSTx1, &instrSTx2,
      &instrSTx3, &instrSTy1, &instrSTy2, &instrSTy3, &instrSTy4, &instrSTz1, &instrSTz2, &instrSTz3, &instrSTz4,
      &instrLPM1, &instrLPM2, &instrLPM3, &instrELPM1, &instrELPM2, &instrELPM3, &instrSPM1, &instrSPM2, &instrIN,
      &instrOUT, &instrPUSH, &instrPOP, &instrXCH, &instrLAS, &instrLAC, &instrLAT,

      &instrLSR, &instrROR, &instrASR, &instrSWAP, &instrBSET, &instrBCLR, &instrSBI, &instrCBI, &instrBST,
      &instrBLD,

      &instrBREAK, &instrNOP, &instrSLEEP, &instrWDR,
    } ;
    for (const Instruction* iInstr: instructions)
      AddInstruction(iInstr) ;

    _knownProgramAddresses = std::vector<Mcu::KnownProgramAddress>
      {
        { 0x000, "RESET"           , "RESET" },
        { 0x002, "OSCF_INT"        , "Crystal oscillator failure interrupt (NMI)" },
        { 0x004, "PORTC_INT0"      , "Port C interrupt 0" },
        { 0x006, "PORTC_INT1"      , "Port C interrupt 1" },
        { 0x008, "PORTR_INT0"      , "Port R interrupt 0" },
        { 0x00a, "PORTR_INT1"      , "Port R interrupt 1" },
        { 0x00C, "DMA_INT_CH0"     , "DMA controller channel 0 interrupt" },
        { 0x00E, "DMA_INT_CH1"     , "DMA controller channel 1 interrupt" },
        { 0x010, "DMA_INT_CH2"     , "DMA controller channel 2 interrupt" },
        { 0x012, "DMA_INT_CH3"     , "DMA controller channel 3 interrupt" },
        { 0x014, "RTC_INT_OVF"     , "Real time counter overflow interrupt" },
        { 0x016, "RTC_INT_COMP"    , "Real time counter compare interrupt" },
        { 0x018, "TWIC_INT_SLAVE"  , "Two-Wire Interface on Port C slave interrupt" },
        { 0x01A, "TWIC_INT_MASTER" , "Two-Wire Interface on Port C master interrupt" },
        { 0x01C, "TCC0_INT_OVF"    , "Timer/counter 0 on port C interrupt" },
        { 0x01E, "TCC0_INT_ERR"    , "Timer/counter 0 on port C interrupt" },
        { 0x020, "TCC0_INT_CCA"    , "Timer/counter 0 on port C interrupt" },
        { 0x022, "TCC0_INT_CCB"    , "Timer/counter 0 on port C interrupt" },
        { 0x024, "TCC0_INT_CCC"    , "Timer/counter 0 on port C interrupt" },
        { 0x026, "TCC0_INT_CCD"    , "Timer/counter 0 on port C interrupt" },
        { 0x028, "TCC1_INT_OVF"    , "Timer/counter 1 on port C interrupt" },
        { 0x02A, "TCC1_INT_ERR"    , "Timer/counter 1 on port C interrupt" },
        { 0x02C, "TCC1_INT_CCA"    , "Timer/counter 1 on port C interrupt" },
        { 0x02E, "TCC1_INT_CCB"    , "Timer/counter 1 on port C interrupt" },
        { 0x030, "SPIC_INT"        , "SPI on port C interrupt" },
        { 0x032, "USARTC0_INT_RXC" , "USART 0 on port C receive complete interrupt" },
        { 0x034, "USARTC0_INT_DRE" , "USART 0 on port C data register empty interrupt" },
        { 0x036, "USARTC0_INT_TXC" , "USART 0 on port C transmit complete interrupt" },
        { 0x038, "USARTC1_INT_RXC" , "USART 1 on port C receive complete interrupt" },
        { 0x03A, "USARTC1_INT_DRE" , "USART 1 on port C data register empty interrupt" },
        { 0x03C, "USARTC1_INT_TXC" , "USART 1 on port C transmit complete interrupt" },
        { 0x03E, "AES_INT"         , "AES interrupt" },
        { 0x040, "NVM_INT_EE"      , "Nonvolatile Memory EEPROM interrupt" },
        { 0x042, "NVM_INT_SPM"     , "Nonvolatile Memory SPM interrupt" },
        { 0x044, "PORTB_INT0"      , "Port B interrupt 0" },
        { 0x046, "PORTB_INT1"      , "Port B interrupt 1" },
        { 0x056, "PORTE_INT0"      , "Port E interrupt 0" },
        { 0x058, "PORTE_INT1"      , "Port E interrupt 1" },
        { 0x05A, "TWIE_INT_SLAVE"  , "Two-wire Interface on Port E slave interrupt" },
        { 0x05C, "TWIE_INT_MASTER" , "Two-wire Interface on Port E master interrupt" },
        { 0x05E, "TCE0_INT_OVF"    , "Timer/counter 0 on port E interrupt" },
        { 0x060, "TCE0_INT_ERR"    , "Timer/counter 0 on port E interrupt" },
        { 0x062, "TCE0_INT_CCA"    , "Timer/counter 0 on port E interrupt" },
        { 0x064, "TCE0_INT_CCB"    , "Timer/counter 0 on port E interrupt" },
        { 0x066, "TCE0_INT_CCC"    , "Timer/counter 0 on port E interrupt" },
        { 0x068, "TCE0_INT_CCD"    , "Timer/counter 0 on port E interrupt" },
        { 0x06A, "TCE1_INT_OVF"    , "Timer/counter 1 on port E interrupt" },
        { 0x06C, "TCE1_INT_ERR"    , "Timer/counter 1 on port E interrupt" },
        { 0x06E, "TCE1_INT_CCA"    , "Timer/counter 1 on port E interrupt" },
        { 0x070, "TCE1_INT_CCB"    , "Timer/counter 1 on port E interrupt" },
        { 0x074, "USARTE0_INT_RXC" , "USART 0 on port E receive complete interrupt" },
        { 0x076, "USARTE0_INT_DRE" , "USART 0 on port E data register empty interrupt" },
        { 0x078, "USARTE0_INT_TXC" , "USART 0 on port E transmit complete interrupt" },
        { 0x080, "PORTD_INT0"      , "Port D interrupt 0" },
        { 0x082, "PORTD_INT1"      , "Port D interrupt 1" },
        { 0x084, "PORTA_INT0"      , "Port A interrupt 0" },
        { 0x086, "PORTA_INT1"      , "Port A interrupt 1" },
        { 0x088, "ACA_INT_COMP0"   , "Analog Comparator 0 on Port A interrupt" },
        { 0x08A, "ACA_INT_COMP1"   , "Analog Comparator 0 on Port A interrupt" },
        { 0x08C, "ACA_INT_WINDOW"  , "Analog Comparator window on Port A interrupt" },
        { 0x08E, "ADCA_INT_CH0"    , "Analog to Digital Converter channel 0 on Port A interrupt base" },
        { 0x090, "ADCA_INT_CH1"    , "Analog to Digital Converter channel 1 on Port A interrupt base" },
        { 0x092, "ADCA_INT_CH2"    , "Analog to Digital Converter channel 2 on Port A interrupt base" },
        { 0x094, "ADCA_INT_CH3"    , "Analog to Digital Converter channel 3 on Port A interrupt base" },
        { 0x09A, "TCD0_INT_OVF"    , "Timer/counter 0 on port D interrupt" },
        { 0x09C, "TCD0_INT_ERR"    , "Timer/counter 0 on port D interrupt" },
        { 0x09E, "TCD0_INT_CCA"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A0, "TCD0_INT_CCB"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A2, "TCD0_INT_CCC"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A4, "TCD0_INT_CCD"    , "Timer/counter 0 on port D interrupt" },
        { 0x0A6, "TCD1_INT_OVF"    , "Timer/counter 1 on port D interrupt" },
        { 0x0A8, "TCD1_INT_ERR"    , "Timer/counter 1 on port D interrupt" },
        { 0x0AA, "TCD1_INT_CCA"    , "Timer/counter 1 on port D interrupt" },
        { 0x0AC, "TCD1_INT_CCB"    , "Timer/counter 1 on port D interrupt" },
        { 0x0AE, "SPID_INT"        , "SPI on port D interrupt" },
        { 0x0B0, "USARTD0_INT_RXC" , "USART 0 on port D receive complete interrupt" },
        { 0x0B2, "USARTD0_INT_DRE" , "USART 0 on port D data register empty interrupt" },
        { 0x0B4, "USARTD0_INT_TXC" , "USART 0 on port D transmit complete interrupt" },
        { 0x0B6, "USARTD1_INT_RXC" , "USART 1 on port D receive complete interrupt" },
        { 0x0B8, "USARTD1_INT_DRE" , "USART 1 on port D data register empty interrupt" },
        { 0x0BA, "USARTD1_INT_TXC" , "USART 1 on port D transmit complete interrupt" },
        { 0x0FA, "USB_INT_BUSEVENT", "USB on port D SOF, suspend, resume, bus reset, CRC, underflow, overflow, and stall error interrupts" },
        { 0x0FC, "USB_INT_TRNCOMPL", "USB on port D Transaction complete interrupt" },
      } ;

    std::vector<std::pair<uint32_t, Io::Register*>> ioRegs
    {
      { 0x0000, new IoRegisterNotImplemented(*this, "GPIOR0") }, // General Purpose IO Registers
      { 0x0001, new IoRegisterNotImplemented(*this, "GPIOR1") },
      { 0x0002, new IoRegisterNotImplemented(*this, "GPIOR2") },
      { 0x0003, new IoRegisterNotImplemented(*this, "GPIOR3") },
      { 0x0004, new IoRegisterNotImplemented(*this, "GPIOR4") },
      { 0x0005, new IoRegisterNotImplemented(*this, "GPIOR5") },
      { 0x0006, new IoRegisterNotImplemented(*this, "GPIOR6") },
      { 0x0007, new IoRegisterNotImplemented(*this, "GPIOR7") },
      { 0x0008, new IoRegisterNotImplemented(*this, "GPIOR8") },
      { 0x0009, new IoRegisterNotImplemented(*this, "GPIOR9") },
      { 0x000a, new IoRegisterNotImplemented(*this, "GPIOR10") },
      { 0x000b, new IoRegisterNotImplemented(*this, "GPIOR11") },
      { 0x000c, new IoRegisterNotImplemented(*this, "GPIOR12") },
      { 0x000d, new IoRegisterNotImplemented(*this, "GPIOR13") },
      { 0x000e, new IoRegisterNotImplemented(*this, "GPIOR14") },
      { 0x000f, new IoRegisterNotImplemented(*this, "GPIOR15") },
        
      { 0x0010, new IoRegisterNotImplemented(*this, "VPORT0_DIR") }, // Virtual Port 0
      { 0x0011, new IoRegisterNotImplemented(*this, "VPORT0_OUT") },
      { 0x0012, new IoRegisterNotImplemented(*this, "VPORT0_IN") },
      { 0x0013, new IoRegisterNotImplemented(*this, "VPORT0_INTFLAGS") },
        
      { 0x0014, new IoRegisterNotImplemented(*this, "VPORT1_DIR") }, // Virtual Port 1
      { 0x0015, new IoRegisterNotImplemented(*this, "VPORT1_OUT") },
      { 0x0016, new IoRegisterNotImplemented(*this, "VPORT1_IN") },
      { 0x0017, new IoRegisterNotImplemented(*this, "VPORT1_INTFLAGS") },
        
      { 0x0018, new IoRegisterNotImplemented(*this, "VPORT2_DIR") }, // Virtual Port 2
      { 0x0019, new IoRegisterNotImplemented(*this, "VPORT2_OUT") },
      { 0x001a, new IoRegisterNotImplemented(*this, "VPORT2_IN") },
      { 0x001b, new IoRegisterNotImplemented(*this, "VPORT2_INTFLAGS") },
        
      { 0x001c, new IoRegisterNotImplemented(*this, "VPORT3_DIR") }, // Virtual Port 3
      { 0x001d, new IoRegisterNotImplemented(*this, "VPORT3_OUT") },
      { 0x001e, new IoRegisterNotImplemented(*this, "VPORT3_IN") },
      { 0x001f, new IoRegisterNotImplemented(*this, "VPORT3_INTFLAGS") },
                
      { 0x0034, new IoXmegaCpu::Ccp(*this, _cpu) }, // CPU
      { 0x0038, new IoRamp::Ramp(*this, "RAMPD", _rampd) },
      { 0x0039, new IoRamp::Ramp(*this, "RAMPX", _rampx) },
      { 0x003a, new IoRamp::Ramp(*this, "RAMPY", _rampy) },
      { 0x003b, new IoRamp::Ramp(*this, "RAMPZ", _rampz) },
      { 0x003c, new IoRamp::Ramp(*this, "EIND" , _eind ) },
      { 0x003d, new IoSP::SPL(*this, _sp) },
      { 0x003e, new IoSP::SPH(*this, _sp) },
      { 0x003f, new IoSREG::SREG(*this, _sreg) },
        
      { 0x0040, new IoRegisterNotImplemented(*this, "CLK_CTRL") }, // Clock Control
      { 0x0042, new IoRegisterNotImplemented(*this, "CLK_PSCTRL") },
      { 0x0043, new IoRegisterNotImplemented(*this, "CLK_LOCK") },
      { 0x0044, new IoRegisterNotImplemented(*this, "CLK_RTCCTRL") },
      { 0x0045, new IoRegisterNotImplemented(*this, "CLK_USBSCTRL") },
        
      { 0x0048, new IoRegisterNotImplemented(*this, "SLEEP_CTRL") }, // Sleep Controller
        
      { 0x0050, new IoRegisterNotImplemented(*this, "OSC_CTRL") }, // Oscillator Control
      { 0x0051, new IoRegisterNotImplemented(*this, "OSC_STATUS", 0x1f) },
      { 0x0052, new IoRegisterNotImplemented(*this, "OSC_XOSCCTRL") },
      { 0x0053, new IoRegisterNotImplemented(*this, "OSC_XOSCFAIL") },
      { 0x0054, new IoRegisterNotImplemented(*this, "OSC_RC32KCAL") },
      { 0x0055, new IoRegisterNotImplemented(*this, "OSC_PLLCTRL") },
      { 0x0056, new IoRegisterNotImplemented(*this, "OSC_DFLLCTRL") },

      { 0x0060, new IoRegisterNotImplemented(*this, "DFLLRC32M_CTRL") }, // DFLL for the 32MHz Internal RC Oscillator
      { 0x0062, new IoRegisterNotImplemented(*this, "DFLLRC32M_CALA") },
      { 0x0063, new IoRegisterNotImplemented(*this, "DFLLRC32M_CALB") },
      { 0x0065, new IoRegisterNotImplemented(*this, "DFLLRC32M_COMP1") },
      { 0x0066, new IoRegisterNotImplemented(*this, "DFLLRC32M_COMP2") },
        
      { 0x0068, new IoRegisterNotImplemented(*this, "DFLLRC2M_CTRL") }, // DFLL for the 2MHz RC Oscillator
      { 0x006a, new IoRegisterNotImplemented(*this, "DFLLRC2M_CALA") },
      { 0x006b, new IoRegisterNotImplemented(*this, "DFLLRC2M_CALB") },
      { 0x006d, new IoRegisterNotImplemented(*this, "DFLLRC2M_COMP1") },
      { 0x006e, new IoRegisterNotImplemented(*this, "DFLLRC2M_COMP2") },
        
      { 0x0070, new IoRegisterNotImplemented(*this, "PR_GEN") }, // Power Reduction
      { 0x0071, new IoRegisterNotImplemented(*this, "PR_PA") },
      { 0x0072, new IoRegisterNotImplemented(*this, "PR_PB") },
      { 0x0073, new IoRegisterNotImplemented(*this, "PR_PC") },
      { 0x0074, new IoRegisterNotImplemented(*this, "PR_PD") },
      { 0x0075, new IoRegisterNotImplemented(*this, "PR_PE") },
      { 0x0076, new IoRegisterNotImplemented(*this, "PR_PF") },

      { 0x0078, new IoRegisterNotImplemented(*this, "RST_STATUS") }, // Reset Controller
      { 0x0079, new IoRegisterNotImplemented(*this, "RST_CTRL") },
        
      { 0x0080, new IoRegisterNotImplemented(*this, "WDT_CTRL") }, // Watch-Dog Timer
      { 0x0081, new IoRegisterNotImplemented(*this, "WDT_WINCTRL") },
      { 0x0082, new IoRegisterNotImplemented(*this, "WDT_STATUS") },
      { 0x0090, new IoRegisterNotImplemented(*this, "MCU_DEVID0") }, // MCU Control
      { 0x0091, new IoRegisterNotImplemented(*this, "MCU_DEVID1") },
      { 0x0092, new IoRegisterNotImplemented(*this, "MCU_DEVID2") },
      { 0x0094, new IoRegisterNotImplemented(*this, "MCU_JTAGUID") },
      { 0x0096, new IoRegisterNotImplemented(*this, "MCU_MCUCR") },
      { 0x0097, new IoRegisterNotImplemented(*this, "MCU_ANAINIT") },
      { 0x0098, new IoRegisterNotImplemented(*this, "MCU_EVSYSLOCK") },
      { 0x0099, new IoRegisterNotImplemented(*this, "MCU_AWEXLOCK") },

      { 0x00A0, new IoRegisterNotImplemented(*this, "PMIC_STATUS") }, // Programmable Multilevel Interrupt Controller
      { 0x00A1, new IoRegisterNotImplemented(*this, "PMIC_INTPRI") },
      { 0x00A2, new IoRegisterNotImplemented(*this, "PMIC_CTRL") },

      { 0x00B0, new IoRegisterNotImplemented(*this, "PORTCFG_MPCMASK") }, // Port Configuration
      { 0x00B2, new IoRegisterNotImplemented(*this, "PORTCFG_VPCTRLA") },
      { 0x00B3, new IoRegisterNotImplemented(*this, "PORTCFG_VPCTRLB") },
      { 0x00B4, new IoRegisterNotImplemented(*this, "PORTCFG_CLKEVOUT") },
      { 0x00B5, new IoRegisterNotImplemented(*this, "PORTCFG_EBIOUT") },
      { 0x00B6, new IoRegisterNotImplemented(*this, "PORTCFG_EVCTRL") },

      { 0x00C0, new IoRegisterNotImplemented(*this, "AES_CTRL") }, // AES Module
      { 0x00C1, new IoRegisterNotImplemented(*this, "AES_STATUS") },
      { 0x00C2, new IoRegisterNotImplemented(*this, "AES_STATE") },
      { 0x00C3, new IoRegisterNotImplemented(*this, "AES_KEY") },
      { 0x00C4, new IoRegisterNotImplemented(*this, "AES_INTCTRL") },

      { 0x00D0, new IoRegisterNotImplemented(*this, "CRC_CTRL") }, // CRC Module
      { 0x00D1, new IoRegisterNotImplemented(*this, "CRC_STATUS") },
      { 0x00D3, new IoRegisterNotImplemented(*this, "CRC_DATAIN") },
      { 0x00D4, new IoRegisterNotImplemented(*this, "CRC_CHECKSUM0") },
      { 0x00D5, new IoRegisterNotImplemented(*this, "CRC_CHECKSUM1") },
      { 0x00D6, new IoRegisterNotImplemented(*this, "CRC_CHECKSUM2") },
      { 0x00D7, new IoRegisterNotImplemented(*this, "CRC_CHECKSUM3") },

      { 0x0100, new IoRegisterNotImplemented(*this, "DMA_CTRL") }, // DMA Module
      { 0x0103, new IoRegisterNotImplemented(*this, "DMA_INTFLAGS") },
      { 0x0104, new IoRegisterNotImplemented(*this, "DMA_STATUS") },
      { 0x0106, new IoRegisterNotImplemented(*this, "DMA_TEMPL") },
      { 0x0107, new IoRegisterNotImplemented(*this, "DMA_TEMPH") },
      { 0x0110, new IoRegisterNotImplemented(*this, "DMA_CH0_CTRLA") },
      { 0x0111, new IoRegisterNotImplemented(*this, "DMA_CH0_CTRLB") },
      { 0x0112, new IoRegisterNotImplemented(*this, "DMA_CH0_ADDCTRL") },
      { 0x0113, new IoRegisterNotImplemented(*this, "DMA_CH0_TRIGSRC") },
      { 0x0114, new IoRegisterNotImplemented(*this, "DMA_CH0_TRFCNTL") },
      { 0x0115, new IoRegisterNotImplemented(*this, "DMA_CH0_TRFCNTH") },
      { 0x0116, new IoRegisterNotImplemented(*this, "DMA_CH0_REPCNT") },
      { 0x0118, new IoRegisterNotImplemented(*this, "DMA_CH0_SRCADDR0") },
      { 0x0119, new IoRegisterNotImplemented(*this, "DMA_CH0_SRCADDR1") },
      { 0x011a, new IoRegisterNotImplemented(*this, "DMA_CH0_SRCADDR2") },
      { 0x011c, new IoRegisterNotImplemented(*this, "DMA_CH0_DESTADDR0") },
      { 0x011d, new IoRegisterNotImplemented(*this, "DMA_CH0_DESTADDR1") },
      { 0x011e, new IoRegisterNotImplemented(*this, "DMA_CH0_DESTADDR2") },
      { 0x0120, new IoRegisterNotImplemented(*this, "DMA_CH1_CTRLA") },
      { 0x0121, new IoRegisterNotImplemented(*this, "DMA_CH1_CTRLB") },
      { 0x0122, new IoRegisterNotImplemented(*this, "DMA_CH1_ADDCTRL") },
      { 0x0123, new IoRegisterNotImplemented(*this, "DMA_CH1_TRIGSRC") },
      { 0x0124, new IoRegisterNotImplemented(*this, "DMA_CH1_TRFCNTL") },
      { 0x0125, new IoRegisterNotImplemented(*this, "DMA_CH1_TRFCNTH") },
      { 0x0126, new IoRegisterNotImplemented(*this, "DMA_CH1_REPCNT") },
      { 0x0128, new IoRegisterNotImplemented(*this, "DMA_CH1_SRCADDR0") },
      { 0x0129, new IoRegisterNotImplemented(*this, "DMA_CH1_SRCADDR1") },
      { 0x012a, new IoRegisterNotImplemented(*this, "DMA_CH1_SRCADDR2") },
      { 0x012c, new IoRegisterNotImplemented(*this, "DMA_CH1_DESTADDR0") },
      { 0x012d, new IoRegisterNotImplemented(*this, "DMA_CH1_DESTADDR1") },
      { 0x012e, new IoRegisterNotImplemented(*this, "DMA_CH1_DESTADDR2") },
      { 0x0130, new IoRegisterNotImplemented(*this, "DMA_CH2_CTRLA") },
      { 0x0131, new IoRegisterNotImplemented(*this, "DMA_CH2_CTRLB") },
      { 0x0132, new IoRegisterNotImplemented(*this, "DMA_CH2_ADDCTRL") },
      { 0x0133, new IoRegisterNotImplemented(*this, "DMA_CH2_TRIGSRC") },
      { 0x0134, new IoRegisterNotImplemented(*this, "DMA_CH2_TRFCNTL") },
      { 0x0135, new IoRegisterNotImplemented(*this, "DMA_CH2_TRFCNTH") },
      { 0x0136, new IoRegisterNotImplemented(*this, "DMA_CH2_REPCNT") },
      { 0x0138, new IoRegisterNotImplemented(*this, "DMA_CH2_SRCADDR0") },
      { 0x0139, new IoRegisterNotImplemented(*this, "DMA_CH2_SRCADDR1") },
      { 0x013a, new IoRegisterNotImplemented(*this, "DMA_CH2_SRCADDR2") },
      { 0x013c, new IoRegisterNotImplemented(*this, "DMA_CH2_DESTADDR0") },
      { 0x013d, new IoRegisterNotImplemented(*this, "DMA_CH2_DESTADDR1") },
      { 0x013e, new IoRegisterNotImplemented(*this, "DMA_CH2_DESTADDR2") },
      { 0x0140, new IoRegisterNotImplemented(*this, "DMA_CH3_CTRLA") },
      { 0x0141, new IoRegisterNotImplemented(*this, "DMA_CH3_CTRLB") },
      { 0x0142, new IoRegisterNotImplemented(*this, "DMA_CH3_ADDCTRL") },
      { 0x0143, new IoRegisterNotImplemented(*this, "DMA_CH3_TRIGSRC") },
      { 0x0144, new IoRegisterNotImplemented(*this, "DMA_CH3_TRFCNTL") },
      { 0x0145, new IoRegisterNotImplemented(*this, "DMA_CH3_TRFCNTH") },
      { 0x0146, new IoRegisterNotImplemented(*this, "DMA_CH3_REPCNT") },
      { 0x0148, new IoRegisterNotImplemented(*this, "DMA_CH3_SRCADDR0") },
      { 0x0149, new IoRegisterNotImplemented(*this, "DMA_CH3_SRCADDR1") },
      { 0x014a, new IoRegisterNotImplemented(*this, "DMA_CH3_SRCADDR2") },
      { 0x014c, new IoRegisterNotImplemented(*this, "DMA_CH3_DESTADDR0") },
      { 0x014d, new IoRegisterNotImplemented(*this, "DMA_CH3_DESTADDR1") },
      { 0x014e, new IoRegisterNotImplemented(*this, "DMA_CH3_DESTADDR2") },

      { 0x0180, new IoRegisterNotImplemented(*this, "EVSYS_CH0MUX") }, // Event System
      { 0x0181, new IoRegisterNotImplemented(*this, "EVSYS_CH1MUX") },
      { 0x0182, new IoRegisterNotImplemented(*this, "EVSYS_CH2MUX") },
      { 0x0183, new IoRegisterNotImplemented(*this, "EVSYS_CH3MUX") },
      { 0x0184, new IoRegisterNotImplemented(*this, "EVSYS_CH4MUX") },
      { 0x0185, new IoRegisterNotImplemented(*this, "EVSYS_CH5MUX") },
      { 0x0186, new IoRegisterNotImplemented(*this, "EVSYS_CH6MUX") },
      { 0x0187, new IoRegisterNotImplemented(*this, "EVSYS_CH7MUX") },
      { 0x0188, new IoRegisterNotImplemented(*this, "EVSYS_CH0CTRL") },
      { 0x0189, new IoRegisterNotImplemented(*this, "EVSYS_CH1CTRL") },
      { 0x018a, new IoRegisterNotImplemented(*this, "EVSYS_CH2CTRL") },
      { 0x018b, new IoRegisterNotImplemented(*this, "EVSYS_CH3CTRL") },
      { 0x018c, new IoRegisterNotImplemented(*this, "EVSYS_CH4CTRL") },
      { 0x018d, new IoRegisterNotImplemented(*this, "EVSYS_CH5CTRL") },
      { 0x018e, new IoRegisterNotImplemented(*this, "EVSYS_CH6CTRL") },
      { 0x018f, new IoRegisterNotImplemented(*this, "EVSYS_CH7CTRL") },
      { 0x0190, new IoRegisterNotImplemented(*this, "EVSYS_STROBE") },
      { 0x0191, new IoRegisterNotImplemented(*this, "EVSYS_DATA") },

      { 0x01C0, new IoXmegaNvm::Addr0   (*this, _nvm) }, // Non Volatile Memory (NVM) Controller
      { 0x01C1, new IoXmegaNvm::Addr1   (*this, _nvm) },
      { 0x01C2, new IoXmegaNvm::Addr2   (*this, _nvm) },
      { 0x01C4, new IoXmegaNvm::Data0   (*this, _nvm) },
      { 0x01C5, new IoXmegaNvm::Data1   (*this, _nvm) },
      { 0x01C6, new IoXmegaNvm::Data2   (*this, _nvm) },
      { 0x01Ca, new IoXmegaNvm::Cmd     (*this, _nvm) },
      { 0x01Cb, new IoXmegaNvm::CtrlA   (*this, _nvm) },
      { 0x01Cc, new IoXmegaNvm::CtrlB   (*this, _nvm) },
      { 0x01Cd, new IoXmegaNvm::IntCtrl (*this, _nvm) },
      { 0x01Cf, new IoXmegaNvm::Status  (*this, _nvm) },
      { 0x01D0, new IoXmegaNvm::LockBits(*this, _nvm) },

      { 0x0200, new IoRegisterNotImplemented(*this, "ADCA_CTRLA") }, // Analog to Digital Converter on port A
      { 0x0201, new IoRegisterNotImplemented(*this, "ADCA_CTRLB") },
      { 0x0202, new IoRegisterNotImplemented(*this, "ADCA_REFCTRL") },
      { 0x0203, new IoRegisterNotImplemented(*this, "ADCA_EVCTRL") },
      { 0x0204, new IoRegisterNotImplemented(*this, "ADCA_PRESCALER") },
      { 0x0205, new IoRegisterNotImplemented(*this, "ADCA_INTFLAGS") },
      { 0x0206, new IoRegisterNotImplemented(*this, "ADCA_TEMP") },
      { 0x020c, new IoRegisterNotImplemented(*this, "ADCA_CALL") },
      { 0x020d, new IoRegisterNotImplemented(*this, "ADCA_CALH") },
      { 0x0210, new IoRegisterNotImplemented(*this, "ADCA_CH0RESL") },
      { 0x0211, new IoRegisterNotImplemented(*this, "ADCA_CH0RESH") },
      { 0x0212, new IoRegisterNotImplemented(*this, "ADCA_CH1RESL") },
      { 0x0213, new IoRegisterNotImplemented(*this, "ADCA_CH1RESH") },
      { 0x0214, new IoRegisterNotImplemented(*this, "ADCA_CH2RESL") },
      { 0x0215, new IoRegisterNotImplemented(*this, "ADCA_CH2RESH") },
      { 0x0216, new IoRegisterNotImplemented(*this, "ADCA_CH3RESL") },
      { 0x0217, new IoRegisterNotImplemented(*this, "ADCA_CH3RESH") },
      { 0x0218, new IoRegisterNotImplemented(*this, "ADCA_CMPL") },
      { 0x0219, new IoRegisterNotImplemented(*this, "ADCA_CMPH") },
      { 0x0220, new IoRegisterNotImplemented(*this, "ADCA_CH0_CTRL") },
      { 0x0221, new IoRegisterNotImplemented(*this, "ADCA_CH0_MUXCTRL") },
      { 0x0222, new IoRegisterNotImplemented(*this, "ADCA_CH0_INTCTRL") },
      { 0x0223, new IoRegisterNotImplemented(*this, "ADCA_CH0_INTFLAGS") },
      { 0x0224, new IoRegisterNotImplemented(*this, "ADCA_CH0_RESL") },
      { 0x0225, new IoRegisterNotImplemented(*this, "ADCA_CH0_RESH") },
      { 0x0226, new IoRegisterNotImplemented(*this, "ADCA_CH0_SCAN") },
      { 0x0228, new IoRegisterNotImplemented(*this, "ADCA_CH1_CTRL") },
      { 0x0229, new IoRegisterNotImplemented(*this, "ADCA_CH1_MUXCTRL") },
      { 0x022a, new IoRegisterNotImplemented(*this, "ADCA_CH1_INTCTRL") },
      { 0x022b, new IoRegisterNotImplemented(*this, "ADCA_CH1_INTFLAGS") },
      { 0x022c, new IoRegisterNotImplemented(*this, "ADCA_CH1_RESL") },
      { 0x022d, new IoRegisterNotImplemented(*this, "ADCA_CH1_RESH") },
      { 0x022e, new IoRegisterNotImplemented(*this, "ADCA_CH1_SCAN") },
      { 0x0230, new IoRegisterNotImplemented(*this, "ADCA_CH2_CTRL") },
      { 0x0231, new IoRegisterNotImplemented(*this, "ADCA_CH2_MUXCTRL") },
      { 0x0232, new IoRegisterNotImplemented(*this, "ADCA_CH2_INTCTRL") },
      { 0x0233, new IoRegisterNotImplemented(*this, "ADCA_CH2_INTFLAGS") },
      { 0x0234, new IoRegisterNotImplemented(*this, "ADCA_CH2_RESL") },
      { 0x0235, new IoRegisterNotImplemented(*this, "ADCA_CH2_RESH") },
      { 0x0236, new IoRegisterNotImplemented(*this, "ADCA_CH2_SCAN") },
      { 0x0238, new IoRegisterNotImplemented(*this, "ADCA_CH3_CTRL") },
      { 0x0239, new IoRegisterNotImplemented(*this, "ADCA_CH3_MUXCTRL") },
      { 0x023a, new IoRegisterNotImplemented(*this, "ADCA_CH3_INTCTRL") },
      { 0x023b, new IoRegisterNotImplemented(*this, "ADCA_CH3_INTFLAGS") },
      { 0x023c, new IoRegisterNotImplemented(*this, "ADCA_CH3_RESL") },
      { 0x023d, new IoRegisterNotImplemented(*this, "ADCA_CH3_RESH") },
      { 0x023e, new IoRegisterNotImplemented(*this, "ADCA_CH3_SCAN") },

      { 0x0300, new IoRegisterNotImplemented(*this, "DACA_CTRLA") },
      { 0x0301, new IoRegisterNotImplemented(*this, "DACA_CTRLB") },
      { 0x0302, new IoRegisterNotImplemented(*this, "DACA_CTRLC") },
      { 0x0303, new IoRegisterNotImplemented(*this, "DACA_EVCTRL") },
      { 0x0305, new IoRegisterNotImplemented(*this, "DACA_STATUS") },
      { 0x0308, new IoRegisterNotImplemented(*this, "DACA_CH0GAINCAL") },
      { 0x0309, new IoRegisterNotImplemented(*this, "DACA_CH0OFFSETCAL") },
      { 0x030A, new IoRegisterNotImplemented(*this, "DACA_CH1GAINCAL") },
      { 0x030B, new IoRegisterNotImplemented(*this, "DACA_CH1OFFSETCAL") },
      { 0x0318, new IoRegisterNotImplemented(*this, "DACA_CH0DATAL") },
      { 0x0319, new IoRegisterNotImplemented(*this, "DACA_CH0DATAH") },
      { 0x031A, new IoRegisterNotImplemented(*this, "DACA_CH1DATAL") },
      { 0x031B, new IoRegisterNotImplemented(*this, "DACA_CH1DATAH") },

      { 0x0320, new IoRegisterNotImplemented(*this, "DACB_CTRLA") },
      { 0x0321, new IoRegisterNotImplemented(*this, "DACB_CTRLB") },
      { 0x0322, new IoRegisterNotImplemented(*this, "DACB_CTRLC") },
      { 0x0323, new IoRegisterNotImplemented(*this, "DACB_EVCTRL") },
      { 0x0325, new IoRegisterNotImplemented(*this, "DACB_STATUS") },
      { 0x0328, new IoRegisterNotImplemented(*this, "DACB_CH0GAINCAL") },
      { 0x0329, new IoRegisterNotImplemented(*this, "DACB_CH0OFFSETCAL") },
      { 0x032A, new IoRegisterNotImplemented(*this, "DACB_CH1GAINCAL") },
      { 0x032B, new IoRegisterNotImplemented(*this, "DACB_CH1OFFSETCAL") },
      { 0x0338, new IoRegisterNotImplemented(*this, "DACB_CH0DATAL") },
      { 0x0339, new IoRegisterNotImplemented(*this, "DACB_CH0DATAH") },
      { 0x033A, new IoRegisterNotImplemented(*this, "DACB_CH1DATAL") },
      { 0x033B, new IoRegisterNotImplemented(*this, "DACB_CH1DATAH") },
        
      { 0x0380, new IoRegisterNotImplemented(*this, "ACA_AC0CTRL") }, // Analog Comparator pair on port A
      { 0x0381, new IoRegisterNotImplemented(*this, "ACA_AC1CTRL") },
      { 0x0382, new IoRegisterNotImplemented(*this, "ACA_AC0MUXCTRL") },
      { 0x0383, new IoRegisterNotImplemented(*this, "ACA_AC1MUXCTRL") },
      { 0x0384, new IoRegisterNotImplemented(*this, "ACA_CTRLA") },
      { 0x0385, new IoRegisterNotImplemented(*this, "ACA_CTRLB") },
      { 0x0386, new IoRegisterNotImplemented(*this, "ACA_WINCTRL") },
      { 0x0387, new IoRegisterNotImplemented(*this, "ACA_STATUS") },
      { 0x0388, new IoRegisterNotImplemented(*this, "ACA_CURRCTRL") },
      { 0x0389, new IoRegisterNotImplemented(*this, "ACA_CURRCALIB") },

      { 0x0400, new IoXmegaRtc::Ctrl(*this, _rtc) }, // Real Time Counter
      { 0x0401, new IoXmegaRtc::Status(*this, _rtc) },
      { 0x0402, new IoRegisterNotImplemented(*this, "RTC_INTCTRL") },
      { 0x0403, new IoRegisterNotImplemented(*this, "RTC_INTFLAGS") },
      { 0x0404, new IoXmegaRtc::Temp(*this, _rtc) },
      { 0x0408, new IoXmegaRtc::CntL(*this, _rtc) },
      { 0x0409, new IoXmegaRtc::CntH(*this, _rtc) },
      { 0x040a, new IoRegisterNotImplemented(*this, "RTC_PERL") },
      { 0x040b, new IoRegisterNotImplemented(*this, "RTC_PERH") },
      { 0x040c, new IoRegisterNotImplemented(*this, "RTC_COMPL") },
      { 0x040d, new IoRegisterNotImplemented(*this, "RTC_COMPH") },

      { 0x0480, new IoRegisterNotImplemented(*this, "TWIC_CTRL") }, // Two Wire Interface on port C
      { 0x0481, new IoRegisterNotImplemented(*this, "TWIC_MASTER_CTRLA") },
      { 0x0482, new IoRegisterNotImplemented(*this, "TWIC_MASTER_CTRLB") },
      { 0x0483, new IoRegisterNotImplemented(*this, "TWIC_MASTER_CTRLC") },
      { 0x0484, new IoRegisterNotImplemented(*this, "TWIC_MASTER_STATUS") },
      { 0x0485, new IoRegisterNotImplemented(*this, "TWIC_MASTER_BAUD") },
      { 0x0486, new IoRegisterNotImplemented(*this, "TWIC_MASTER_ADDR") },
      { 0x0487, new IoRegisterNotImplemented(*this, "TWIC_MASTER_DATA") },
      { 0x0488, new IoRegisterNotImplemented(*this, "TWIC_SLAVE_CTRLA") },
      { 0x0489, new IoRegisterNotImplemented(*this, "TWIC_SLAVE_CTRLB") },
      { 0x048a, new IoRegisterNotImplemented(*this, "TWIC_SLAVE_STATUS") },
      { 0x048b, new IoRegisterNotImplemented(*this, "TWIC_SLAVE_ADDR") },
      { 0x048c, new IoRegisterNotImplemented(*this, "TWIC_SLAVE_DATA") },
      { 0x048d, new IoRegisterNotImplemented(*this, "TWIC_SLAVE_ADDRMASK") },

      { 0x04a0, new IoRegisterNotImplemented(*this, "TWIE_CTRL") }, // Two Wire Interface on port E
      { 0x04a1, new IoRegisterNotImplemented(*this, "TWIE_MASTER_CTRLA") },
      { 0x04a2, new IoRegisterNotImplemented(*this, "TWIE_MASTER_CTRLB") },
      { 0x04a3, new IoRegisterNotImplemented(*this, "TWIE_MASTER_CTRLC") },
      { 0x04a4, new IoRegisterNotImplemented(*this, "TWIE_MASTER_STATUS") },
      { 0x04a5, new IoRegisterNotImplemented(*this, "TWIE_MASTER_BAUD") },
      { 0x04a6, new IoRegisterNotImplemented(*this, "TWIE_MASTER_ADDR") },
      { 0x04a7, new IoRegisterNotImplemented(*this, "TWIE_MASTER_DATA") },
      { 0x04a8, new IoRegisterNotImplemented(*this, "TWIE_SLAVE_CTRLA") },
      { 0x04a9, new IoRegisterNotImplemented(*this, "TWIE_SLAVE_CTRLB") },
      { 0x04aa, new IoRegisterNotImplemented(*this, "TWIE_SLAVE_STATUS") },
      { 0x04ab, new IoRegisterNotImplemented(*this, "TWIE_SLAVE_ADDR") },
      { 0x04ac, new IoRegisterNotImplemented(*this, "TWIE_SLAVE_DATA") },
      { 0x04ad, new IoRegisterNotImplemented(*this, "TWIE_SLAVE_ADDRMASK") },

      { 0x04C0, new IoRegisterNotImplemented(*this, "USB_FRAMENUML") }, // Universal Serial Bus Interface
      { 0x04C1, new IoRegisterNotImplemented(*this, "USB_FRAMENUMH") },

      { 0x0600, new IoRegisterNotImplemented(*this, "PORTA_DIR") }, // Port A
      { 0x0601, new IoRegisterNotImplemented(*this, "PORTA_DIRSET") },
      { 0x0602, new IoRegisterNotImplemented(*this, "PORTA_DIRCLR") },
      { 0x0603, new IoRegisterNotImplemented(*this, "PORTA_DIRTGL") },
      { 0x0604, new IoRegisterNotImplemented(*this, "PORTA_OUT") },
      { 0x0605, new IoRegisterNotImplemented(*this, "PORTA_OUTSET") },
      { 0x0606, new IoRegisterNotImplemented(*this, "PORTA_OUTCLR") },
      { 0x0607, new IoRegisterNotImplemented(*this, "PORTA_OUTTGL") },
      { 0x0608, new IoRegisterNotImplemented(*this, "PORTA_IN") },
      { 0x0609, new IoRegisterNotImplemented(*this, "PORTA_INTCTRL") },
      { 0x060a, new IoRegisterNotImplemented(*this, "PORTA_INT0MASK") },
      { 0x060b, new IoRegisterNotImplemented(*this, "PORTA_INT1MASK") },
      { 0x060c, new IoRegisterNotImplemented(*this, "PORTA_INTFLAGS") },
      { 0x060e, new IoRegisterNotImplemented(*this, "PORTA_REMAP") },
      { 0x0610, new IoRegisterNotImplemented(*this, "PORTA_PIN0CTRL") },
      { 0x0611, new IoRegisterNotImplemented(*this, "PORTA_PIN1CTRL") },
      { 0x0612, new IoRegisterNotImplemented(*this, "PORTA_PIN2CTRL") },
      { 0x0613, new IoRegisterNotImplemented(*this, "PORTA_PIN3CTRL") },
      { 0x0614, new IoRegisterNotImplemented(*this, "PORTA_PIN4CTRL") },
      { 0x0615, new IoRegisterNotImplemented(*this, "PORTA_PIN5CTRL") },
      { 0x0616, new IoRegisterNotImplemented(*this, "PORTA_PIN6CTRL") },
      { 0x0617, new IoRegisterNotImplemented(*this, "PORTA_PIN7CTRL") },

      { 0x0620, new IoRegisterNotImplemented(*this, "PORTB_DIR") }, // Port B
      { 0x0621, new IoRegisterNotImplemented(*this, "PORTB_DIRSET") },
      { 0x0622, new IoRegisterNotImplemented(*this, "PORTB_DIRCLR") },
      { 0x0623, new IoRegisterNotImplemented(*this, "PORTB_DIRTGL") },
      { 0x0624, new IoRegisterNotImplemented(*this, "PORTB_OUT") },
      { 0x0625, new IoRegisterNotImplemented(*this, "PORTB_OUTSET") },
      { 0x0626, new IoRegisterNotImplemented(*this, "PORTB_OUTCLR") },
      { 0x0627, new IoRegisterNotImplemented(*this, "PORTB_OUTTGL") },
      { 0x0628, new IoRegisterNotImplemented(*this, "PORTB_IN") },
      { 0x0629, new IoRegisterNotImplemented(*this, "PORTB_INTCTRL") },
      { 0x062a, new IoRegisterNotImplemented(*this, "PORTB_INT0MASK") },
      { 0x062b, new IoRegisterNotImplemented(*this, "PORTB_INT1MASK") },
      { 0x062c, new IoRegisterNotImplemented(*this, "PORTB_INTFLAGS") },
      { 0x062e, new IoRegisterNotImplemented(*this, "PORTB_REMAP") },
      { 0x0630, new IoRegisterNotImplemented(*this, "PORTB_PIN0CTRL") },
      { 0x0631, new IoRegisterNotImplemented(*this, "PORTB_PIN1CTRL") },
      { 0x0632, new IoRegisterNotImplemented(*this, "PORTB_PIN2CTRL") },
      { 0x0633, new IoRegisterNotImplemented(*this, "PORTB_PIN3CTRL") },
      { 0x0634, new IoRegisterNotImplemented(*this, "PORTB_PIN4CTRL") },
      { 0x0635, new IoRegisterNotImplemented(*this, "PORTB_PIN5CTRL") },
      { 0x0636, new IoRegisterNotImplemented(*this, "PORTB_PIN6CTRL") },
      { 0x0637, new IoRegisterNotImplemented(*this, "PORTB_PIN7CTRL") },

      { 0x0640, new IoRegisterNotImplemented(*this, "PORTC_DIR") }, // Port C
      { 0x0641, new IoRegisterNotImplemented(*this, "PORTC_DIRSET") },
      { 0x0642, new IoRegisterNotImplemented(*this, "PORTC_DIRCLR") },
      { 0x0643, new IoRegisterNotImplemented(*this, "PORTC_DIRTGL") },
      { 0x0644, new IoRegisterNotImplemented(*this, "PORTC_OUT") },
      { 0x0645, new IoRegisterNotImplemented(*this, "PORTC_OUTSET") },
      { 0x0646, new IoRegisterNotImplemented(*this, "PORTC_OUTCLR") },
      { 0x0647, new IoRegisterNotImplemented(*this, "PORTC_OUTTGL") },
      { 0x0648, new IoRegisterNotImplemented(*this, "PORTC_IN") },
      { 0x0649, new IoRegisterNotImplemented(*this, "PORTC_INTCTRL") },
      { 0x064a, new IoRegisterNotImplemented(*this, "PORTC_INT0MASK") },
      { 0x064b, new IoRegisterNotImplemented(*this, "PORTC_INT1MASK") },
      { 0x064c, new IoRegisterNotImplemented(*this, "PORTC_INTFLAGS") },
      { 0x064e, new IoRegisterNotImplemented(*this, "PORTC_REMAP") },
      { 0x0650, new IoRegisterNotImplemented(*this, "PORTC_PIN0CTRL") },
      { 0x0651, new IoRegisterNotImplemented(*this, "PORTC_PIN1CTRL") },
      { 0x0652, new IoRegisterNotImplemented(*this, "PORTC_PIN2CTRL") },
      { 0x0653, new IoRegisterNotImplemented(*this, "PORTC_PIN3CTRL") },
      { 0x0654, new IoRegisterNotImplemented(*this, "PORTC_PIN4CTRL") },
      { 0x0655, new IoRegisterNotImplemented(*this, "PORTC_PIN5CTRL") },
      { 0x0656, new IoRegisterNotImplemented(*this, "PORTC_PIN6CTRL") },
      { 0x0657, new IoRegisterNotImplemented(*this, "PORTC_PIN7CTRL") },

      { 0x0660, new IoRegisterNotImplemented(*this, "PORTD_DIR") }, // Port D
      { 0x0661, new IoRegisterNotImplemented(*this, "PORTD_DIRSET") },
      { 0x0662, new IoRegisterNotImplemented(*this, "PORTD_DIRCLR") },
      { 0x0663, new IoRegisterNotImplemented(*this, "PORTD_DIRTGL") },
      { 0x0664, new IoRegisterNotImplemented(*this, "PORTD_OUT") },
      { 0x0665, new IoRegisterNotImplemented(*this, "PORTD_OUTSET") },
      { 0x0666, new IoRegisterNotImplemented(*this, "PORTD_OUTCLR") },
      { 0x0667, new IoRegisterNotImplemented(*this, "PORTD_OUTTGL") },
      { 0x0668, new IoRegisterNotImplemented(*this, "PORTD_IN") },
      { 0x0669, new IoRegisterNotImplemented(*this, "PORTD_INTCTRL") },
      { 0x066a, new IoRegisterNotImplemented(*this, "PORTD_INT0MASK") },
      { 0x066b, new IoRegisterNotImplemented(*this, "PORTD_INT1MASK") },
      { 0x066c, new IoRegisterNotImplemented(*this, "PORTD_INTFLAGS") },
      { 0x066e, new IoRegisterNotImplemented(*this, "PORTD_REMAP") },
      { 0x0670, new IoRegisterNotImplemented(*this, "PORTD_PIN0CTRL") },
      { 0x0671, new IoRegisterNotImplemented(*this, "PORTD_PIN1CTRL") },
      { 0x0672, new IoRegisterNotImplemented(*this, "PORTD_PIN2CTRL") },
      { 0x0673, new IoRegisterNotImplemented(*this, "PORTD_PIN3CTRL") },
      { 0x0674, new IoRegisterNotImplemented(*this, "PORTD_PIN4CTRL") },
      { 0x0675, new IoRegisterNotImplemented(*this, "PORTD_PIN5CTRL") },
      { 0x0676, new IoRegisterNotImplemented(*this, "PORTD_PIN6CTRL") },
      { 0x0677, new IoRegisterNotImplemented(*this, "PORTD_PIN7CTRL") },

      { 0x0680, new IoRegisterNotImplemented(*this, "PORTE_DIR") }, // Port E
      { 0x0681, new IoRegisterNotImplemented(*this, "PORTE_DIRSET") },
      { 0x0682, new IoRegisterNotImplemented(*this, "PORTE_DIRCLR") },
      { 0x0683, new IoRegisterNotImplemented(*this, "PORTE_DIRTGL") },
      { 0x0684, new IoRegisterNotImplemented(*this, "PORTE_OUT") },
      { 0x0685, new IoRegisterNotImplemented(*this, "PORTE_OUTSET") },
      { 0x0686, new IoRegisterNotImplemented(*this, "PORTE_OUTCLR") },
      { 0x0687, new IoRegisterNotImplemented(*this, "PORTE_OUTTGL") },
      { 0x0688, new IoRegisterNotImplemented(*this, "PORTE_IN") },
      { 0x0689, new IoRegisterNotImplemented(*this, "PORTE_INTCTRL") },
      { 0x068a, new IoRegisterNotImplemented(*this, "PORTE_INT0MASK") },
      { 0x068b, new IoRegisterNotImplemented(*this, "PORTE_INT1MASK") },
      { 0x068c, new IoRegisterNotImplemented(*this, "PORTE_INTFLAGS") },
      { 0x068e, new IoRegisterNotImplemented(*this, "PORTE_REMAP") },
      { 0x0690, new IoRegisterNotImplemented(*this, "PORTE_PIN0CTRL") },
      { 0x0691, new IoRegisterNotImplemented(*this, "PORTE_PIN1CTRL") },
      { 0x0692, new IoRegisterNotImplemented(*this, "PORTE_PIN2CTRL") },
      { 0x0693, new IoRegisterNotImplemented(*this, "PORTE_PIN3CTRL") },
      { 0x0694, new IoRegisterNotImplemented(*this, "PORTE_PIN4CTRL") },
      { 0x0695, new IoRegisterNotImplemented(*this, "PORTE_PIN5CTRL") },
      { 0x0696, new IoRegisterNotImplemented(*this, "PORTE_PIN6CTRL") },
      { 0x0697, new IoRegisterNotImplemented(*this, "PORTE_PIN7CTRL") },

      { 0x07E0, new IoRegisterNotImplemented(*this, "PORTR_DIR") }, // Port R
      { 0x07E1, new IoRegisterNotImplemented(*this, "PORTR_DIRSET") },
      { 0x07E2, new IoRegisterNotImplemented(*this, "PORTR_DIRCLR") },
      { 0x07E3, new IoRegisterNotImplemented(*this, "PORTR_DIRTGL") },
      { 0x07E4, new IoRegisterNotImplemented(*this, "PORTR_OUT") },
      { 0x07E5, new IoRegisterNotImplemented(*this, "PORTR_OUTSET") },
      { 0x07E6, new IoRegisterNotImplemented(*this, "PORTR_OUTCLR") },
      { 0x07E7, new IoRegisterNotImplemented(*this, "PORTR_OUTTGL") },
      { 0x07E8, new IoRegisterNotImplemented(*this, "PORTR_IN") },
      { 0x07E9, new IoRegisterNotImplemented(*this, "PORTR_INTCTRL") },
      { 0x07Ea, new IoRegisterNotImplemented(*this, "PORTR_INT0MASK") },
      { 0x07Eb, new IoRegisterNotImplemented(*this, "PORTR_INT1MASK") },
      { 0x07Ec, new IoRegisterNotImplemented(*this, "PORTR_INTFLAGS") },
      { 0x07Ee, new IoRegisterNotImplemented(*this, "PORTR_REMAP") },
      { 0x07F0, new IoRegisterNotImplemented(*this, "PORTR_PIN0CTRL") },
      { 0x07F1, new IoRegisterNotImplemented(*this, "PORTR_PIN1CTRL") },
      { 0x07F2, new IoRegisterNotImplemented(*this, "PORTR_PIN2CTRL") },
      { 0x07F3, new IoRegisterNotImplemented(*this, "PORTR_PIN3CTRL") },
      { 0x07F4, new IoRegisterNotImplemented(*this, "PORTR_PIN4CTRL") },
      { 0x07F5, new IoRegisterNotImplemented(*this, "PORTR_PIN5CTRL") },
      { 0x07F6, new IoRegisterNotImplemented(*this, "PORTR_PIN6CTRL") },
      { 0x07F7, new IoRegisterNotImplemented(*this, "PORTR_PIN7CTRL") },

      { 0x0800, new IoRegisterNotImplemented(*this, "TCC0_CTRLA") }, // Timer/Counter 0 on port C
      { 0x0801, new IoRegisterNotImplemented(*this, "TCC0_CTRLB") },
      { 0x0802, new IoRegisterNotImplemented(*this, "TCC0_CTRLC") },
      { 0x0803, new IoRegisterNotImplemented(*this, "TCC0_CTRLD") },
      { 0x0804, new IoRegisterNotImplemented(*this, "TCC0_CTRLE") },
      { 0x0806, new IoRegisterNotImplemented(*this, "TCC0_INTCTRLA") },
      { 0x0807, new IoRegisterNotImplemented(*this, "TCC0_INTCTRLB") },
      { 0x0808, new IoRegisterNotImplemented(*this, "TCC0_CTRLFCLR") },
      { 0x0809, new IoRegisterNotImplemented(*this, "TCC0_CTRLFSET") },
      { 0x080a, new IoRegisterNotImplemented(*this, "TCC0_CTRLGCLR") },
      { 0x080b, new IoRegisterNotImplemented(*this, "TCC0_CTRLGSET") },
      { 0x080c, new IoRegisterNotImplemented(*this, "TCC0_INTFLAGS") },
      { 0x080f, new IoRegisterNotImplemented(*this, "TCC0_TEMP") },
      { 0x0820, new IoRegisterNotImplemented(*this, "TCC0_CNTL") },
      { 0x0821, new IoRegisterNotImplemented(*this, "TCC0_CNTH") },
      { 0x0826, new IoRegisterNotImplemented(*this, "TCC0_PERL") },
      { 0x0827, new IoRegisterNotImplemented(*this, "TCC0_PERH") },
      { 0x0828, new IoRegisterNotImplemented(*this, "TCC0_CCAL") },
      { 0x0829, new IoRegisterNotImplemented(*this, "TCC0_CCAH") },
      { 0x082a, new IoRegisterNotImplemented(*this, "TCC0_CCBL") },
      { 0x082b, new IoRegisterNotImplemented(*this, "TCC0_CCBH") },
      { 0x082c, new IoRegisterNotImplemented(*this, "TCC0_CCCL") },
      { 0x082d, new IoRegisterNotImplemented(*this, "TCC0_CCCH") },
      { 0x082e, new IoRegisterNotImplemented(*this, "TCC0_CCDL") },
      { 0x082f, new IoRegisterNotImplemented(*this, "TCC0_CCDH") },
      { 0x0836, new IoRegisterNotImplemented(*this, "TCC0_PERBUFL") },
      { 0x0837, new IoRegisterNotImplemented(*this, "TCC0_PERBUFH") },
      { 0x0838, new IoRegisterNotImplemented(*this, "TCC0_CCABUFL") },
      { 0x0839, new IoRegisterNotImplemented(*this, "TCC0_CCABUFH") },
      { 0x083a, new IoRegisterNotImplemented(*this, "TCC0_CCBBUFL") },
      { 0x083b, new IoRegisterNotImplemented(*this, "TCC0_CCBBUFH") },
      { 0x083c, new IoRegisterNotImplemented(*this, "TCC0_CCCBUFL") },
      { 0x083d, new IoRegisterNotImplemented(*this, "TCC0_CCCBUFH") },
      { 0x083e, new IoRegisterNotImplemented(*this, "TCC0_CCDBUFL") },
      { 0x083f, new IoRegisterNotImplemented(*this, "TCC0_CCDBUFH") },

      { 0x0840, new IoRegisterNotImplemented(*this, "TCC1_CTRLA") }, // Timer/Counter 1 on port C
      { 0x0841, new IoRegisterNotImplemented(*this, "TCC1_CTRLB") },
      { 0x0842, new IoRegisterNotImplemented(*this, "TCC1_CTRLC") },
      { 0x0843, new IoRegisterNotImplemented(*this, "TCC1_CTRLD") },
      { 0x0844, new IoRegisterNotImplemented(*this, "TCC1_CTRLE") },
      { 0x0846, new IoRegisterNotImplemented(*this, "TCC1_INTCTRLA") },
      { 0x0847, new IoRegisterNotImplemented(*this, "TCC1_INTCTRLB") },
      { 0x0848, new IoRegisterNotImplemented(*this, "TCC1_CTRLFCLR") },
      { 0x0849, new IoRegisterNotImplemented(*this, "TCC1_CTRLFSET") },
      { 0x084a, new IoRegisterNotImplemented(*this, "TCC1_CTRLGCLR") },
      { 0x084b, new IoRegisterNotImplemented(*this, "TCC1_CTRLGSET") },
      { 0x084c, new IoRegisterNotImplemented(*this, "TCC1_INTFLAGS") },
      { 0x084f, new IoRegisterNotImplemented(*this, "TCC1_TEMP") },
      { 0x0860, new IoRegisterNotImplemented(*this, "TCC1_CNTL") },
      { 0x0861, new IoRegisterNotImplemented(*this, "TCC1_CNTH") },
      { 0x0866, new IoRegisterNotImplemented(*this, "TCC1_PERL") },
      { 0x0867, new IoRegisterNotImplemented(*this, "TCC1_PERH") },
      { 0x0868, new IoRegisterNotImplemented(*this, "TCC1_CCAL") },
      { 0x0869, new IoRegisterNotImplemented(*this, "TCC1_CCAH") },
      { 0x086a, new IoRegisterNotImplemented(*this, "TCC1_CCBL") },
      { 0x086b, new IoRegisterNotImplemented(*this, "TCC1_CCBH") },
      { 0x086c, new IoRegisterNotImplemented(*this, "TCC1_CCCL") },
      { 0x086d, new IoRegisterNotImplemented(*this, "TCC1_CCCH") },
      { 0x086e, new IoRegisterNotImplemented(*this, "TCC1_CCDL") },
      { 0x086f, new IoRegisterNotImplemented(*this, "TCC1_CCDH") },
      { 0x0876, new IoRegisterNotImplemented(*this, "TCC1_PERBUFL") },
      { 0x0877, new IoRegisterNotImplemented(*this, "TCC1_PERBUFH") },
      { 0x0878, new IoRegisterNotImplemented(*this, "TCC1_CCABUFL") },
      { 0x0879, new IoRegisterNotImplemented(*this, "TCC1_CCABUFH") },
      { 0x087a, new IoRegisterNotImplemented(*this, "TCC1_CCBBUFL") },
      { 0x087b, new IoRegisterNotImplemented(*this, "TCC1_CCBBUFH") },
      { 0x087c, new IoRegisterNotImplemented(*this, "TCC1_CCCBUFL") },
      { 0x087d, new IoRegisterNotImplemented(*this, "TCC1_CCCBUFH") },
      { 0x087e, new IoRegisterNotImplemented(*this, "TCC1_CCDBUFL") },
      { 0x087f, new IoRegisterNotImplemented(*this, "TCC1_CCDBUFH") },

      { 0x0880, new IoRegisterNotImplemented(*this, "AWEXC_CTRL") }, // Advanced Waveform Extension on port C
      { 0x0882, new IoRegisterNotImplemented(*this, "AWEXC_FDEMASK") },
      { 0x0883, new IoRegisterNotImplemented(*this, "AWEXC_FDCTRL") },
      { 0x0884, new IoRegisterNotImplemented(*this, "AWEXC_STATUS") },
      { 0x0886, new IoRegisterNotImplemented(*this, "AWEXC_DTBOTH") },
      { 0x0887, new IoRegisterNotImplemented(*this, "AWEXC_DTBOTHBUF") },
      { 0x0888, new IoRegisterNotImplemented(*this, "AWEXC_DTLS") },
      { 0x0889, new IoRegisterNotImplemented(*this, "AWEXC_DTHS") },
      { 0x088a, new IoRegisterNotImplemented(*this, "AWEXC_DTLSBUF") },
      { 0x088b, new IoRegisterNotImplemented(*this, "AWEXC_DTHSBUF") },
      { 0x088c, new IoRegisterNotImplemented(*this, "AWEXC_OUTOVEN") },

      { 0x0890, new IoRegisterNotImplemented(*this, "HIRESC_CTRLA") }, // High Resolution Extension on port C

      { 0x08A0, new IoXmegaUsart::Data     (*this, _usartC0) }, // USART 0 on port C
      { 0x08A1, new IoXmegaUsart::Status   (*this, _usartC0) },
      { 0x08A3, new IoXmegaUsart::CtrlA    (*this, _usartC0) },
      { 0x08A4, new IoXmegaUsart::CtrlB    (*this, _usartC0) },
      { 0x08A5, new IoXmegaUsart::CtrlC    (*this, _usartC0) },
      { 0x08A6, new IoXmegaUsart::BaudCtrlA(*this, _usartC0) },
      { 0x08A7, new IoXmegaUsart::BaudCtrlB(*this, _usartC0) },

      { 0x08B0, new IoXmegaUsart::Data     (*this, _usartC1) }, // USART 1 on port C
      { 0x08B1, new IoXmegaUsart::Status   (*this, _usartC1) },                     
      { 0x08B3, new IoXmegaUsart::CtrlA    (*this, _usartC1) },                     
      { 0x08B4, new IoXmegaUsart::CtrlB    (*this, _usartC1) },                     
      { 0x08B5, new IoXmegaUsart::CtrlC    (*this, _usartC1) },                     
      { 0x08B6, new IoXmegaUsart::BaudCtrlA(*this, _usartC1) },                     
      { 0x08B7, new IoXmegaUsart::BaudCtrlB(*this, _usartC1) },                     

      { 0x08C0, new IoRegisterNotImplemented(*this, "SPIC_CTRL") }, // Serial Peripheral Interface on port C
      { 0x08C1, new IoRegisterNotImplemented(*this, "SPIC_INTCTRL") },
      { 0x08C2, new IoRegisterNotImplemented(*this, "SPIC_STATUS", 0x80) },
      { 0x08C3, new IoRegisterNotImplemented(*this, "SPIC_DATA") },

      { 0x08F8, new IoRegisterNotImplemented(*this, "IRCOM_CTRL") }, // Infrared Communication Module
      { 0x08F9, new IoRegisterNotImplemented(*this, "IRCOM_TXPLCTRL") },
      { 0x08Fa, new IoRegisterNotImplemented(*this, "IRCOM_RXPLCTRL") },

      { 0x0900, new IoRegisterNotImplemented(*this, "TCD0_CTRLA") }, // Timer/Counter 0 on port D
      { 0x0901, new IoRegisterNotImplemented(*this, "TCD0_CTRLB") },
      { 0x0902, new IoRegisterNotImplemented(*this, "TCD0_CTRLC") },
      { 0x0903, new IoRegisterNotImplemented(*this, "TCD0_CTRLD") },
      { 0x0904, new IoRegisterNotImplemented(*this, "TCD0_CTRLE") },
      { 0x0906, new IoRegisterNotImplemented(*this, "TCD0_INTCTRLA") },
      { 0x0907, new IoRegisterNotImplemented(*this, "TCD0_INTCTRLB") },
      { 0x0908, new IoRegisterNotImplemented(*this, "TCD0_CTRLFCLR") },
      { 0x0909, new IoRegisterNotImplemented(*this, "TCD0_CTRLFSET") },
      { 0x090a, new IoRegisterNotImplemented(*this, "TCD0_CTRLGCLR") },
      { 0x090b, new IoRegisterNotImplemented(*this, "TCD0_CTRLGSET") },
      { 0x090c, new IoRegisterNotImplemented(*this, "TCD0_INTFLAGS") },
      { 0x090f, new IoRegisterNotImplemented(*this, "TCD0_TEMP") },
      { 0x0920, new IoRegisterNotImplemented(*this, "TCD0_CNTL") },
      { 0x0921, new IoRegisterNotImplemented(*this, "TCD0_CNTH") },
      { 0x0926, new IoRegisterNotImplemented(*this, "TCD0_PERL") },
      { 0x0927, new IoRegisterNotImplemented(*this, "TCD0_PERH") },
      { 0x0928, new IoRegisterNotImplemented(*this, "TCD0_CCAL") },
      { 0x0929, new IoRegisterNotImplemented(*this, "TCD0_CCAH") },
      { 0x092a, new IoRegisterNotImplemented(*this, "TCD0_CCBL") },
      { 0x092b, new IoRegisterNotImplemented(*this, "TCD0_CCBH") },
      { 0x092c, new IoRegisterNotImplemented(*this, "TCD0_CCCL") },
      { 0x092d, new IoRegisterNotImplemented(*this, "TCD0_CCCH") },
      { 0x092e, new IoRegisterNotImplemented(*this, "TCD0_CCDL") },
      { 0x092f, new IoRegisterNotImplemented(*this, "TCD0_CCDH") },
      { 0x0936, new IoRegisterNotImplemented(*this, "TCD0_PERBUFL") },
      { 0x0937, new IoRegisterNotImplemented(*this, "TCD0_PERBUFH") },
      { 0x0938, new IoRegisterNotImplemented(*this, "TCD0_CCABUFL") },
      { 0x0939, new IoRegisterNotImplemented(*this, "TCD0_CCABUFH") },
      { 0x093a, new IoRegisterNotImplemented(*this, "TCD0_CCBBUFL") },
      { 0x093b, new IoRegisterNotImplemented(*this, "TCD0_CCBBUFH") },
      { 0x093c, new IoRegisterNotImplemented(*this, "TCD0_CCCBUFL") },
      { 0x093d, new IoRegisterNotImplemented(*this, "TCD0_CCCBUFH") },
      { 0x093e, new IoRegisterNotImplemented(*this, "TCD0_CCDBUFL") },
      { 0x093f, new IoRegisterNotImplemented(*this, "TCD0_CCDBUFH") },

      { 0x0940, new IoRegisterNotImplemented(*this, "TCD1_CTRLA") }, // Timer/Counter 1 on port D
      { 0x0941, new IoRegisterNotImplemented(*this, "TCD1_CTRLB") },
      { 0x0942, new IoRegisterNotImplemented(*this, "TCD1_CTRLC") },
      { 0x0943, new IoRegisterNotImplemented(*this, "TCD1_CTRLD") },
      { 0x0944, new IoRegisterNotImplemented(*this, "TCD1_CTRLE") },
      { 0x0946, new IoRegisterNotImplemented(*this, "TCD1_INTCTRLA") },
      { 0x0947, new IoRegisterNotImplemented(*this, "TCD1_INTCTRLB") },
      { 0x0948, new IoRegisterNotImplemented(*this, "TCD1_CTRLFCLR") },
      { 0x0949, new IoRegisterNotImplemented(*this, "TCD1_CTRLFSET") },
      { 0x094a, new IoRegisterNotImplemented(*this, "TCD1_CTRLGCLR") },
      { 0x094b, new IoRegisterNotImplemented(*this, "TCD1_CTRLGSET") },
      { 0x094c, new IoRegisterNotImplemented(*this, "TCD1_INTFLAGS") },
      { 0x094f, new IoRegisterNotImplemented(*this, "TCD1_TEMP") },
      { 0x0960, new IoRegisterNotImplemented(*this, "TCD1_CNTL") },
      { 0x0961, new IoRegisterNotImplemented(*this, "TCD1_CNTH") },
      { 0x0966, new IoRegisterNotImplemented(*this, "TCD1_PERL") },
      { 0x0967, new IoRegisterNotImplemented(*this, "TCD1_PERH") },
      { 0x0968, new IoRegisterNotImplemented(*this, "TCD1_CCAL") },
      { 0x0969, new IoRegisterNotImplemented(*this, "TCD1_CCAH") },
      { 0x096a, new IoRegisterNotImplemented(*this, "TCD1_CCBL") },
      { 0x096b, new IoRegisterNotImplemented(*this, "TCD1_CCBH") },
      { 0x096c, new IoRegisterNotImplemented(*this, "TCD1_CCCL") },
      { 0x096d, new IoRegisterNotImplemented(*this, "TCD1_CCCH") },
      { 0x096e, new IoRegisterNotImplemented(*this, "TCD1_CCDL") },
      { 0x096f, new IoRegisterNotImplemented(*this, "TCD1_CCDH") },
      { 0x0976, new IoRegisterNotImplemented(*this, "TCD1_PERBUFL") },
      { 0x0977, new IoRegisterNotImplemented(*this, "TCD1_PERBUFH") },
      { 0x0978, new IoRegisterNotImplemented(*this, "TCD1_CCABUFL") },
      { 0x0979, new IoRegisterNotImplemented(*this, "TCD1_CCABUFH") },
      { 0x097a, new IoRegisterNotImplemented(*this, "TCD1_CCBBUFL") },
      { 0x097b, new IoRegisterNotImplemented(*this, "TCD1_CCBBUFH") },
      { 0x097c, new IoRegisterNotImplemented(*this, "TCD1_CCCBUFL") },
      { 0x097d, new IoRegisterNotImplemented(*this, "TCD1_CCCBUFH") },
      { 0x097e, new IoRegisterNotImplemented(*this, "TCD1_CCDBUFL") },
      { 0x097f, new IoRegisterNotImplemented(*this, "TCD1_CCDBUFH") },

      { 0x0990, new IoRegisterNotImplemented(*this, "HIRESD_CTRLA") }, // High Resolution Extension on port D

      { 0x09A0, new IoXmegaUsart::Data     (*this, _usartD0) }, // USART 0 on port D
      { 0x09A1, new IoXmegaUsart::Status   (*this, _usartD0) },                     
      { 0x09A3, new IoXmegaUsart::CtrlA    (*this, _usartD0) },                     
      { 0x09A4, new IoXmegaUsart::CtrlB    (*this, _usartD0) },                     
      { 0x09A5, new IoXmegaUsart::CtrlC    (*this, _usartD0) },                     
      { 0x09A6, new IoXmegaUsart::BaudCtrlA(*this, _usartD0) },                     
      { 0x09A7, new IoXmegaUsart::BaudCtrlB(*this, _usartD0) },                     

      { 0x09B0, new IoXmegaUsart::Data     (*this, _usartD1) }, // USART 1 on port D
      { 0x09B1, new IoXmegaUsart::Status   (*this, _usartD1) },                     
      { 0x09B3, new IoXmegaUsart::CtrlA    (*this, _usartD1) },                     
      { 0x09B4, new IoXmegaUsart::CtrlB    (*this, _usartD1) },                     
      { 0x09B5, new IoXmegaUsart::CtrlC    (*this, _usartD1) },                     
      { 0x09B6, new IoXmegaUsart::BaudCtrlA(*this, _usartD1) },                     
      { 0x09B7, new IoXmegaUsart::BaudCtrlB(*this, _usartD1) },                     

      { 0x09C0, new IoRegisterNotImplemented(*this, "SPID_CTRL") }, // Serial Peripheral Interface on port D
      { 0x09C1, new IoRegisterNotImplemented(*this, "SPID_INTCTRL") },
      { 0x09C2, new IoRegisterNotImplemented(*this, "SPID_STATUS", 0x80) },
      { 0x09C3, new IoRegisterNotImplemented(*this, "SPID_DATA") },

      { 0x0A00, new IoRegisterNotImplemented(*this, "TCE0_CTRLA") }, // Timer/Counter 0 on port E
      { 0x0A01, new IoRegisterNotImplemented(*this, "TCE0_CTRLB") },
      { 0x0A02, new IoRegisterNotImplemented(*this, "TCE0_CTRLC") },
      { 0x0A03, new IoRegisterNotImplemented(*this, "TCE0_CTRLD") },
      { 0x0A04, new IoRegisterNotImplemented(*this, "TCE0_CTRLE") },
      { 0x0A06, new IoRegisterNotImplemented(*this, "TCE0_INTCTRLA") },
      { 0x0A07, new IoRegisterNotImplemented(*this, "TCE0_INTCTRLB") },
      { 0x0A08, new IoRegisterNotImplemented(*this, "TCE0_CTRLFCLR") },
      { 0x0A09, new IoRegisterNotImplemented(*this, "TCE0_CTRLFSET") },
      { 0x0A0a, new IoRegisterNotImplemented(*this, "TCE0_CTRLGCLR") },
      { 0x0A0b, new IoRegisterNotImplemented(*this, "TCE0_CTRLGSET") },
      { 0x0A0c, new IoRegisterNotImplemented(*this, "TCE0_INTFLAGS") },
      { 0x0A0f, new IoRegisterNotImplemented(*this, "TCE0_TEMP") },
      { 0x0A20, new IoRegisterNotImplemented(*this, "TCE0_CNTL") },
      { 0x0A21, new IoRegisterNotImplemented(*this, "TCE0_CNTH") },
      { 0x0A26, new IoRegisterNotImplemented(*this, "TCE0_PERL") },
      { 0x0A27, new IoRegisterNotImplemented(*this, "TCE0_PERH") },
      { 0x0A28, new IoRegisterNotImplemented(*this, "TCE0_CCAL") },
      { 0x0A29, new IoRegisterNotImplemented(*this, "TCE0_CCAH") },
      { 0x0A2a, new IoRegisterNotImplemented(*this, "TCE0_CCBL") },
      { 0x0A2b, new IoRegisterNotImplemented(*this, "TCE0_CCBH") },
      { 0x0A2c, new IoRegisterNotImplemented(*this, "TCE0_CCCL") },
      { 0x0A2d, new IoRegisterNotImplemented(*this, "TCE0_CCCH") },
      { 0x0A2e, new IoRegisterNotImplemented(*this, "TCE0_CCDL") },
      { 0x0A2f, new IoRegisterNotImplemented(*this, "TCE0_CCDH") },
      { 0x0A36, new IoRegisterNotImplemented(*this, "TCE0_PERBUFL") },
      { 0x0A37, new IoRegisterNotImplemented(*this, "TCE0_PERBUFH") },
      { 0x0A38, new IoRegisterNotImplemented(*this, "TCE0_CCABUFL") },
      { 0x0A39, new IoRegisterNotImplemented(*this, "TCE0_CCABUFH") },
      { 0x0A3a, new IoRegisterNotImplemented(*this, "TCE0_CCBBUFL") },
      { 0x0A3b, new IoRegisterNotImplemented(*this, "TCE0_CCBBUFH") },
      { 0x0A3c, new IoRegisterNotImplemented(*this, "TCE0_CCCBUFL") },
      { 0x0A3d, new IoRegisterNotImplemented(*this, "TCE0_CCCBUFH") },
      { 0x0A3e, new IoRegisterNotImplemented(*this, "TCE0_CCDBUFL") },
      { 0x0A3f, new IoRegisterNotImplemented(*this, "TCE0_CCDBUFH") },

      { 0x0A80, new IoRegisterNotImplemented(*this, "AWEXE_CTRL") }, // Advanced Waveform Extension on port E
      { 0x0A82, new IoRegisterNotImplemented(*this, "AWEXE_FDEMASK") },
      { 0x0A83, new IoRegisterNotImplemented(*this, "AWEXE_FDCTRL") },
      { 0x0A84, new IoRegisterNotImplemented(*this, "AWEXE_STATUS") },
      { 0x0A86, new IoRegisterNotImplemented(*this, "AWEXE_DTBOTH") },
      { 0x0A87, new IoRegisterNotImplemented(*this, "AWEXE_DTBOTHBUF") },
      { 0x0A88, new IoRegisterNotImplemented(*this, "AWEXE_DTLS") },
      { 0x0A89, new IoRegisterNotImplemented(*this, "AWEXE_DTHS") },
      { 0x0A8a, new IoRegisterNotImplemented(*this, "AWEXE_DTLSBUF") },
      { 0x0A8b, new IoRegisterNotImplemented(*this, "AWEXE_DTHSBUF") },
      { 0x0A8c, new IoRegisterNotImplemented(*this, "AWEXE_OUTOVEN") },

      { 0x0A90, new IoRegisterNotImplemented(*this, "HIRESE_CTRLA") }, // High Resolution Extension on port E

      { 0x0AA0, new IoXmegaUsart::Data     (*this, _usartE0) }, // USART 0 on port E
      { 0x0AA1, new IoXmegaUsart::Status   (*this, _usartE0) },                     
      { 0x0AA3, new IoXmegaUsart::CtrlA    (*this, _usartE0) },                     
      { 0x0AA4, new IoXmegaUsart::CtrlB    (*this, _usartE0) },                     
      { 0x0AA5, new IoXmegaUsart::CtrlC    (*this, _usartE0) },                     
      { 0x0AA6, new IoXmegaUsart::BaudCtrlA(*this, _usartE0) },                     
      { 0x0AA7, new IoXmegaUsart::BaudCtrlB(*this, _usartE0) },                     

      // DAC, Fuse, Signature?
    } ;
    for (const auto &iIoReg: ioRegs)
    {
      _io[iIoReg.first] = iIoReg.second ;
    }
  }

  ATxmegaAU::~ATxmegaAU()
  {
  }

  uint8_t ATxmegaAU::UserSignature(uint32_t addr) const
  {
    return addr & 1 ? 0xaa : 0x55 ;
  }
  
  uint8_t ATxmegaAU::ProductionSignature(uint32_t addr) const
  {
    static const uint8_t sig[] =
      {
        0x10, 0x10, 0x10, 0x10,  0x10, 0xff, 0xff, 0xff,  0x42, 0x42, 0x42, 0x42,  0x42, 0x42, 0xff, 0xff,  
        0x13, 0xff, 0x13, 0x13,  0x13, 0x13, 0xff, 0xff,  0xff, 0xff, 0x14, 0x14,  0x14, 0xff, 0xff, 0xff,  
        0x15, 0x15, 0xff, 0xff,  0x16, 0x16, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0x17, 0xf7,  
        0xde, 0xad, 0xbe, 0xef,  0xca, 0xfe, 0xca, 0xfe,  0xff, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  
      } ;

    return (addr < sizeof(sig)) ? sig[addr] : 0xff ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////

  ATxmega128A4U::ATxmega128A4U() : ATxmegaAU("ATxmega128A4U", 0x20000/2, 0x2000, 0x800)
  {
    _pcIs22Bit = true ;
  }
  ATxmega128A4U::~ATxmega128A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega64A4U::ATxmega64A4U() : ATxmegaAU("ATxmega64A4U", 0x10000/2, 0x1000, 0x800)
  {
  }
  ATxmega64A4U::~ATxmega64A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega32A4U::ATxmega32A4U() : ATxmegaAU("ATxmega32A4U", 0x8000/2, 0x1000, 0x400)
  {
  }
  ATxmega32A4U::~ATxmega32A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega16A4U::ATxmega16A4U() : ATxmegaAU("ATxmega16A4U", 0x4000/2, 0x800, 0x400)
  {
  }
  ATxmega16A4U::~ATxmega16A4U()
  {
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
