/*
 * KonaNFI ([Kona]ta [N]AND [F]lash [I]nterface) for Phison PS2251-68
 *
 * TODO: learn how to use EP1/EP2 and NAND DMA stuff to make it work better
 */
#include <stdint.h>
#include <mcs51/8051.h>

uint8_t __xdata __at(0x2600)	usb_ep0_buff[64];
uint8_t __xdata __at(0x2800)	usb_ep1_buff[512];
uint8_t __xdata __at(0x2800)	usb_ep2_buff[512];

uint8_t __xdata __at(0xF000)	usb_ep0_ctl;
uint8_t __xdata __at(0xF001)	usb_irq0;
uint8_t __xdata __at(0xF011)	usb_ep0_rxlen;
uint8_t __xdata __at(0xF012)	usb_ep0_txlen;

uint8_t __xdata __at(0xF020)	usb_20; /* ? */
uint8_t __xdata __at(0xF021)	usb_21; /* ep1/ep2 events */
uint8_t __xdata __at(0xF023)	usb_23; /* ep1/ep2 pptr */

uint8_t __xdata __at(0xF025)	usb_25; /* ep1 plen */
uint8_t __xdata __at(0xF026)	usb_26; /* ep1 stat */
uint8_t __xdata __at(0xF027)	usb_27; /* ep1 ctl */
uint8_t __xdata __at(0xF028)	usb_28; /* ep1 len */

uint8_t __xdata __at(0xF029)	usb_29; /* ep2 ctl + stat + lenh */
uint8_t __xdata __at(0xF02A)	usb_2a; /* ep2 len */
uint8_t __xdata __at(0xF02B)	usb_2b; /* ep2 plen */
uint8_t __xdata __at(0xF02C)	usb_2c; /* ep2 stat2 */
uint8_t __xdata __at(0xF02D)	usb_2d; /* ep2 ctl2 */

uint8_t __xdata __at(0xF207)	nand_busy_val;
uint8_t __xdata __at(0xF208)	nand_cs_val;
uint8_t __xdata __at(0xF209)	nand_cs_dir;
uint8_t __xdata __at(0xF211)	nand_pio_addr;
uint8_t __xdata __at(0xF213)	nand_pio_cmd;
uint8_t __xdata __at(0xF215)	nand_pio_data;

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

const uint8_t usb_device_desc[] = {
	18,			/* bLength */
	0x01,			/* bDescriptorType */
	0x00,0x02,		/* bcdUSB */
	0,			/* bDeviceClass */
	0,			/* bDeviceSubClass */
	0,			/* bDeviceProtocol */
	0x40,			/* bMaxPacketSize */
	0xca,0x67,		/* idVendor */		/*0x67ca = hiiragi, 0x6c34 = mizu*/
	0x51,0x22,		/* idProduct */
	0x13,0x07,		/* bcdDevice */
	1,			/* iManufacturer */
	2,			/* iProduct */
	3,			/* iSerialNumber */
	1			/* bNumConfigurations */
};

const uint8_t usb_devqualifier_desc[] = {
	10,			/* bLength */
	0x06,			/* bDescriptorType */
	0x00,0x02,		/* bcdUSB */
	0,			/* bDeviceClass */
	0,			/* bDeviceSubClass */
	0,			/* bDeviceProtocol */
	0x40,			/* bMaxPacketSize0 */
	1,			/* bNumConfigurations */
	0,			/* bReserved */
};

const uint8_t usb_config_desc[] = {
	9,			/* bLength */
	0x02,			/* bDescriptorType */
	0x27,0x00,		/* wTotalLength */
	1,			/* bNumInterfaces */
	1,			/* bConfigurationValue */
	4,			/* iConfiguration */
	0x80,			/* bmAttributes */
	100/2,			/* bMaxPower */

		9,			/* bLength */
		0x04,			/* bDescriptorType */
		0,			/* bInterfaceNumber */
		0,			/* bAlternateSetting */
		3,			/* bNumEndpoints */
		0xff,			/* bInterfaceClass */
		0xff,			/* bInterfaceSubClass */
		0xff,			/* bInterfaceProtocol */
		5,			/* iInterface */

			/* EP1 -> Bulk In */
			7,			/* bLength */
			0x05,			/* bDescriptorType */
			0x81,			/* bEndpointAddress */
			0x02,			/* bmAttributes */
			0x00,0x02,		/* wMaxPacketSize */
			0,			/* bInterval */

			/* EP2 -> Bulk Out */
			7,			/* bLength */
			0x05,			/* bDescriptorType */
			0x02,			/* bEndpointAddress */
			0x02,			/* bmAttributes */
			0x00,0x02,		/* wMaxPacketSize */
			0,			/* bInterval */

			/* EP3 -> Interrupt In */
			7,			/* bLength */
			0x05,			/* bDescriptorType */
			0x83,			/* bEndpointAddress */
			0x03,			/* bmAttributes */
			0x40,0x00,		/* wMaxPacketSize */
			7,			/* bInterval */
};

/*
 * Now i'm doing it this way to avoid using the XRAM as much as possible to make
 * possible to mess with the stuff that access the XRAM without any consequence
 */
