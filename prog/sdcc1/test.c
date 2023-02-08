#include <stdio.h>
#include <stdint.h>
#include <mcs51/8051.h>

uint8_t __xdata __at(0xf122)	xfr122;
uint8_t __xdata __at(0xf123)	xfr123;

uint8_t __xdata __at(0xf207)	nand_busy_val;
uint8_t __xdata __at(0xf208)	nand_cs_val;
uint8_t __xdata __at(0xf209)	nand_cs_dir;
uint8_t __xdata __at(0xf211)	nand_pio_addr;
uint8_t __xdata __at(0xf213)	nand_pio_cmd;
uint8_t __xdata __at(0xf215)	nand_pio_data;


volatile char ctr = 0;

void Timer0_IRQ(void) __interrupt(1) {
	TR0 = 0;
	
	if (ctr++ >= 100) {
		ctr = 0;
		P1_1 ^= 1;
	}
	
	// 50000
	TH0 = 0x3c;
	TL0 = 0xb0;
	
	TR0 = 1;
}


void main(void) {
	IE = 0x02; // only Timer0
	EA = 1;
	
	TMOD = 0x01; // T0 mode1 (16 bit counter)
	
	IE0 = 1; // en T0 int
	
	// immediate goto int
	TH0 = 0xff;
	TL0 = 0xff;
	
	TR0 = 1; // start T0
	
	for (;;) ;
}
