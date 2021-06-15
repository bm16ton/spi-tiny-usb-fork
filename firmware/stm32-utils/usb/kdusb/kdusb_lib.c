#include "kdusb_lib.h"

#include "../../stm32f10x.h"
#include "../usb_defs.h"
#include "../kdusb_config.h"
#include "kdusb_lib.h"

#include "../../myprintf.h"

void USB_handleSETUP();
void USB_handleOUT();
void USB_handleIN();
void USB_prepareData();

// Variables
const USB_descriptorInfo USB_StringDescriptors[];
USB_Request usbRequest;
uint8_t USB_ep;
uint8_t *usbData;
volatile int16_t USB_dataOffset, USB_dataLen;
volatile enum USBStates USB_state = NONE;

// #define IMR_MSK (USB_CNTR_CTRM | USB_CNTR_SOFM | USB_CNTR_RESETM | USB_CNTR_SUSPM | USB_CNTR_WKUPM)
#define IMR_MSK (USB_CNTR_CTRM | USB_CNTR_RESETM)

#include "kdusb_funcs.h"

#define PORTCR_FROM_PORTPIN(port,pin) *(uint32_t*)((uint32_t)port + 0x04 * (pin / 8))
static inline void IO_SET_CNF_MODE(GPIO_TypeDef* port, int pin, uint32_t cnf, uint32_t mode)
{
	uint32_t v = PORTCR_FROM_PORTPIN(port, pin);
	v &= ~(0b1111 << ((pin % 8) * 4));
	v |= ((cnf << 2) | mode) << ((pin % 8) * 4);
	PORTCR_FROM_PORTPIN(port, pin) = v;
}
static inline void IO_PUSH_PULL(GPIO_TypeDef* port, int pin)
{
	IO_SET_CNF_MODE(port, pin, 0b00, 0b11);
}
static inline void IO_INPUT(GPIO_TypeDef* port, int pin)
{
	IO_SET_CNF_MODE(port, pin, 0b01, 0b00);
}
static inline void IO_HIGH(GPIO_TypeDef* port, int pin)
{
	// uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	// *addr = 1;l
	port->BSRR = 1 << pin;
}
static inline void IO_LOW(GPIO_TypeDef* port, int pin)
{
	// uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	// *addr = 0;
	port->BRR = 1 << pin;
}

