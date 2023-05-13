# USB device

The first essential component is the USB device controller, that allows the stick to recognise
as an mass storage device (or practically anything within some limits..)

## Quirks

- Control requests:
  * SET_ADDRESS is handled by HW.
  * ..todo
  * Every request handled by HW is not propagated to the SW, which means that you might get in trouble whenever you need these.

## Registers

_(the addresses are specified as addresses in XDATA)_

- 0xF000 = EP0 control
  * b0 = EP0 send data
  * b1 = EP0 stall
  * b2 = EP0 ~ack received data
- 0xF001 = Interrupt 0
  * b0 = Setup packet received
  * b1 = EP0 data sent
  * b2 = EP0 data received
  * b3 = Bus reset
  * b4 = ?
- 0xF011 = EP0 RX length
- 0xF012 = EP0 TX length
- 0xF020 = ..
- 0xF021 = Interrupt 1
  * b1 = x
  * b3 = x
- 0xF023 = .. ~ EP1/EP2 pptr
- 0xF025 = .. ~ EP1 plen
- 0xF026 = .. ~ EP1 stat
- 0xF027 = .. ~ EP1 ctl
- 0xF028 = .. ~ EP1 len
- 0xF029 = .. ~ EP2 ctl / stat / lenh
- 0xF02A = .. ~ EP2 len
- 0xF02B = .. ~ EP2 plen
- 0xF02C = .. ~ EP2 stat2
- 0xF02D = .. ~ EP2 ctl2