const uint8_t usb_string_desc_0[] = {
	2+2,		/* bLength */
	0x03,		/* bDescriptorType */
	0x09,0x04,	/* wLANGID[0] */
};

const uint8_t usb_string_desc_1[] = {
	2+8*2,		/* bLength */
	0x03,		/* bDescriptorType */
	'M',0, 'i',0, 'z',0, 'u',0, '-',0, 'D',0, 'E',0, 'C',0,
};

const uint8_t usb_string_desc_2[] = {
	2+(7+5+9)*2,	/* bLength */
	0x03,		/* bDescriptorType */
	'K',0, 'o',0, 'n',0, 'a',0, 'N',0, 'F',0, 'I',0,
	' ',0, 'f',0, 'o',0, 'r',0, ' ',0,
	'P',0, 'S',0, '2',0, '2',0, '5',0, '1',0, '-',0, '6',0, '8',0,
};

const uint8_t usb_string_desc_3[] = {
	2+(7+5+6+6)*2,	/* bLength */
	0x03,		/* bDescriptorType */
	'T',0, 'o',0, 't',0, 'a',0, 'l',0, 'l',0, 'y',0,
	'L',0, 'e',0, 'g',0, 'i',0, 't',0,
	'S',0, 'e',0, 'r',0, 'i',0, 'a',0, 'l',0,
	'N',0, 'u',0, 'm',0, 'b',0, 'e',0, 'r',0,
};

const uint8_t usb_string_desc_4[] = {
	2+(8+7)*2,	/* bLength */
	0x03,		/* bDescriptorType */
	'S',0, 'p',0, 'e',0, 'c',0, 'i',0, 'a',0, 'l',0, ' ',0,
	'C',0, 'o',0, 'n',0, 'f',0, 'i',0, 'g',0, 's',0,
};

const uint8_t usb_string_desc_5[] = {
	2+(8+10)*2,	/* bLength */
	0x03,		/* bDescriptorType */
	'S',0, 'p',0, 'e',0, 'c',0, 'i',0, 'a',0, 'l',0, ' ',0,
	'I',0, 'n',0, 't',0, 'e',0, 'r',0, 'f',0, 'a',0, 'c',0, 'e',0, 's',0,
};



char usb_get_desc(struct usb_setup_pkt *setup) {
	const uint8_t *desc = 0;

	switch (setup->wValue >> 8) {
	case 0x01: desc = usb_device_desc; break;
	case 0x02: desc = usb_config_desc; break;

	case 0x03:
		switch (setup->wValue & 0xff) {
		case 0: desc = usb_string_desc_0; break;
		case 1: desc = usb_string_desc_1; break;
		case 2: desc = usb_string_desc_2; break;
		case 3: desc = usb_string_desc_3; break;
		case 4: desc = usb_string_desc_4; break;
		case 5: desc = usb_string_desc_5; break;
		}
		break;

	case 0x06: desc = usb_devqualifier_desc; break;
	}

	if (desc) {
		uint16_t len = desc[0];
		if (desc[1] == 0x02) len = desc[2] | (desc[3] << 8);
		if (setup->wLength < len) len = setup->wLength;

		for (uint16_t i = 0; i < len;) {
			char n = (len - i) > 64 ? 64 : (len - i);

			/* Wait for the EP0 TX interrupt */
			while (!(usb_irq0 & 0x02));
			usb_irq0 = 0x02;	/* clear int */

			/* Fill out the buffer */
			for (char j = 0; j < n; j++)
				usb_ep0_buff[j] = desc[i+j];

			/* Send the data */
			usb_ep0_txlen = n;
			usb_ep0_ctl = 0x01;

			i += n;
		}

		return 1;
	}

	return 0;
}

