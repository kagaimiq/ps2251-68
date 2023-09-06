# Some programs

## host tools

### runner.py

A script that loads code into PRAM and runs it, for it to work the chip should be in the
PRAM mode, which can be entered by not letting it boot from the NAND,
e.g. by shorting two data lines together on the NAND.

Usage: `runner.py --device <device path> [-p/--pramhdr] [-v/--vecred] <file>`

- `<device path>` is the path to the PRAM device e.g. `/dev/sg2`
- `-p/--pramhdr` will generate the `BtPramCd` header instead of using the first 512 bytes of the file.
- `-v/--vecred` will prepend a 128-byte block which contains jump instructions that redirect the respective interrupt vectors
  128 bytes ahead, as the code is loaded into address 0x4000 but the execution starts at 0x4080 instead, which is inconvenient.
- `<file>` is the file that you want to load

It depends on my own `scsiio` module, which you can currently grab from [jl-uboot-tool],
as I haven't made it into a proper package yet..

[jl-uboot-tool]: https://github.com/kagaimiq/jl-uboot-tool

## code for PS2251-68

### KonaNFI

This is the firmware that shows up as a USB device from which you can (really slowly) access the NAND flash
using some custom control commands that are transferred solely on the control endpoint, without any kind of DMA (on both sides).

In fact this came handy when I wanted to dump (later reuse and finally restore it back) a NAND flash from my Zoran-based camera, so this was actually useful in practice.

Extra stuff not available (yet): a quirky Linux kernel driver that makes this appear as an mtd device (partly the reason why I started to discover this chip, I just wanted to play with NANDs..)
