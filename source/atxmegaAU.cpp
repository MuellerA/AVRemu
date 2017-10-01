////////////////////////////////////////////////////////////////////////////////
// atxmegaAU.cpp
// ATxmega128A4U / 64A4U / 32A4U / 16A4U
////////////////////////////////////////////////////////////////////////////////

#include "avr.h"
#include "instr.h"

namespace AVR
{
  ATxmegaAU::ATxmegaAU(std::size_t programSize, std::size_t dataSize, std::size_t eepromSize)
    : Mcu(programSize, 0x1000, 0x2000, dataSize, eepromSize)
  {
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

    std::vector<std::pair<uint32, std::string>> ioRegs
    {
      { 0x0000, "GPIOR0" }, // General Purpose IO Registers
      { 0x0001, "GPIOR1" },
      { 0x0002, "GPIOR2" },
      { 0x0003, "GPIOR3" },
      { 0x0004, "GPIOR4" },
      { 0x0005, "GPIOR5" },
      { 0x0006, "GPIOR6" },
      { 0x0007, "GPIOR7" },
      { 0x0008, "GPIOR8" },
      { 0x0009, "GPIOR9" },
      { 0x000a, "GPIOR10" },
      { 0x000b, "GPIOR11" },
      { 0x000c, "GPIOR12" },
      { 0x000d, "GPIOR13" },
      { 0x000e, "GPIOR14" },
      { 0x000f, "GPIOR15" },
        
      { 0x0010, "VPORT0_DIR" }, // Virtual Port 0
      { 0x0011, "VPORT0_OUT" },
      { 0x0012, "VPORT0_IN" },
      { 0x0013, "VPORT0_INTFLAGS" },
        
      { 0x0014, "VPORT1_DIR" }, // Virtual Port 1
      { 0x0015, "VPORT1_OUT" },
      { 0x0016, "VPORT1_IN" },
      { 0x0017, "VPORT1_INTFLAGS" },
        
      { 0x0018, "VPORT2_DIR" }, // Virtual Port 2
      { 0x0019, "VPORT2_OUT" },
      { 0x001a, "VPORT2_IN" },
      { 0x001b, "VPORT2_INTFLAGS" },
        
      { 0x001c, "VPORT3_DIR" }, // Virtual Port 3
      { 0x001d, "VPORT3_OUT" },
      { 0x001e, "VPORT3_IN" },
      { 0x001f, "VPORT3_INTFLAGS" },
                
      { 0x0034, "CPU_CCP" }, // CPU
      { 0x0038, "CPU_RAMPD" },
      { 0x0039, "CPU_RAMPX" },
      { 0x003a, "CPU_RAMPY" },
      { 0x003b, "CPU_RAMPZ" },
      { 0x003c, "CPU_EIND" },
      //{ 0x003d, "CPU_SPL" },
      //{ 0x003e, "CPU_SPH" },
      //{ 0x003f, "CPU_SREG" },
        
      { 0x0040, "CLK_CTRL" }, // Clock Control
      { 0x0042, "CLK_PSCTRL" },
      { 0x0043, "CLK_LOCK" },
      { 0x0044, "CLK_RTCCTRL" },
      { 0x0045, "CLK_USBSCTRL" },
        
      { 0x0048, "SLEEP_CTRL" }, // Sleep Controller
        
      { 0x0050, "OSC_CTRL" }, // Oscillator Control
      { 0x0051, "OSC_STATUS" },
      { 0x0052, "OSC_XOSCCTRL" },
      { 0x0053, "OSC_XOSCFAIL" },
      { 0x0054, "OSC_RC32KCAL" },
      { 0x0055, "OSC_PLLCTRL" },
      { 0x0056, "OSC_DFLLCTRL" },

      { 0x0060, "DFLLRC32M_CTRL" }, // DFLL for the 32MHz Internal RC Oscillator
      { 0x0062, "DFLLRC32M_CALA" },
      { 0x0063, "DFLLRC32M_CALB" },
      { 0x0065, "DFLLRC32M_COMP1" },
      { 0x0066, "DFLLRC32M_COMP2" },
        
      { 0x0068, "DFLLRC2M_CTRL" }, // DFLL for the 2MHz RC Oscillator
      { 0x006a, "DFLLRC2M_CALA" },
      { 0x006b, "DFLLRC2M_CALB" },
      { 0x006d, "DFLLRC2M_COMP1" },
      { 0x006e, "DFLLRC2M_COMP2" },
        
      { 0x0070, "PR_GEN" }, // Power Reduction
      { 0x0071, "PR_PA" },
      { 0x0072, "PR_PB" },
      { 0x0073, "PR_PC" },
      { 0x0074, "PR_PD" },
      { 0x0075, "PR_PE" },
      { 0x0076, "PR_PF" },

      { 0x0078, "RST_STATUS" }, // Reset Controller
      { 0x0079, "RST_CTRL" },
        
      { 0x0080, "WDT_CTRL" }, // Watch-Dog Timer
      { 0x0081, "WDT_WINCTRL" },
      { 0x0082, "WDT_STATUS" },
      { 0x0090, "MCU_DEVID0" }, // MCU Control
      { 0x0091, "MCU_DEVID1" },
      { 0x0092, "MCU_DEVID2" },
      { 0x0094, "MCU_JTAGUID" },
      { 0x0096, "MCU_MCUCR" },
      { 0x0097, "MCU_ANAINIT" },
      { 0x0098, "MCU_EVSYSLOCK" },
      { 0x0099, "MCU_AWEXLOCK" },

      { 0x00A0, "PMIC_STATUS" }, // Programmable Multilevel Interrupt Controller
      { 0x00A1, "PMIC_INTPRI" },
      { 0x00A2, "PMIC_CTRL" },

      { 0x00B0, "PORTCFG_MPCMASK" }, // Port Configuration
      { 0x00B2, "PORTCFG_VPCTRLA" },
      { 0x00B3, "PORTCFG_VPCTRLB" },
      { 0x00B4, "PORTCFG_CLKEVOUT" },
      { 0x00B5, "PORTCFG_EBIOUT" },
      { 0x00B6, "PORTCFG_EVCTRL" },

      { 0x00C0, "AES_CTRL" }, // AES Module
      { 0x00C1, "AES_STATUS" },
      { 0x00C2, "AES_STATE" },
      { 0x00C3, "AES_KEY" },
      { 0x00C4, "AES_INTCTRL" },

      { 0x00D0, "CRC_CTRL" }, // CRC Module
      { 0x00D1, "CRC_STATUS" },
      { 0x00D3, "CRC_DATAIN" },
      { 0x00D4, "CRC_CHECKSUM0" },
      { 0x00D5, "CRC_CHECKSUM1" },
      { 0x00D6, "CRC_CHECKSUM2" },
      { 0x00D7, "CRC_CHECKSUM3" },

      { 0x0100, "DMA_CTRL" }, // DMA Module
      { 0x0103, "DMA_INTFLAGS" },
      { 0x0104, "DMA_STATUS" },
      { 0x0106, "DMA_TEMPL" },
      { 0x0107, "DMA_TEMPH" },
      { 0x0110, "DMA_CH0_CTRLA" },
      { 0x0111, "DMA_CH0_CTRLB" },
      { 0x0112, "DMA_CH0_ADDCTRL" },
      { 0x0113, "DMA_CH0_TRIGSRC" },
      { 0x0114, "DMA_CH0_TRFCNTL" },
      { 0x0115, "DMA_CH0_TRFCNTH" },
      { 0x0116, "DMA_CH0_REPCNT" },
      { 0x0118, "DMA_CH0_SRCADDR0" },
      { 0x0119, "DMA_CH0_SRCADDR1" },
      { 0x011a, "DMA_CH0_SRCADDR2" },
      { 0x011c, "DMA_CH0_DESTADDR0" },
      { 0x011d, "DMA_CH0_DESTADDR1" },
      { 0x011e, "DMA_CH0_DESTADDR2" },
      { 0x0120, "DMA_CH1_CTRLA" },
      { 0x0121, "DMA_CH1_CTRLB" },
      { 0x0122, "DMA_CH1_ADDCTRL" },
      { 0x0123, "DMA_CH1_TRIGSRC" },
      { 0x0124, "DMA_CH1_TRFCNTL" },
      { 0x0125, "DMA_CH1_TRFCNTH" },
      { 0x0126, "DMA_CH1_REPCNT" },
      { 0x0128, "DMA_CH1_SRCADDR0" },
      { 0x0129, "DMA_CH1_SRCADDR1" },
      { 0x012a, "DMA_CH1_SRCADDR2" },
      { 0x012c, "DMA_CH1_DESTADDR0" },
      { 0x012d, "DMA_CH1_DESTADDR1" },
      { 0x012e, "DMA_CH1_DESTADDR2" },
      { 0x0130, "DMA_CH2_CTRLA" },
      { 0x0131, "DMA_CH2_CTRLB" },
      { 0x0132, "DMA_CH2_ADDCTRL" },
      { 0x0133, "DMA_CH2_TRIGSRC" },
      { 0x0134, "DMA_CH2_TRFCNTL" },
      { 0x0135, "DMA_CH2_TRFCNTH" },
      { 0x0136, "DMA_CH2_REPCNT" },
      { 0x0138, "DMA_CH2_SRCADDR0" },
      { 0x0139, "DMA_CH2_SRCADDR1" },
      { 0x013a, "DMA_CH2_SRCADDR2" },
      { 0x013c, "DMA_CH2_DESTADDR0" },
      { 0x013d, "DMA_CH2_DESTADDR1" },
      { 0x013e, "DMA_CH2_DESTADDR2" },
      { 0x0140, "DMA_CH3_CTRLA" },
      { 0x0141, "DMA_CH3_CTRLB" },
      { 0x0142, "DMA_CH3_ADDCTRL" },
      { 0x0143, "DMA_CH3_TRIGSRC" },
      { 0x0144, "DMA_CH3_TRFCNTL" },
      { 0x0145, "DMA_CH3_TRFCNTH" },
      { 0x0146, "DMA_CH3_REPCNT" },
      { 0x0148, "DMA_CH3_SRCADDR0" },
      { 0x0149, "DMA_CH3_SRCADDR1" },
      { 0x014a, "DMA_CH3_SRCADDR2" },
      { 0x014c, "DMA_CH3_DESTADDR0" },
      { 0x014d, "DMA_CH3_DESTADDR1" },
      { 0x014e, "DMA_CH3_DESTADDR2" },

      { 0x0180, "EVSYS_CH0MUX" }, // Event System
      { 0x0181, "EVSYS_CH1MUX" },
      { 0x0182, "EVSYS_CH2MUX" },
      { 0x0183, "EVSYS_CH3MUX" },
      { 0x0184, "EVSYS_CH4MUX" },
      { 0x0185, "EVSYS_CH5MUX" },
      { 0x0186, "EVSYS_CH6MUX" },
      { 0x0187, "EVSYS_CH7MUX" },
      { 0x0188, "EVSYS_CH0CTRL" },
      { 0x0189, "EVSYS_CH1CTRL" },
      { 0x018a, "EVSYS_CH2CTRL" },
      { 0x018b, "EVSYS_CH3CTRL" },
      { 0x018c, "EVSYS_CH4CTRL" },
      { 0x018d, "EVSYS_CH5CTRL" },
      { 0x018e, "EVSYS_CH6CTRL" },
      { 0x018f, "EVSYS_CH7CTRL" },
      { 0x0190, "EVSYS_STROBE" },
      { 0x0191, "EVSYS_DATA" },

      { 0x01C0, "NVM_ADDR0" }, // Non Volatile Memory (NVM) Controller
      { 0x01C1, "NVM_ADDR1" },
      { 0x01C2, "NVM_ADDR2" },
      { 0x01C4, "NVM_DATA0" },
      { 0x01C5, "NVM_DATA1" },
      { 0x01C6, "NVM_DATA2" },
      { 0x01Ca, "NVM_CMD" },
      { 0x01Cb, "NVM_CTRLA" },
      { 0x01Cc, "NVM_CTRLB" },
      { 0x01Cd, "NVM_INTCTRL" },
      { 0x01Cf, "NVM_STATUS" },
      { 0x01D0, "NVM_LOCKBITS" },

      { 0x0200, "ADCA_CTRLA" }, // Analog to Digital Converter on port A
      { 0x0201, "ADCA_CTRLB" },
      { 0x0202, "ADCA_REFCTRL" },
      { 0x0203, "ADCA_EVCTRL" },
      { 0x0204, "ADCA_PRESCALER" },
      { 0x0205, "ADCA_INTFLAGS" },
      { 0x0206, "ADCA_TEMP" },
      { 0x020c, "ADCA_CALL" },
      { 0x020d, "ADCA_CALH" },
      { 0x0210, "ADCA_CH0RESL" },
      { 0x0211, "ADCA_CH0RESH" },
      { 0x0212, "ADCA_CH1RESL" },
      { 0x0213, "ADCA_CH1RESH" },
      { 0x0214, "ADCA_CH2RESL" },
      { 0x0215, "ADCA_CH2RESH" },
      { 0x0216, "ADCA_CH3RESL" },
      { 0x0217, "ADCA_CH3RESH" },
      { 0x0218, "ADCA_CMPL" },
      { 0x0219, "ADCA_CMPH" },
      { 0x0220, "ADCA_CH0_CTRL" },
      { 0x0221, "ADCA_CH0_MUXCTRL" },
      { 0x0222, "ADCA_CH0_INTCTRL" },
      { 0x0223, "ADCA_CH0_INTFLAGS" },
      { 0x0224, "ADCA_CH0_RESL" },
      { 0x0225, "ADCA_CH0_RESH" },
      { 0x0226, "ADCA_CH0_SCAN" },
      { 0x0228, "ADCA_CH1_CTRL" },
      { 0x0229, "ADCA_CH1_MUXCTRL" },
      { 0x022a, "ADCA_CH1_INTCTRL" },
      { 0x022b, "ADCA_CH1_INTFLAGS" },
      { 0x022c, "ADCA_CH1_RESL" },
      { 0x022d, "ADCA_CH1_RESH" },
      { 0x022e, "ADCA_CH1_SCAN" },
      { 0x0230, "ADCA_CH2_CTRL" },
      { 0x0231, "ADCA_CH2_MUXCTRL" },
      { 0x0232, "ADCA_CH2_INTCTRL" },
      { 0x0233, "ADCA_CH2_INTFLAGS" },
      { 0x0234, "ADCA_CH2_RESL" },
      { 0x0235, "ADCA_CH2_RESH" },
      { 0x0236, "ADCA_CH2_SCAN" },
      { 0x0238, "ADCA_CH3_CTRL" },
      { 0x0239, "ADCA_CH3_MUXCTRL" },
      { 0x023a, "ADCA_CH3_INTCTRL" },
      { 0x023b, "ADCA_CH3_INTFLAGS" },
      { 0x023c, "ADCA_CH3_RESL" },
      { 0x023d, "ADCA_CH3_RESH" },
      { 0x023e, "ADCA_CH3_SCAN" },
        
      { 0x0380, "ACA_AC0CTRL" }, // Analog Comparator pair on port A
      { 0x0381, "ACA_AC1CTRL" },
      { 0x0382, "ACA_AC0MUXCTRL" },
      { 0x0383, "ACA_AC1MUXCTRL" },
      { 0x0384, "ACA_CTRLA" },
      { 0x0385, "ACA_CTRLB" },
      { 0x0386, "ACA_WINCTRL" },
      { 0x0387, "ACA_STATUS" },
      { 0x0388, "ACA_CURRCTRL" },
      { 0x0389, "ACA_CURRCALIB" },

      { 0x0400, "RTC_CTRL" }, // Real Time Counter
      { 0x0401, "RTC_STATUS" },
      { 0x0402, "RTC_INTCTRL" },
      { 0x0403, "RTC_INTFLAGS" },
      { 0x0404, "RTC_TEMP" },
      { 0x0408, "RTC_CNTL" },
      { 0x0409, "RTC_CNTH" },
      { 0x040a, "RTC_PERL" },
      { 0x040b, "RTC_PERH" },
      { 0x040c, "RTC_COMPL" },
      { 0x040d, "RTC_COMPH" },

      { 0x0480, "TWIC_CTRL" }, // Two Wire Interface on port C
      { 0x0481, "TWIC_MASTER_CTRLA" },
      { 0x0482, "TWIC_MASTER_CTRLB" },
      { 0x0483, "TWIC_MASTER_CTRLC" },
      { 0x0484, "TWIC_MASTER_STATUS" },
      { 0x0485, "TWIC_MASTER_BAUD" },
      { 0x0486, "TWIC_MASTER_ADDR" },
      { 0x0487, "TWIC_MASTER_DATA" },
      { 0x0488, "TWIC_SLAVE_CTRLA" },
      { 0x0489, "TWIC_SLAVE_CTRLB" },
      { 0x048a, "TWIC_SLAVE_STATUS" },
      { 0x048b, "TWIC_SLAVE_ADDR" },
      { 0x048c, "TWIC_SLAVE_DATA" },
      { 0x048d, "TWIC_SLAVE_ADDRMASK" },

      { 0x04a0, "TWIE_CTRL" }, // Two Wire Interface on port E
      { 0x04a1, "TWIE_MASTER_CTRLA" },
      { 0x04a2, "TWIE_MASTER_CTRLB" },
      { 0x04a3, "TWIE_MASTER_CTRLC" },
      { 0x04a4, "TWIE_MASTER_STATUS" },
      { 0x04a5, "TWIE_MASTER_BAUD" },
      { 0x04a6, "TWIE_MASTER_ADDR" },
      { 0x04a7, "TWIE_MASTER_DATA" },
      { 0x04a8, "TWIE_SLAVE_CTRLA" },
      { 0x04a9, "TWIE_SLAVE_CTRLB" },
      { 0x04aa, "TWIE_SLAVE_STATUS" },
      { 0x04ab, "TWIE_SLAVE_ADDR" },
      { 0x04ac, "TWIE_SLAVE_DATA" },
      { 0x04ad, "TWIE_SLAVE_ADDRMASK" },

      { 0x04C0, "USB_FRAMENUML" }, // Universal Serial Bus Interface
      { 0x04C1, "USB_FRAMENUMH" },

      { 0x0600, "PORTA_DIR" }, // Port A
      { 0x0601, "PORTA_DIRSET" },
      { 0x0602, "PORTA_DIRCLR" },
      { 0x0603, "PORTA_DIRTGL" },
      { 0x0604, "PORTA_OUT" },
      { 0x0605, "PORTA_OUTSET" },
      { 0x0606, "PORTA_OUTCLR" },
      { 0x0607, "PORTA_OUTTGL" },
      { 0x0608, "PORTA_IN" },
      { 0x0609, "PORTA_INTCTRL" },
      { 0x060a, "PORTA_INT0MASK" },
      { 0x060b, "PORTA_INT1MASK" },
      { 0x060c, "PORTA_INTFLAGS" },
      { 0x060e, "PORTA_REMAP" },
      { 0x0610, "PORTA_PIN0CTRL" },
      { 0x0611, "PORTA_PIN1CTRL" },
      { 0x0612, "PORTA_PIN2CTRL" },
      { 0x0613, "PORTA_PIN3CTRL" },
      { 0x0614, "PORTA_PIN4CTRL" },
      { 0x0615, "PORTA_PIN5CTRL" },
      { 0x0616, "PORTA_PIN6CTRL" },
      { 0x0617, "PORTA_PIN7CTRL" },

      { 0x0620, "PORTB_DIR" }, // Port B
      { 0x0621, "PORTB_DIRSET" },
      { 0x0622, "PORTB_DIRCLR" },
      { 0x0623, "PORTB_DIRTGL" },
      { 0x0624, "PORTB_OUT" },
      { 0x0625, "PORTB_OUTSET" },
      { 0x0626, "PORTB_OUTCLR" },
      { 0x0627, "PORTB_OUTTGL" },
      { 0x0628, "PORTB_IN" },
      { 0x0629, "PORTB_INTCTRL" },
      { 0x062a, "PORTB_INT0MASK" },
      { 0x062b, "PORTB_INT1MASK" },
      { 0x062c, "PORTB_INTFLAGS" },
      { 0x062e, "PORTB_REMAP" },
      { 0x0630, "PORTB_PIN0CTRL" },
      { 0x0631, "PORTB_PIN1CTRL" },
      { 0x0632, "PORTB_PIN2CTRL" },
      { 0x0633, "PORTB_PIN3CTRL" },
      { 0x0634, "PORTB_PIN4CTRL" },
      { 0x0635, "PORTB_PIN5CTRL" },
      { 0x0636, "PORTB_PIN6CTRL" },
      { 0x0637, "PORTB_PIN7CTRL" },

      { 0x0640, "PORTC_DIR" }, // Port C
      { 0x0641, "PORTC_DIRSET" },
      { 0x0642, "PORTC_DIRCLR" },
      { 0x0643, "PORTC_DIRTGL" },
      { 0x0644, "PORTC_OUT" },
      { 0x0645, "PORTC_OUTSET" },
      { 0x0646, "PORTC_OUTCLR" },
      { 0x0647, "PORTC_OUTTGL" },
      { 0x0648, "PORTC_IN" },
      { 0x0649, "PORTC_INTCTRL" },
      { 0x064a, "PORTC_INT0MASK" },
      { 0x064b, "PORTC_INT1MASK" },
      { 0x064c, "PORTC_INTFLAGS" },
      { 0x064e, "PORTC_REMAP" },
      { 0x0650, "PORTC_PIN0CTRL" },
      { 0x0651, "PORTC_PIN1CTRL" },
      { 0x0652, "PORTC_PIN2CTRL" },
      { 0x0653, "PORTC_PIN3CTRL" },
      { 0x0654, "PORTC_PIN4CTRL" },
      { 0x0655, "PORTC_PIN5CTRL" },
      { 0x0656, "PORTC_PIN6CTRL" },
      { 0x0657, "PORTC_PIN7CTRL" },

      { 0x0660, "PORTD_DIR" }, // Port D
      { 0x0661, "PORTD_DIRSET" },
      { 0x0662, "PORTD_DIRCLR" },
      { 0x0663, "PORTD_DIRTGL" },
      { 0x0664, "PORTD_OUT" },
      { 0x0665, "PORTD_OUTSET" },
      { 0x0666, "PORTD_OUTCLR" },
      { 0x0667, "PORTD_OUTTGL" },
      { 0x0668, "PORTD_IN" },
      { 0x0669, "PORTD_INTCTRL" },
      { 0x066a, "PORTD_INT0MASK" },
      { 0x066b, "PORTD_INT1MASK" },
      { 0x066c, "PORTD_INTFLAGS" },
      { 0x066e, "PORTD_REMAP" },
      { 0x0670, "PORTD_PIN0CTRL" },
      { 0x0671, "PORTD_PIN1CTRL" },
      { 0x0672, "PORTD_PIN2CTRL" },
      { 0x0673, "PORTD_PIN3CTRL" },
      { 0x0674, "PORTD_PIN4CTRL" },
      { 0x0675, "PORTD_PIN5CTRL" },
      { 0x0676, "PORTD_PIN6CTRL" },
      { 0x0677, "PORTD_PIN7CTRL" },

      { 0x0680, "PORTE_DIR" }, // Port E
      { 0x0681, "PORTE_DIRSET" },
      { 0x0682, "PORTE_DIRCLR" },
      { 0x0683, "PORTE_DIRTGL" },
      { 0x0684, "PORTE_OUT" },
      { 0x0685, "PORTE_OUTSET" },
      { 0x0686, "PORTE_OUTCLR" },
      { 0x0687, "PORTE_OUTTGL" },
      { 0x0688, "PORTE_IN" },
      { 0x0689, "PORTE_INTCTRL" },
      { 0x068a, "PORTE_INT0MASK" },
      { 0x068b, "PORTE_INT1MASK" },
      { 0x068c, "PORTE_INTFLAGS" },
      { 0x068e, "PORTE_REMAP" },
      { 0x0690, "PORTE_PIN0CTRL" },
      { 0x0691, "PORTE_PIN1CTRL" },
      { 0x0692, "PORTE_PIN2CTRL" },
      { 0x0693, "PORTE_PIN3CTRL" },
      { 0x0694, "PORTE_PIN4CTRL" },
      { 0x0695, "PORTE_PIN5CTRL" },
      { 0x0696, "PORTE_PIN6CTRL" },
      { 0x0697, "PORTE_PIN7CTRL" },

      { 0x07E0, "PORTR_DIR" }, // Port R
      { 0x07E1, "PORTR_DIRSET" },
      { 0x07E2, "PORTR_DIRCLR" },
      { 0x07E3, "PORTR_DIRTGL" },
      { 0x07E4, "PORTR_OUT" },
      { 0x07E5, "PORTR_OUTSET" },
      { 0x07E6, "PORTR_OUTCLR" },
      { 0x07E7, "PORTR_OUTTGL" },
      { 0x07E8, "PORTR_IN" },
      { 0x07E9, "PORTR_INTCTRL" },
      { 0x07Ea, "PORTR_INT0MASK" },
      { 0x07Eb, "PORTR_INT1MASK" },
      { 0x07Ec, "PORTR_INTFLAGS" },
      { 0x07Ee, "PORTR_REMAP" },
      { 0x07F0, "PORTR_PIN0CTRL" },
      { 0x07F1, "PORTR_PIN1CTRL" },
      { 0x07F2, "PORTR_PIN2CTRL" },
      { 0x07F3, "PORTR_PIN3CTRL" },
      { 0x07F4, "PORTR_PIN4CTRL" },
      { 0x07F5, "PORTR_PIN5CTRL" },
      { 0x07F6, "PORTR_PIN6CTRL" },
      { 0x07F7, "PORTR_PIN7CTRL" },

      { 0x0800, "TCC0_CTRLA" }, // Timer/Counter 0 on port C
      { 0x0801, "TCC0_CTRLB" },
      { 0x0802, "TCC0_CTRLC" },
      { 0x0803, "TCC0_CTRLD" },
      { 0x0804, "TCC0_CTRLE" },
      { 0x0806, "TCC0_INTCTRLA" },
      { 0x0807, "TCC0_INTCTRLB" },
      { 0x0808, "TCC0_CTRLFCLR" },
      { 0x0809, "TCC0_CTRLFSET" },
      { 0x080a, "TCC0_CTRLGCLR" },
      { 0x080b, "TCC0_CTRLGSET" },
      { 0x080c, "TCC0_INTFLAGS" },
      { 0x080f, "TCC0_TEMP" },
      { 0x0820, "TCC0_CNTL" },
      { 0x0821, "TCC0_CNTH" },
      { 0x0826, "TCC0_PERL" },
      { 0x0827, "TCC0_PERH" },
      { 0x0828, "TCC0_CCAL" },
      { 0x0829, "TCC0_CCAH" },
      { 0x082a, "TCC0_CCBL" },
      { 0x082b, "TCC0_CCBH" },
      { 0x082c, "TCC0_CCCL" },
      { 0x082d, "TCC0_CCCH" },
      { 0x082e, "TCC0_CCDL" },
      { 0x082f, "TCC0_CCDH" },
      { 0x0836, "TCC0_PERBUFL" },
      { 0x0837, "TCC0_PERBUFH" },
      { 0x0838, "TCC0_CCABUFL" },
      { 0x0839, "TCC0_CCABUFH" },
      { 0x083a, "TCC0_CCBBUFL" },
      { 0x083b, "TCC0_CCBBUFH" },
      { 0x083c, "TCC0_CCCBUFL" },
      { 0x083d, "TCC0_CCCBUFH" },
      { 0x083e, "TCC0_CCDBUFL" },
      { 0x083f, "TCC0_CCDBUFH" },

      { 0x0840, "TCC1_CTRLA" }, // Timer/Counter 1 on port C
      { 0x0841, "TCC1_CTRLB" },
      { 0x0842, "TCC1_CTRLC" },
      { 0x0843, "TCC1_CTRLD" },
      { 0x0844, "TCC1_CTRLE" },
      { 0x0846, "TCC1_INTCTRLA" },
      { 0x0847, "TCC1_INTCTRLB" },
      { 0x0848, "TCC1_CTRLFCLR" },
      { 0x0849, "TCC1_CTRLFSET" },
      { 0x084a, "TCC1_CTRLGCLR" },
      { 0x084b, "TCC1_CTRLGSET" },
      { 0x084c, "TCC1_INTFLAGS" },
      { 0x084f, "TCC1_TEMP" },
      { 0x0860, "TCC1_CNTL" },
      { 0x0861, "TCC1_CNTH" },
      { 0x0866, "TCC1_PERL" },
      { 0x0867, "TCC1_PERH" },
      { 0x0868, "TCC1_CCAL" },
      { 0x0869, "TCC1_CCAH" },
      { 0x086a, "TCC1_CCBL" },
      { 0x086b, "TCC1_CCBH" },
      { 0x086c, "TCC1_CCCL" },
      { 0x086d, "TCC1_CCCH" },
      { 0x086e, "TCC1_CCDL" },
      { 0x086f, "TCC1_CCDH" },
      { 0x0876, "TCC1_PERBUFL" },
      { 0x0877, "TCC1_PERBUFH" },
      { 0x0878, "TCC1_CCABUFL" },
      { 0x0879, "TCC1_CCABUFH" },
      { 0x087a, "TCC1_CCBBUFL" },
      { 0x087b, "TCC1_CCBBUFH" },
      { 0x087c, "TCC1_CCCBUFL" },
      { 0x087d, "TCC1_CCCBUFH" },
      { 0x087e, "TCC1_CCDBUFL" },
      { 0x087f, "TCC1_CCDBUFH" },

      { 0x0880, "AWEXC_CTRL" }, // Advanced Waveform Extension on port C
      { 0x0882, "AWEXC_FDEMASK" },
      { 0x0883, "AWEXC_FDCTRL" },
      { 0x0884, "AWEXC_STATUS" },
      { 0x0886, "AWEXC_DTBOTH" },
      { 0x0887, "AWEXC_DTBOTHBUF" },
      { 0x0888, "AWEXC_DTLS" },
      { 0x0889, "AWEXC_DTHS" },
      { 0x088a, "AWEXC_DTLSBUF" },
      { 0x088b, "AWEXC_DTHSBUF" },
      { 0x088c, "AWEXC_OUTOVEN" },

      { 0x0890, "HIRESC_CTRLA" }, // High Resolution Extension on port C

      { 0x08A0, "USARTC0_DATA" }, // USART 0 on port C
      { 0x08A1, "USARTC0_STATUS" },
      { 0x08A3, "USARTC0_CTRLA" },
      { 0x08A4, "USARTC0_CTRLB" },
      { 0x08A5, "USARTC0_CTRLC" },
      { 0x08A6, "USARTC0_BAUDCTRLA" },
      { 0x08A7, "USARTC0_BAUDCTRLB" },

      { 0x08B0, "USARTC1_DATA" }, // USART 1 on port C
      { 0x08B1, "USARTC1_STATUS" },
      { 0x08B3, "USARTC1_CTRLA" },
      { 0x08B4, "USARTC1_CTRLB" },
      { 0x08B5, "USARTC1_CTRLC" },
      { 0x08B6, "USARTC1_BAUDCTRLA" },
      { 0x08B7, "USARTC1_BAUDCTRLB" },

      { 0x08C0, "SPIC_CTRL" }, // Serial Peripheral Interface on port C
      { 0x08C1, "SPIC_INTCTRL" },
      { 0x08C2, "SPIC_STATUS" },
      { 0x08C3, "SPIC_DATA" },

      { 0x08F8, "IRCOM_CTRL" }, // Infrared Communication Module
      { 0x08F9, "IRCOM_TXPLCTRL" },
      { 0x08Fa, "IRCOM_RXPLCTRL" },

      { 0x0900, "TCD0_CTRLA" }, // Timer/Counter 0 on port D
      { 0x0901, "TCD0_CTRLB" },
      { 0x0902, "TCD0_CTRLC" },
      { 0x0903, "TCD0_CTRLD" },
      { 0x0904, "TCD0_CTRLE" },
      { 0x0906, "TCD0_INTCTRLA" },
      { 0x0907, "TCD0_INTCTRLB" },
      { 0x0908, "TCD0_CTRLFCLR" },
      { 0x0909, "TCD0_CTRLFSET" },
      { 0x090a, "TCD0_CTRLGCLR" },
      { 0x090b, "TCD0_CTRLGSET" },
      { 0x090c, "TCD0_INTFLAGS" },
      { 0x090f, "TCD0_TEMP" },
      { 0x0920, "TCD0_CNTL" },
      { 0x0921, "TCD0_CNTH" },
      { 0x0926, "TCD0_PERL" },
      { 0x0927, "TCD0_PERH" },
      { 0x0928, "TCD0_CCAL" },
      { 0x0929, "TCD0_CCAH" },
      { 0x092a, "TCD0_CCBL" },
      { 0x092b, "TCD0_CCBH" },
      { 0x092c, "TCD0_CCCL" },
      { 0x092d, "TCD0_CCCH" },
      { 0x092e, "TCD0_CCDL" },
      { 0x092f, "TCD0_CCDH" },
      { 0x0936, "TCD0_PERBUFL" },
      { 0x0937, "TCD0_PERBUFH" },
      { 0x0938, "TCD0_CCABUFL" },
      { 0x0939, "TCD0_CCABUFH" },
      { 0x093a, "TCD0_CCBBUFL" },
      { 0x093b, "TCD0_CCBBUFH" },
      { 0x093c, "TCD0_CCCBUFL" },
      { 0x093d, "TCD0_CCCBUFH" },
      { 0x093e, "TCD0_CCDBUFL" },
      { 0x093f, "TCD0_CCDBUFH" },

      { 0x0940, "TCD1_CTRLA" }, // Timer/Counter 1 on port D
      { 0x0941, "TCD1_CTRLB" },
      { 0x0942, "TCD1_CTRLC" },
      { 0x0943, "TCD1_CTRLD" },
      { 0x0944, "TCD1_CTRLE" },
      { 0x0946, "TCD1_INTCTRLA" },
      { 0x0947, "TCD1_INTCTRLB" },
      { 0x0948, "TCD1_CTRLFCLR" },
      { 0x0949, "TCD1_CTRLFSET" },
      { 0x094a, "TCD1_CTRLGCLR" },
      { 0x094b, "TCD1_CTRLGSET" },
      { 0x094c, "TCD1_INTFLAGS" },
      { 0x094f, "TCD1_TEMP" },
      { 0x0960, "TCD1_CNTL" },
      { 0x0961, "TCD1_CNTH" },
      { 0x0966, "TCD1_PERL" },
      { 0x0967, "TCD1_PERH" },
      { 0x0968, "TCD1_CCAL" },
      { 0x0969, "TCD1_CCAH" },
      { 0x096a, "TCD1_CCBL" },
      { 0x096b, "TCD1_CCBH" },
      { 0x096c, "TCD1_CCCL" },
      { 0x096d, "TCD1_CCCH" },
      { 0x096e, "TCD1_CCDL" },
      { 0x096f, "TCD1_CCDH" },
      { 0x0976, "TCD1_PERBUFL" },
      { 0x0977, "TCD1_PERBUFH" },
      { 0x0978, "TCD1_CCABUFL" },
      { 0x0979, "TCD1_CCABUFH" },
      { 0x097a, "TCD1_CCBBUFL" },
      { 0x097b, "TCD1_CCBBUFH" },
      { 0x097c, "TCD1_CCCBUFL" },
      { 0x097d, "TCD1_CCCBUFH" },
      { 0x097e, "TCD1_CCDBUFL" },
      { 0x097f, "TCD1_CCDBUFH" },

      { 0x0990, "HIRESD_CTRLA" }, // High Resolution Extension on port D

      { 0x09A0, "USARTD0_DATA" }, // USART 0 on port D
      { 0x09A1, "USARTD0_STATUS" },
      { 0x09A3, "USARTD0_CTRLA" },
      { 0x09A4, "USARTD0_CTRLB" },
      { 0x09A5, "USARTD0_CTRLC" },
      { 0x09A6, "USARTD0_BAUDCTRLA" },
      { 0x09A7, "USARTD0_BAUDCTRLB" },

      { 0x09B0, "USARTD1_DATA" }, // USART 1 on port D
      { 0x09B1, "USARTD1_STATUS" },
      { 0x09B3, "USARTD1_CTRLA" },
      { 0x09B4, "USARTD1_CTRLB" },
      { 0x09B5, "USARTD1_CTRLC" },
      { 0x09B6, "USARTD1_BAUDCTRLA" },
      { 0x09B7, "USARTD1_BAUDCTRLB" },

      { 0x09C0, "SPID_CTRL" }, // Serial Peripheral Interface on port D
      { 0x09C1, "SPID_INTCTRL" },
      { 0x09C2, "SPID_STATUS" },
      { 0x09C3, "SPID_DATA" },

      { 0x0A00, "TCE0_CTRLA" }, // Timer/Counter 0 on port E
      { 0x0A01, "TCE0_CTRLB" },
      { 0x0A02, "TCE0_CTRLC" },
      { 0x0A03, "TCE0_CTRLD" },
      { 0x0A04, "TCE0_CTRLE" },
      { 0x0A06, "TCE0_INTCTRLA" },
      { 0x0A07, "TCE0_INTCTRLB" },
      { 0x0A08, "TCE0_CTRLFCLR" },
      { 0x0A09, "TCE0_CTRLFSET" },
      { 0x0A0a, "TCE0_CTRLGCLR" },
      { 0x0A0b, "TCE0_CTRLGSET" },
      { 0x0A0c, "TCE0_INTFLAGS" },
      { 0x0A0f, "TCE0_TEMP" },
      { 0x0A20, "TCE0_CNTL" },
      { 0x0A21, "TCE0_CNTH" },
      { 0x0A26, "TCE0_PERL" },
      { 0x0A27, "TCE0_PERH" },
      { 0x0A28, "TCE0_CCAL" },
      { 0x0A29, "TCE0_CCAH" },
      { 0x0A2a, "TCE0_CCBL" },
      { 0x0A2b, "TCE0_CCBH" },
      { 0x0A2c, "TCE0_CCCL" },
      { 0x0A2d, "TCE0_CCCH" },
      { 0x0A2e, "TCE0_CCDL" },
      { 0x0A2f, "TCE0_CCDH" },
      { 0x0A36, "TCE0_PERBUFL" },
      { 0x0A37, "TCE0_PERBUFH" },
      { 0x0A38, "TCE0_CCABUFL" },
      { 0x0A39, "TCE0_CCABUFH" },
      { 0x0A3a, "TCE0_CCBBUFL" },
      { 0x0A3b, "TCE0_CCBBUFH" },
      { 0x0A3c, "TCE0_CCCBUFL" },
      { 0x0A3d, "TCE0_CCCBUFH" },
      { 0x0A3e, "TCE0_CCDBUFL" },
      { 0x0A3f, "TCE0_CCDBUFH" },

      { 0x0A80, "AWEXE_CTRL" }, // Advanced Waveform Extension on port E
      { 0x0A82, "AWEXE_FDEMASK" },
      { 0x0A83, "AWEXE_FDCTRL" },
      { 0x0A84, "AWEXE_STATUS" },
      { 0x0A86, "AWEXE_DTBOTH" },
      { 0x0A87, "AWEXE_DTBOTHBUF" },
      { 0x0A88, "AWEXE_DTLS" },
      { 0x0A89, "AWEXE_DTHS" },
      { 0x0A8a, "AWEXE_DTLSBUF" },
      { 0x0A8b, "AWEXE_DTHSBUF" },
      { 0x0A8c, "AWEXE_OUTOVEN" },

      { 0x0A90, "HIRESE_CTRLA" }, // High Resolution Extension on port E

      { 0x0AA0, "USARTE0_DATA" }, // USART 0 on port E
      { 0x0AA1, "USARTE0_STATUS" },
      { 0x0AA3, "USARTE0_CTRLA" },
      { 0x0AA4, "USARTE0_CTRLB" },
      { 0x0AA5, "USARTE0_CTRLC" },
      { 0x0AA6, "USARTE0_BAUDCTRLA" },
      { 0x0AA7, "USARTE0_BAUDCTRLB" },

      // DAC, Fuse, Signature?
    } ;
    for (const auto &iIoReg: ioRegs)
    {
      _io[iIoReg.first-0x20] = new IoRegisterNotImplemented(iIoReg.second) ;
    }
    _io[0x3f] = new IoSREG::SREG(_sreg) ;
    _io[0x3e] = new IoSP::SPH(_sp) ;
    _io[0x3d] = new IoSP::SPL(_sp) ;
  }