void usbInit()
{
	USB->CNTR = IMR_MSK;
	USB->ISTR = 0;

	// Pullup is conected to pin
#if USB_PULLUP == 0

	// Pullup is connected to VCC
#elif USB_PULLUP == 1

#endif
}
void usbDisconnect()
{
	USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN | IMR_MSK;
	USB->ISTR = 0;

	// Pullup is conected to pin
#if USB_PULLUP == 0
	IO_INPUT(USB_PULLUP_PORT, USB_PULLUP_PIN);
	IO_LOW(USB_PULLUP_PORT, USB_PULLUP_PIN);

	// Pullup is connected to VCC
#elif USB_PULLUP == 1
	IO_PUSH_PULL(USB_PORT, USB_DPLUS_PIN);
	IO_PUSH_PULL(USB_PORT, USB_DMINUS_PIN);
	USB_PORT->BRR = (1 << USB_DPLUS_PIN) | (1 << USB_DMINUS_PIN);
#endif
}
void usbConnect()
{
	USB->CNTR = IMR_MSK;
	USB->ISTR = 0;

	// Pullup is conected to pin
#if USB_PULLUP == 0
	IO_PUSH_PULL(USB_PULLUP_PORT, USB_PULLUP_PIN);
	IO_HIGH(USB_PULLUP_PORT, USB_PULLUP_PIN);

	// Pullup is connected to VCC
#elif USB_PULLUP == 1
	IO_INPUT(USB_PORT, USB_DPLUS_PIN);
	IO_INPUT(USB_PORT, USB_DMINUS_PIN);
	USB_PORT->BSRR = (1 << USB_DPLUS_PIN) | (1 << USB_DMINUS_PIN);
#endif
}
void USB_LP_CAN_RX0_Handler()
{
	// if (USB->ISTR & USB_CNTR_SOFM)
	// {
	// USB_sof++;
	// USB->ISTR = ~USB_ISTR_SOF;
	// }
	if (USB->ISTR & USB_CNTR_CTRM)
	{
		uint8_t dir = USB_get_ISTR_DIR();
		USB_ep = USB_get_ISTR_EP_ID();

		if (USB_ep == 0)
		{
			uint8_t setup = USB_get_SETUP(0);

			if (dir == 1) // host to device transaction
			{
				USB_clear_CTR_RX(0);
				if (setup) // SETUP token
				{
					uint8_t cnt = USB_getEPRxCount(0);
					USB_PMA2userspace((uint8_t*)&usbRequest, USB_getEPRxAddr(0), cnt);
					USB_handleSETUP();
				}
				else // OUT token
				{
					USB_handleOUT();
				}
			}
			else // device to host transaction - IN token
			{
				USB_clear_CTR_TX(0);
				USB_handleIN();
			}
		}
#if USB_ENABLEINTERRUPT1 == 1
		else
		{
			if (dir == 1) // host to device transaction
			{
				USB_clear_CTR_RX(USB_ep);

				uint8_t cnt = USB_getEPRxCount(USB_ep);
				DBG("EP OUT %d l: %d", USB_ep, cnt);

				USB_set_STAT_RX(USB_ep, USB_EP_VALID);
			}
			else // device to host transaction - IN token
			{
				USB_clear_CTR_TX(USB_ep);
				DBG("EP IN %d", USB_ep);
				USB_set_STAT_RX(USB_ep, USB_EP_VALID);
			}
		}
#endif
	}
	if (USB->ISTR & USB_CNTR_RESETM)
	{
		USB->ISTR = ~USB_CNTR_RESETM;

		USB->BTABLE = 0;

		USB_setEPNumTypeKind(0, 0, USB_EP_CONTROL, 0);
		USB_set_STAT_TX(0, USB_EP_STALL);
		USB_set_STAT_RX(0, USB_EP_VALID);

		USB_setEPTxAddr(0, 0x40);
		USB_setEPTxCount(0, 0);
		USB_setEPRxAddr(0, 0x80);
		USB_setEPRxBlocks32(0, 2);

#if USB_ENABLEINTERRUPT1 == 1
		USB_setEPNumTypeKind(1, 1, USB_EP_INTERRUPT, 0);
		USB_set_STAT_TX(1, USB_EP_NAK);
		USB_set_STAT_RX(1, USB_EP_VALID);

		USB_setEPTxAddr(1, 0xc0);
		USB_setEPTxCount(1, 0);
#endif
		// // USB_setEPRxAddr     (1, 0x100);
		// // USB_setEPRxBlocks32 (1, 2);

		// USB_setEPNum (2, 2);
		// USB_setEPType (2, USB_EP_INTERRUPT);
		// USB_setEPKind (2, 0);
		// USB_set_STAT_TX (2, USB_EP_NAK);
		// USB_set_STAT_RX (2, USB_EP_VALID);

		// // USB_setEPTxAddr     (2, 0x100);
		// // USB_setEPTxCount    (2, 0);
		// USB_setEPRxAddr     (2, 0x100);
		// USB_setEPRxBlocks32 (2, 2);

		USB_enableDevice();

		DBG(" ----- RESET -----");
	}
	// if (USB->ISTR & USB_CNTR_SUSPM)
	// {
	// USB->ISTR = ~USB_ISTR_SUSP;
	// }
	// if (USB->ISTR & USB_CNTR_WKUPM)
	// {
	// USB->ISTR = ~USB_ISTR_WKUP;
	// }
}

void USB_handleStandardRequest()
{
	if (USB_RECP_IS_DEVICE(usbRequest.bmRequestType))
	{
		uint8_t cmd, idx;
		switch (usbRequest.bRequest)
		{
		case GET_DESCRIPTOR:
			cmd = usbRequest.wValue.bytes[1];

			switch (cmd)
			{
			case DEVICE_DESCRIPTOR:
				USB_setDataToSend((uint8_t*)&USB_DeviceDescriptor, USB_DEVICE_DESCRIPTOR_LEN);
				break;

			case CONFIG_DESCRIPTOR:
				USB_setDataToSend((uint8_t*)&USB_ConfigDescriptor, USB_CONFIG_DESCRIPTOR_LEN);
				break;

			case STRING_DESCRIPTOR:
				idx = usbRequest.wValue.bytes[0]; // string index
				if (idx <= 3)
					USB_setDataToSend((uint8_t*)USB_StringDescriptors[idx].descr, USB_StringDescriptors[idx].len);
				break;

			case QUALIFIER_DESCRIPTOR:
				break;
			}
			break;

		case SET_ADDRESS:
			// Address will be setup after STATUS_IN
			break;

		case SET_CONFIGURATION:
			break;

		case GET_CONFIGURATION:
			USB_setDataToSend(0x00, 1);
			break;

		case GET_STATUS:
			USB_setDataToSend("\x01\x00", 2);
			break;
		}
	}
	else if (USB_RECP_IS_INTERFACE(usbRequest.bmRequestType))
	{
		switch (usbRequest.bRequest)
		{
		case GET_STATUS:
			USB_setDataToSend(0x00, 1);
			break;
		}
	}
	else if (USB_RECP_IS_ENDPOINT(usbRequest.bmRequestType))
	{
		switch (usbRequest.bRequest)
		{
		case GET_STATUS:
			USB_setDataToSend(0x00, 1);
			break;
		}
	}
}

