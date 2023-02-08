#include <stdint.h>
#include <mcs51/8051.h>


uint8_t __xdata __at(0x2600)	usb_ep0_buff[64];
uint8_t __xdata __at(0x2800)	usb_ep1_buff[512];
uint8_t __xdata __at(0x2800)	usb_ep2_buff[512];

uint8_t __xdata __at(0xf000)	usb_ep0_ctl;
uint8_t __xdata __at(0xf001)	usb_irq0;
uint8_t __xdata __at(0xf011)	usb_ep0_rxlen;
uint8_t __xdata __at(0xf012)	usb_ep0_txlen;

uint8_t __xdata __at(0xf020)	usb_020; // ?
uint8_t __xdata __at(0xf021)	usb_021; // ep1/ep2 events
uint8_t __xdata __at(0xf023)	usb_023; // ep1/ep2 pptr

uint8_t __xdata __at(0xf025)	usb_025; // ep1 plen
uint8_t __xdata __at(0xf026)	usb_026; // ep1 stat
uint8_t __xdata __at(0xf027)	usb_027; // ep1 ctl
uint8_t __xdata __at(0xf028)	ubs_028; // ep1 len

uint8_t __xdata __at(0xf029)	usb_029; // ep2 ctl + stat + lenh
uint8_t __xdata __at(0xf02a)	usb_02a; // ep2 len
uint8_t __xdata __at(0xf02b)	usb_02b; // ep2 plen
uint8_t __xdata __at(0xf02c)	usb_02c; // ep2 stat2
uint8_t __xdata __at(0xf02d)	usb_02d; // ep2 ctl2

uint8_t __xdata __at(0xf207)	nand_busy_val;
uint8_t __xdata __at(0xf208)	nand_cs_val;
uint8_t __xdata __at(0xf209)	nand_cs_dir;
uint8_t __xdata __at(0xf211)	nand_pio_addr;
uint8_t __xdata __at(0xf213)	nand_pio_cmd;
uint8_t __xdata __at(0xf215)	nand_pio_data;

#define KNFI_CMD_SEL		0x00
#define KNFI_CMD_GET_BUSY	0x01
#define KNFI_CMD_SEND_CMD	0x02
#define KNFI_CMD_SEND_ADDR	0x03
#define KNFI_CMD_WRITE_DATA	0x04
#define KNFI_CMD_READ_DATA	0x05

