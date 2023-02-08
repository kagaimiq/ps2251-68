#include <stdint.h>
#include <mcs51/8051.h>


uint8_t __xdata __at(0x2600)	usb_ep0_buff[64];
uint8_t __xdata __at(0x2800)	usb_ep1_buff[512];
uint8_t __xdata __at(0x2800)	usb_ep2_buff[512];

uint8_t __xdata __at(0xf000)	usb_ep0_ctl;
uint8_t __xdata __at(0xf001)	usb_irq0;
uint8_t __xdata __at(0xf011)	usb_ep0_rxlen;
uint8_t __xdata __at(0xf012)	usb_ep0_txlen;

uint8_t __xdata __at(0xf207)	nand_busy_val;
uint8_t __xdata __at(0xf208)	nand_cs_val;
uint8_t __xdata __at(0xf209)	nand_cs_dir;
uint8_t __xdata __at(0xf211)	nand_pio_addr;
uint8_t __xdata __at(0xf213)	nand_pio_cmd;
uint8_t __xdata __at(0xf215)	nand_pio_data;





void usb_main_irq(void) __interrupt(0) {
	uint8_t irq = usb_irq0;
	usb_irq0 = irq;
	
	if (irq & 0x08) {
		// bus reset
		
		
	}
	
	if (irq & 0x01) {
		// setup
		uint8_t setup[8];
		for (char i = 0; i < 8; i++) setup[i] = usb_ep0_buff[i];
		
		usb_ep0_ctl = 0x04; // ack
		
		P1_1 ^= 1;
	} else
	
	if (irq & 0x02) {
		// ep0 tx
		
		
	} else
	
	if (irq & 0x04) {
		// ep0 rx
		
		
	}
}





void main(void) {
	IE = 0; // clear all unwanted ones
	EX0 = 1; // USB main irq
	EX1 = 0; // USB EP1/2/3 irq
	
	P1_1 = 1; // turn off led
	
	EA = 1; // en ints
	
	for (;;) ;
}