// Handlers
void USB_handleSETUP()
{
	USB_dataLen = 0;

	USB_printRequest();

	uint8_t rqType = usbRequest.bmRequestType;
	if (USB_TYPE_IS_STANDARD(rqType))
	{
		USB_handleStandardRequest();

		// Device to host
		if (USB_DIR_IS_DEVICE_TO_HOST(rqType)) // pending IN
		{
			if (USB_dataLen)
			{
				USB_dataOffset = 0;
				if (USB_dataLen > usbRequest.wLength)
					USB_dataLen = usbRequest.wLength;
				USB_prepareData();
			}
			else
			{
				USB_set_STAT_TX(0, USB_EP_STALL);
				DBG("set STALL (1)");
				USB_state = NONE;
			}
		}
		// Host to device (we don't want any data from the host's OUT tokens, so we wait until host sends STATUS IN)
		else // pending OUT
		{
			if (usbRequest.wLength == 0) // host doesn't want to send data, so send STATUS IN
			{
				USB_send0LengthFrame();
				USB_state = WAIT_STATUS_IN;
			}
			else // host will send some data, but we don't want it
			{
				USB_set_STAT_TX(0, USB_EP_STALL);
				DBG("set STALL (2)");
				USB_state = NONE;
			}
		}

		USB_set_STAT_RX(0, USB_EP_VALID);
	}
	else // if non-standard request
	{
		uint16_t res = usbFunctionSetup();
		// if res == 0xffff - data will be provided on the fly using usbPrepareData function
		// otherwise, res is amount of bytes in usbData

		if (USB_DIR_IS_DEVICE_TO_HOST(rqType)) // pending IN (host will send IN token on next transmission)
		{
			if (usbRequest.wLength == 0 || res == 0) // host doesn't want to receive any data, so send STATUS IN
			{
				USB_send0LengthFrame();
				USB_state = WAIT_STATUS_IN;
			}
			else // usb host expects some data
			{
				if (res == 0xffff) // user will provide own data on fly
				{
#if USB_IMPLEMENT_PREPAREUSERDATA == 1
					USB_dataLen = -1;
#else
					DBG("USER SHOULD NOT RETURN 0XFFFF WHEN USB_IMPLEMENT_PREPAREUSERDATA IS 0!");
					_errorloop();
#endif
				}
				else
				{
					USB_dataLen = res;
					if (USB_dataLen > usbRequest.wLength)
						USB_dataLen = usbRequest.wLength;
				}
				USB_dataOffset = 0;
				USB_prepareData();
			}
		}
		else // pending OUT (host will send OUT token on next transmission)
		{
			if (usbRequest.wLength == 0) // host doesn't want to send any data, so send STATUS IN
			{
				USB_send0LengthFrame();
				USB_state = WAIT_STATUS_IN;
			}
			else // prepare for receiving data
			{
				USB_state = OUT_DATA;
				USB_dataOffset = 0;
				USB_dataLen = usbRequest.wLength;
			}
		}
		// USB_send0LengthFrame ();
		USB_set_STAT_RX(0, USB_EP_VALID);
	}
}
void USB_handleOUT()
{
	if (USB_state == WAIT_STATUS_OUT)
	{
		DBG("STATUS OUT");
	}
	else if (USB_state == OUT_DATA)
	{
		uint8_t cnt = USB_getEPRxCount(0);
		USB_dataOffset += cnt;
		DBG("OUT (len: %u)", cnt);
		if (!USB_TYPE_IS_STANDARD(usbRequest.bmRequestType))
		{
			USB_PMA2userspace(usbData, USB_getEPRxAddr(0), cnt);
			usbHandleData(cnt);
		}

		if (USB_dataOffset == USB_dataLen)
		{
			DBG("read all");
			USB_send0LengthFrame();
			USB_state = WAIT_STATUS_IN;
		}
		USB_set_STAT_RX(0, USB_EP_VALID);
	}
	else
	{
		DBG("OUT");
	}
}
void USB_handleIN()
{
	if (USB_state == WAIT_STATUS_IN)
	{
		DBG("STATUS IN");
		uint8_t rqType = usbRequest.bmRequestType;
		if (USB_TYPE_IS_STANDARD(rqType) && USB_RECP_IS_DEVICE(rqType))
		{
			if (usbRequest.bRequest == SET_ADDRESS)
			{
				DBG("set addr to %d", usbRequest.wValue);
				USB_setAddress(usbRequest.wValue.bytes[0]);
				USB_set_STAT_TX(0, USB_EP_STALL);
			}
		}
		USB_state = NONE;
	}
	else if (USB_state == IN_DATA)
	{
		DBG("IN");
		USB_prepareData();
	}
	else if (USB_state == LAST_IN_DATA)
	{
		DBG("IN (last)");
		USB_state = WAIT_STATUS_OUT;
	}
	else
	{
		DBG("unknown IN state");
	}
	USB_set_STAT_RX(0, USB_EP_VALID);
}

