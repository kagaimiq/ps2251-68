# NAND interface

The second essential component that allows to access the memory we're storing the data on,
in this case the raw NAND flash.

## Registers

_(the addresses are specified as addresses in XDATA)_

- 0xF207 = Busy value?
  * b0 = R/B# state
- 0xF208 = CS output value
  * b0 = CS0# output level
  * b1 = CS1# output level
  * etc.
- 0xF209 = something related with CS too?
- 0xF211 = Send address (ALE=1, CLE=0)
- 0xF213 = Send command (ALE=0, CLE=1)
- 0xF215 = Send/receive data (ALE=0, CLE=0)
