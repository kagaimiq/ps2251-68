# NAND interface

- 8-bit NAND interface
- Up to 8 CS lines (but only first two is wired out of the chip)
- Usual NAND management stuff like ECC and randomizer, etc.

## Registers

_(the addresses are specified as addresses in XDATA)_

- 0xF203 = /
- 0xF204 = /
- 0xF205 = /
- 0xF206 = /
- 0xF207 = /
  * b0 = R/B# state
- 0xF208 = CS output value
  * b0 = CS0# output level
  * b1 = CS1# output level
  * etc.
- 0xF209 = something related with CS too?
- 0xF20A = /
- 0xF20B = /
- 0xF20C = /
- 0xF20D = /
- 0xF20E = /
- 0xF20F = /
- 0xF210 = /
- 0xF211 = Write address (ALE=1, CLE=0)
- 0xF213 = Write command (ALE=0, CLE=1)
- 0xF215 = Read/Write data (ALE=0, CLE=0)
- 0xF216 = /
- 0xF219 = /
- 0xF21B = /
- 0xF21C = /
- 0xF21D = /
- 0xF21E = /
- 0xF21F = /
- 0xF220 = /
- 0xF221 = /
- 0xF224 = /
- 0xF227 = /
- 0xF228 = /
- 0xF229 = /
- 0xF22A = /
- 0xF22B = /
- 0xF232 = /
- 0xF233 = /
- 0xF234 = /
- 0xF238 = /
- 0xF239 = /
- 0xF23A = /
- 0xF23B = /
- 0xF23C = /
- 0xF23F = /
- 0xF245 = /
- 0xF246 = /
- 0xF247 = /
- 0xF248 = /
- 0xF24D = /
- 0xF257 = /
- 0xF259 = /
- 0xF260 = /
- 0xF261 = /
- 0xF262 = /
- 0xF266 = /
- 0xF267 = /
- 0xF279 = /
- 0xF27A = /
- 0xF27B = /
- 0xF27C = /
- 0xF27E = /
- 0xF280 = /
- 0xF285 = /
- 0xF28E = /
- 0xF28F = /
