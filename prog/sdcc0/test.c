#include <stdio.h>
#include <stdint.h>
#include <mcs51/8052.h>

#define mzw_set0()	{P1_1 = 1;}
#define mzw_set1()	{P1_1 = 0;}
#define mzw_delay()	{ for (volatile char i = 100; i; i--); }

int putchar(int ch) {
	for (int m = 0x1; m != 0x200; m <<= 1) {
		// reference pulse
		mzw_set1();
		mzw_delay();
		
		// data pulse
		mzw_set0();
		mzw_delay();
		
		if (!(m & 0xff)) {
			// end
			mzw_delay();
			mzw_delay();
		} else if (ch & m) {
			// one
			mzw_delay();
		}
	}
	
	return 0;
}

uint8_t __xdata __at(0xf023)	xdata_pram_off; // I think it's actually the EP Buffer addr
uint8_t __xdata __at(0xf207)	nand_busy_val;
uint8_t __xdata __at(0xf208)	nand_cs_val;
uint8_t __xdata __at(0xf209)	nand_cs_dir; // probably
uint8_t __xdata __at(0xf211)	nand_pio_addr;
uint8_t __xdata __at(0xf213)	nand_pio_cmd;
uint8_t __xdata __at(0xf215)	nand_pio_data;

uint8_t __xdata buff[128];

void hexdumpxd(uint8_t __xdata *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		printf("%p: ", ptr + i);
		
		for (char j = 0; j < 16; j++)
			printf("%02x ", ptr[i+j]);
		
		printf("|");
		
		for (char j = 0; j < 16; j++) {
			uint8_t b = ptr[i+j];
			if (b < 0x20 || b >= 0x7f) b = '.';
			putchar(b);
		}
		
		printf("|\n");
	}
}

void hexdumpid(uint8_t __idata *ptr, int len) {
	for (int i = 0; i < len; i += 16) {
		printf("%p: ", ptr + i);
		
		for (char j = 0; j < 16; j++)
			printf("%02x ", ptr[i+j]);
		
		printf("|");
		
		for (char j = 0; j < 16; j++) {
			uint8_t b = ptr[i+j];
			if (b < 0x20 || b >= 0x7f) b = '.';
			putchar(b);
		}
		
		printf("|\n");
	}
}

void main(void) {
	for (char i = 0; i < 10; i++) putchar('\n');
	puts("====== konata IZUMI, Phison PS2251-68 ======");
	
	#if 0
	// sel nand
	nand_cs_val = 0xfe;
	nand_cs_dir = 0xff;
	
	// reset
	nand_pio_cmd = 0xff;
	while (!(nand_busy_val & 0x01));
	
	// id
	nand_pio_cmd = 0x90;
	nand_pio_addr = 0;
	for (char i = 0; i < 6; i++) buff[i] = nand_pio_data;
	printf("NAND ID: %02X-%02X-%02X-%02X-%02X-%02X\n",
		buff[0],buff[1],buff[2],buff[3],buff[4],buff[5]);
	
	// program
	/*nand_pio_cmd = 0x80;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x04;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_data = 'k';
	nand_pio_data = 'a';
	nand_pio_data = 'g';
	nand_pio_data = 'a';
	nand_pio_data = 'm';
	nand_pio_data = 'i';
	nand_pio_data = '-';
	nand_pio_data = 'c';
	nand_pio_data = 'h';
	nand_pio_data = 'a';
	nand_pio_data = 'n';
	nand_pio_cmd = 0x10;
	while (!(nand_busy_val & 0x01));
	nand_pio_cmd = 0x70;
	printf("WRite_STatus=%02x\n", nand_pio_data);*/
	
	// read
	nand_pio_cmd = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_cmd = 0x30;
	while (!(nand_busy_val & 0x01));
	
	for (char i = 0; i < 128; i++)
		buff[i] = nand_pio_data;
	
	hexdumpxd(buff, sizeof(buff));
	
	// read
	nand_pio_cmd = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x04;
	nand_pio_addr = 0x00;
	nand_pio_addr = 0x00;
	nand_pio_cmd = 0x30;
	while (!(nand_busy_val & 0x01));
	
	for (char i = 0; i < 128; i++)
		buff[i] = nand_pio_data;
	
	hexdumpxd(buff, sizeof(buff));
	#endif
	
	xdata_pram_off = 0x30;
	hexdumpxd((void __xdata *)0x0000, 0x100);
	xdata_pram_off = 0x00;
	hexdumpxd((void __xdata *)0x0000, 0x100);
	
	for (;;) ;
}
