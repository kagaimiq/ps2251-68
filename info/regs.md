# Registers

## SFR

_(hai, i also mention the 8051 regs there!)_

- 0x80 = _P0_
- 0x81 = _SP_
- 0x82 = _DPL_
- 0x83 = _DPH_
- 0x87 = _PCON_
- 0x88 = _TCON_
- 0x89 = _TMID_
- 0x8A = _TL0_
- 0x8B = _TL1_
- 0x8C = _TH0_
- 0x8D = _TH1_
- 0x90 = _P1_
  * b0 = LED output (0 = on, 1 = off)
- 0x98 = _SCON_
- 0x99 = _SBUF_
- 0xA0 = _P2_
- 0xA8 = _IE_
- 0xB0 = _P3_
- 0xB8 = _IP_
- 0xBC = Interrupt vectors base address? (8..15)
- 0xD0 = _PSW_
- 0xE0 = _ACC_
- 0xF0 = _B_

## XFR

_(the addresses are specified as addresses in XDATA)_

### 0xF000-0xF0FF

- See [USB](usb.md#registers)

### 0xF100-0xF1FF

- 0xF122 = ..
  * b5 = ..
- 0xF123 = ..
  * b0~b3 = ..
- 0xF141 = ..
- 0xF148 = ..
  * b0 = R/B# pin state
  * b1 = DQS pin state

### 0xF200-0xF2FF

- See [NANDif](nandif.md#registers)