struct usb_setup_pkt {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

const uint8_t usb_my_desc_device[] = {
	18,			// bLength
	0x01,			// bDescriptorType
	0x00,0x02,		// bcdUSB
	0xff,			// bDeviceClass
	0x00,			// bDeviceSubClass
	0x00,			// bDeviceProtocol
	0x40,			// bMaxPacketSize
	0xca,0x67,		// idVendor
	0xc6,0xbe,		// idProduct
	0x13,0x07,		// bcdDevice
	1,			// iManufacturer
	2,			// iProduct
	3,			// iSerialNumber
	1			// bNumConfigurations
};

const uint8_t usb_my_desc_config[] = {
	9,			// bLength
	0x02,			// bDescriptorType
	0x27,0x00,		// wTotalLength
	1,			// bNumInterfaces
	1,			// bConfigurationValue
	4,			// iConfiguration
	0x80,			// bmAttributes
	100/2,			// bMaxPower
	
		9,			// bLength
		0x04,			// bDescriptorType
		0,			// bInterfaceNumber
		0,			// bAlternateSetting
		3,			// bNumEndpoints
		0xff,			// bInterfaceClass
		0x00,			// bInterfaceSubClass
		0x00,			// bInterfaceProtocol
		5,			// iInterface
		
		/* why do i need all those endpoints ? */
			/* EP1 -> Bulk In */
			7,			// bLength
			0x05,			// bDescriptorType
			0x81,			// bEndpointAddress
			0x02,			// bmAttributes
			0x00,0x02,		// wMaxPacketSize
			0,			// bInterval
			/* EP2 -> Bulk Out */
			7,			// bLength
			0x05,			// bDescriptorType
			0x02,			// bEndpointAddress
			0x02,			// bmAttributes
			0x00,0x02,		// wMaxPacketSize
			0,			// bInterval
			/* EP3 -> Interrupt In */
			7,			// bLength
			0x05,			// bDescriptorType
			0x83,			// bEndpointAddress
			0x03,			// bmAttributes
			0x40,0x00,		// wMaxPacketSize
			7,			// bInterval
};

const short *usb_my_desc_string[] = {
	u"\x0409", // FIXME, does only store the low byet !!
	u"Mizu-DEC",
	u"Phison PS2251-68 NAND Interface",
	u"kagami hiiragi @ "__DATE__" "__TIME__,
	u"Some nonsense configs",
	u"NAND access popas"
};

void usb_ep0_send(uint8_t *ptr, int len) {
	do {
		char cnt = len > 64 ? 64 : len;
		
		// wait for the ep0 sent int
		while (!(usb_irq0 & 0x02));
		usb_irq0 = 0x02; // clr int
		
		// copy data
		for (char i = 0; i < cnt; i++) usb_ep0_buff[i] = ptr[i];
		
		// start to send
		usb_ep0_txlen = cnt;
		usb_ep0_ctl = 0x01;
		
		len -= cnt; ptr += cnt;
	} while (len > 0);
}

void usb_h_setup(struct usb_setup_pkt *setup) {
	switch (setup->bmRequestType & 0x7f) {
	/* standard, device */
	case 0x00:
		switch (setup->bRequest) {
		/* Get descriptor */
		case 0x06:
			if (!(setup->bmRequestType & 0x80)) break;
			
			switch (setup->wValue >> 8) {
			/* device descriptor */
			case 0x01:
				usb_ep0_send((void *)usb_my_desc_device, setup->wLength);
				return;
			
			/* config descriptor */
			case 0x02:
				usb_ep0_send((void *)usb_my_desc_config, setup->wLength);
				return;
			
			/* string descriptor */
			case 0x03:
				{
					static __xdata uint16_t descdat[128];
					uint8_t idx = setup->wValue & 0xff;
					
					// does the index overflow?
					if ((sizeof(usb_my_desc_string) / sizeof(usb_my_desc_string[0])) <= idx)
						break;
					
					// count the length
					int len = 2;
					for (const short *s = usb_my_desc_string[idx]; *s; s++) len += 2;
					if (len > 255) len = 255; // clamp the length
					
					// make the descriptor
					descdat[0] = (0x03 << 8) | len;
					//memcpy(&descdat[1], usb_my_desc_string[idx], len - 2);
					for (char i = 1; i < len / 2; i++) descdat[i] = usb_my_desc_string[idx][i-1];
					
					usb_ep0_send((void *)descdat, setup->wLength);
				}
				return;
			}
			break;
		}
		break;
	
	/* vendor, interface */
	case 0x41:
		switch (setup->bRequest) {
		/* select nand */
		case KNFI_CMD_SEL:
			if (setup->bmRequestType & 0x80) break;
			
			nand_cs_val = ~setup->wValue;
			nand_cs_dir = ~0;
			
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01; // send ack
			return;
		
		/* get busy */
		case KNFI_CMD_GET_BUSY:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;
			
			usb_ep0_buff[0] = (nand_busy_val & 0x01) ? 0x00 : 0xff;
			
			usb_ep0_txlen = 1;
			usb_ep0_ctl = 0x01; // tx
			return;
		
		/* send cmd */
		case KNFI_CMD_SEND_CMD:
			if (setup->bmRequestType & 0x80) break;
			
			nand_pio_cmd = setup->wValue;
			
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01; // send ack
			return;
		
		/* send addr */
		case KNFI_CMD_SEND_ADDR:
			if (setup->bmRequestType & 0x80) break;
			
			if (setup->wLength) {
				for (uint16_t len = setup->wLength; len; ) {
					char cnt = len > 64 ? 64 : len;
					len -= cnt;
					
					// wait for ep0 rx int
					while (!(usb_irq0 & 0x04));
					usb_irq0 = 0x04; // clr int
					
					for (char i = 0; i < cnt; i++)
						nand_pio_addr = usb_ep0_buff[i];
					
					usb_ep0_ctl = 0x04; // ack
				}
			} else {
				nand_pio_addr = setup->wValue;
			}
			
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01; // send ack
			return;
		
		/* write data */
		case KNFI_CMD_WRITE_DATA:
			if (setup->bmRequestType & 0x80) break;
			if (!setup->wLength) break;
			
			/* TODO: Direct this to the EP2 instead */
			
			for (uint16_t len = setup->wLength; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;
				
				// wait for ep0 rx int
				while (!(usb_irq0 & 0x04));
				usb_irq0 = 0x04; // clr int
				
				for (char i = 0; i < cnt; i++)
					nand_pio_data = usb_ep0_buff[i];
				
				usb_ep0_ctl = 0x04; // ack
			}
			
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01; // send ack
			return;
		
		/* read data */
		case KNFI_CMD_READ_DATA:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;
			
			/* TODO: Direct this to the EP1 instead */
			
			for (uint16_t len = setup->wLength; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;
				
				// wait for ep0 tx int
				while (!(usb_irq0 & 0x02));
				usb_irq0 = 0x02; // clr int
				
				for (char i = 0; i < cnt; i++)
					usb_ep0_buff[i] = nand_pio_data;
				
				usb_ep0_txlen = cnt;
				usb_ep0_ctl = 0x01; // tx
			}
			return;
		
		/* write xdata */
		case 0x10:
			if (setup->bmRequestType & 0x80) break;
			
			if (setup->wLength) {
				for (uint16_t len = setup->wLength, addr = setup->wIndex; len; ) {
					char cnt = len > 64 ? 64 : len;
					len -= cnt;
					
					// wait for ep0 rx int
					while (!(usb_irq0 & 0x04));
					usb_irq0 = 0x04; // clr int
					
					for (char i = 0; i < cnt; i++)
						*(uint8_t __xdata *)(addr++) = usb_ep0_buff[i];
					
					usb_ep0_ctl = 0x04; // ack
				}
			} else {
				*(uint8_t __xdata *)setup->wIndex = setup->wValue;
			}
			
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01; // send ack
			return;
		
		/* read xdata */
		case 0x11:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;
			
			for (uint16_t len = setup->wLength, addr = setup->wIndex; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;
				
				// wait for ep0 tx int
				while (!(usb_irq0 & 0x02));
				usb_irq0 = 0x02; // clr int
				
				for (char i = 0; i < cnt; i++)
					usb_ep0_buff[i] = *(uint8_t __xdata *)(addr++);
				
				usb_ep0_txlen = cnt;
				usb_ep0_ctl = 0x01; // tx
			}
			
			return;
		}
		break;
	}
	
	usb_ep0_ctl = 0x02; // send stall
}


void main(void) {
	// reset all NANDs
	nand_cs_dir = ~0;    // all CS to output?
	nand_cs_val = 0;     // select all NANDs
	nand_pio_cmd = 0xff; // reset
	nand_cs_val = ~0;    // deselect all NANDs

	for (;;) {
		P1_1 = 1; // turn off LED
		
		if (usb_irq0 & 0x08) {
			// bus reset
			usb_irq0 = 0x08; // clr int
			
			/* Do some reset stuff */
		}
		
		if (usb_irq0 & 0x01) {
			// setup received
			usb_irq0 = 0x01; // clr int
			
			uint8_t setup[8];
			for (char i = 0; i < 8; i++) setup[i] = usb_ep0_buff[i];
			
			usb_ep0_ctl = 0x04; // ack
			
			P1_1 = 0; // light up LED
			
			usb_h_setup((void *)setup);
		}
	}
}