void usb_h_setup(struct usb_setup_pkt *setup) {
	switch (setup->bmRequestType & 0x7f) {
	/* standard, device */
	case 0x00:
		switch (setup->bRequest) {
		/* Get descriptor */
		case 0x06:
			if (!(setup->bmRequestType & 0x80)) break;

			if (usb_get_desc(setup))
				return;

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

			/* Acknowledge */
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01;
			return;

		/* get busy */
		case KNFI_CMD_GET_BUSY:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;

			/* Response: 0xFF (busy), 0x00 (ready) */
			usb_ep0_buff[0] = (nand_busy_val & 0x01) ? 0x00 : 0xff;

			/* Send the response */
			usb_ep0_txlen = 1;
			usb_ep0_ctl = 0x01;
			return;

		/* send cmd */
		case KNFI_CMD_SEND_CMD:
			if (setup->bmRequestType & 0x80) break;

			nand_pio_cmd = setup->wValue;

			/* Acknowledge */
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01;
			return;

		/* send addr */
		case KNFI_CMD_SEND_ADDR:
			if (setup->bmRequestType & 0x80) break;

			if (setup->wLength) {
				for (uint16_t len = setup->wLength; len; ) {
					char cnt = len > 64 ? 64 : len;
					len -= cnt;

					/* Wait for the EP0 RX interrupt */
					while (!(usb_irq0 & 0x04));
					usb_irq0 = 0x04;	/* clr int */

					/* Send the address bytes to NAND */
					for (char i = 0; i < cnt; i++)
						nand_pio_addr = usb_ep0_buff[i];

					/* Acknowledge data reception */
					usb_ep0_ctl = 0x04;
				}
			} else {
				nand_pio_addr = setup->wValue;
			}

			/* Acknowledge */
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01;
			return;

		/* write data */
		case KNFI_CMD_WRITE_DATA:
			if (setup->bmRequestType & 0x80) break;
			if (!setup->wLength) break;

			/* TODO: Direct this to the EP2 instead */

			for (uint16_t len = setup->wLength; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;

				/* Wait for EP0 RX interrupt */
				while (!(usb_irq0 & 0x04));
				usb_irq0 = 0x04;	/* clr int */

				/* Send the data to NAND */
				for (char i = 0; i < cnt; i++)
					nand_pio_data = usb_ep0_buff[i];

				/* Acknowledge data reception */
				usb_ep0_ctl = 0x04;
			}

			/* Acknowledge */
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01;
			return;

		/* read data */
		case KNFI_CMD_READ_DATA:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;

			/* TODO: Direct this to the EP1 instead */

			for (uint16_t len = setup->wLength; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;

				/* Wait for EP0 TX interrupt */
				while (!(usb_irq0 & 0x02));
				usb_irq0 = 0x02;	/* clr int */

				/* Receive the data from NAND */
				for (char i = 0; i < cnt; i++)
					usb_ep0_buff[i] = nand_pio_data;

				/* Send the data */
				usb_ep0_txlen = cnt;
				usb_ep0_ctl = 0x01;
			}
			return;

		/* write xdata */
		case 0x10:
			if (setup->bmRequestType & 0x80) break;

			if (setup->wLength) {
				for (uint16_t len = setup->wLength, addr = setup->wIndex; len; ) {
					char cnt = len > 64 ? 64 : len;
					len -= cnt;

					/* Wait for EP0 RX interrupt */
					while (!(usb_irq0 & 0x04));
					usb_irq0 = 0x04;	/* clr int */

					/* Write data to XDATA */
					for (char i = 0; i < cnt; i++)
						*(uint8_t __xdata *)(addr++) = usb_ep0_buff[i];

					/* Acknowledge data reception */
					usb_ep0_ctl = 0x04;
				}
			} else {
				*(uint8_t __xdata *)setup->wIndex = setup->wValue;
			}

			/* Acknowledge */
			usb_ep0_txlen = 0;
			usb_ep0_ctl = 0x01;
			return;

		/* read xdata */
		case 0x11:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;

			for (uint16_t len = setup->wLength, addr = setup->wIndex; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;

				/* Wait for EP0 TX interrupt */
				while (!(usb_irq0 & 0x02));
				usb_irq0 = 0x02;	/* clr int */

				/* Read data from XDATA */
				for (char i = 0; i < cnt; i++)
					usb_ep0_buff[i] = *(uint8_t __xdata *)(addr++);

				/* Send data */
				usb_ep0_txlen = cnt;
				usb_ep0_ctl = 0x01;
			}
			return;

		/* read code */
		case 0x12:
			if (!(setup->bmRequestType & 0x80)) break;
			if (!setup->wLength) break;

			for (uint16_t len = setup->wLength, addr = setup->wIndex; len; ) {
				char cnt = len > 64 ? 64 : len;
				len -= cnt;

				/* Wait for EP0 TX interrupt */
				while (!(usb_irq0 & 0x02));
				usb_irq0 = 0x02;	/* clr int */

				/* Read data from XDATA */
				for (char i = 0; i < cnt; i++)
					usb_ep0_buff[i] = *(uint8_t __code *)(addr++);

				/* Send data */
				usb_ep0_txlen = cnt;
				usb_ep0_ctl = 0x01;
			}
			return;
		}
		break;
	}

	/* Send stall (when nobody returned from this function yet) */
	usb_ep0_ctl = 0x02;
}

void main(void) {
	/* Reset all NANDs */
	nand_cs_dir = ~0;	/* All CS pins to output,  */
	nand_cs_val = 0;	/* Select all NANDs */
	nand_pio_cmd = 0xff;	/* Reset command */
	nand_cs_val = ~0;	/* Deselect all NANDs */

	for (;;) {
		/* Turn off the LED */
		//P1_1 = 1;

		if (usb_irq0 & 0x08) {
			/* Bus reset */
			usb_irq0 = 0x08;	/* clr int */

			/* Do some reset stuff */
		}

		if (usb_irq0 & 0x01) {
			/* Setup packet received */
			usb_irq0 = 0x01;	/* clr int */

			/* Get the setup packet data */
			uint8_t setup[8];
			for (char i = 0; i < 8; i++) setup[i] = usb_ep0_buff[i];

			/* Acknowledge setup packet reception */
			usb_ep0_ctl = 0x04;

			/* Light up the LED */
			//P1_1 = 0;
			P1_1 ^= 1;

			/* Handle the setup packet! */
			usb_h_setup((void *)setup);
		}
	}
}