  ATxmegaAU::~ATxmegaAU()
  {
  }

  void ATxmegaAU::PushPC()
  {
    Push(_pc >> 16) ;
    Push(_pc >>  8) ;
    Push(_pc >>  0) ;
  }

  void ATxmegaAU::PopPC()
  {
    _pc = (Pop() << 0) | (Pop() << 8) | (Pop() << 16) ;
  }
  
  ////////////////////////////////////////////////////////////////////////////////

  ATxmega128A4U::ATxmega128A4U() : ATxmegaAU(0x20000/2, 0x2000, 0x800)
  {
  }
  ATxmega128A4U::~ATxmega128A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega64A4U::ATxmega64A4U() : ATxmegaAU(0x10000/2, 0x1000, 0x800)
  {
  }
  ATxmega64A4U::~ATxmega64A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega32A4U::ATxmega32A4U() : ATxmegaAU(0x8000/2, 0x1000, 0x400)
  {
  }
  ATxmega32A4U::~ATxmega32A4U()
  {
  }

  ////////////////////////////////////////////////////////////////////////////////

  ATxmega16A4U::ATxmega16A4U() : ATxmegaAU(0x4000/2, 0x800, 0x400)
  {
  }
  ATxmega16A4U::~ATxmega16A4U()
  {
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