void USB_prepareData()
{
	uint16_t toSend = 0;
#if USB_IMPLEMENT_PREPAREUSERDATA == 1
	if (USB_dataLen == -1)
	{
		uint16_t avail = usbRequest.wLength - USB_dataOffset;

		if (avail > USB_PACKET_SIZE)
			avail = USB_PACKET_SIZE;

		toSend = usbPrepareUserData(avail);
		DBG("user provided %d", toSend);
		USB_userspace2PMA(USB_getEPTxAddr(0), usbData, toSend);
	}
	else if (USB_dataLen > 0)
#endif
	{
		toSend = USB_dataLen - USB_dataOffset;

		if (toSend > USB_PACKET_SIZE)
			toSend = USB_PACKET_SIZE;

		DBG("provided %d", toSend);
		USB_userspace2PMA(USB_getEPTxAddr(0), usbData + USB_dataOffset, toSend);
	}

	if (toSend == 0)
	{
		DBG("NO DATA TO SEND ON IN TOKEN");
		_errorloop();
	}

	USB_dataOffset += toSend;
	USB_setEPTxCount(0, toSend);
	USB_set_STAT_TX(0, USB_EP_VALID);
	if (USB_dataOffset == usbRequest.wLength || toSend < USB_PACKET_SIZE)
	{
		USB_dataLen = 0;
		USB_state = LAST_IN_DATA;
		DBG("all sent");
	}
	else
	{
		USB_state = IN_DATA;
	}
}

// Interrupt handling
void usbSetInterruptData(uint8_t ep, uint8_t* data, uint8_t len)
{
	USB_setEPTxCount(ep, len);
	USB_userspace2PMA(USB_getEPTxAddr(ep), data, len);
	USB_set_STAT_TX(ep, USB_EP_VALID);
}

// Consts
const char USB_String0Descriptor[] =   /* language descriptor */
{
	4,          /* sizeof(usbDescriptorString0): length of descriptor in bytes */
	3,          /* descriptor type */
	0x09, 0x04, /* language index (0x0409 = US-English) */
};
#define USB_STRING_DESCRIPTOR_HEADER(strLen) (((strLen) * 2 + 2) | (3 << 8))
const uint16_t USB_String1Descriptor[] =
{
	USB_STRING_DESCRIPTOR_HEADER(USB_VENDOR_LEN),
	USB_VENDOR
};
const uint16_t USB_String2Descriptor[] =
{
	USB_STRING_DESCRIPTOR_HEADER(USB_PRODUCT_LEN),
	USB_PRODUCT
};
const uint16_t USB_String3Descriptor[] =
{
	USB_STRING_DESCRIPTOR_HEADER(USB_SERIAL_LEN),
	USB_SERIAL
};
const USB_descriptorInfo USB_StringDescriptors[] =
{
	{ (uint8_t*)&USB_String0Descriptor, 4 },
	{ (uint8_t*)&USB_String1Descriptor, USB_STRING_DESCRIPTOR_LEN(USB_VENDOR_LEN) },
	{ (uint8_t*)&USB_String2Descriptor, USB_STRING_DESCRIPTOR_LEN(USB_PRODUCT_LEN) },
	{ (uint8_t*)&USB_String3Descriptor, USB_STRING_DESCRIPTOR_LEN(USB_SERIAL_LEN) },
};
