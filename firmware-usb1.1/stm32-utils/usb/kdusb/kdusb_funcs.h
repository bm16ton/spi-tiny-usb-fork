#ifndef __KDUSB_FUNCS_H__
#define __KDUSB_FUNCS_H__

// Debug
#ifdef KDUSB_DEBUG
#define DBG2(format, ...) myprintf (format, ##__VA_ARGS__)
#else
#define DBG2(format, ...)
#endif
#define DBG(format, ...) DBG2(format "\r\n", ##__VA_ARGS__)

#define VV(st,le) ((v >> st) & ((1 << le) - 1))
#define VVx(x,st,le) ((x >> st) & ((1 << le) - 1))
void USB_printEP0R()
{
#ifdef KDUSB_DEBUG
	uint16_t v = USB->EP0R;
	DBG("EP0R = CTR_RX %d DTOG_RX %d STAT_RX 0b%02b SETUP %d EP_TYPE 0b%02b EP_KIND %d CTR_TX %d DTOG_TX %d STAT_TX 0b%02b EA %d",
	    VV(15, 1), VV(14, 1), VV(12, 2), VV(11, 1), VV(9, 2), VV(8, 1), VV(7, 1), VV(6, 1), VV(4, 2), VV(0, 4));
#endif
}
void USB_printRequest()
{
#ifdef KDUSB_DEBUG
	DBG2("SETUP rqt: 0x%02x (", usbRequest.bmRequestType);
	
	if (USB_DIR_IS_DEVICE_TO_HOST(usbRequest.bmRequestType))
		DBG2("D>H");
	else if (USB_DIR_IS_HOST_TO_DEVICE(usbRequest.bmRequestType))
		DBG2("H>D");
		
	if (USB_TYPE_IS_STANDARD(usbRequest.bmRequestType))
		DBG2(",STD");
	else if (USB_TYPE_IS_CLASS(usbRequest.bmRequestType))
		DBG2(",CLS");
	else if (USB_TYPE_IS_VENDOR(usbRequest.bmRequestType))
		DBG2(",VEN");
		
	if (USB_RECP_IS_DEVICE(usbRequest.bmRequestType))
		DBG2(",DEV");
	else if (USB_RECP_IS_INTERFACE(usbRequest.bmRequestType))
		DBG2(",INT");
	else if (USB_RECP_IS_ENDPOINT(usbRequest.bmRequestType))
		DBG2(",END");
		
	DBG(") rq: 0x%02x v: 0x%04x i: 0x%04x l: %u",
	    usbRequest.bRequest, usbRequest.wValue.word, usbRequest.wIndex.word, usbRequest.wLength);
#endif
}

// Functions
#define USB_STRING_DESCRIPTOR_LEN(strLen) ((strLen) * 2 + 2)

void USB_PMA2userspace(uint8_t* dst, uint32_t offset, uint16_t len)
{
	len = (len + 1) / 2;
	uint32_t src = PMA_ADDR + 2 * offset;
	while (len)
	{
		uint16_t t1 = *(uint16_t*)src;
		*(uint16_t*)dst = t1;
		src += 4;
		dst += 2;
		len--;
	}
}
void USB_userspace2PMA(uint32_t offset, uint8_t* src, uint16_t len)
{
	len = (len + 1) / 2;
	uint32_t dst = PMA_ADDR + 2 * offset;
	while (len)
	{
		uint16_t t1 = *(uint16_t*)src;
		*(uint16_t*)dst = t1;
		src += 2;
		dst += 4;
		len--;
	}
}

// USB
static inline void USB_enableDevice()
{
	USB->DADDR = USB_DADDR_EF;
}
static inline void USB_disableDevice()
{
	USB->DADDR = 0;
}
static inline void USB_setAddress(uint8_t addr)
{
	USB->DADDR = USB_DADDR_EF | addr;
}

// ISTR
static inline uint8_t USB_get_ISTR_DIR()
{
	return (USB->ISTR >> 4) & 0x01;
}
static inline uint8_t USB_get_ISTR_EP_ID()
{
	return USB->ISTR & 0x0f;
}

// Endpoint control register management
#define USB_EPxR_MASK (USB_EP0R_STAT_RX | USB_EP0R_STAT_TX | USB_EP0R_DTOG_RX | USB_EP0R_DTOG_TX)
void USB_set_STAT_TX(uint8_t ep, uint8_t v)
{
	uint16_t val = EPxR(ep) & ~USB_EPxR_MASK;
	if (((v >> 0) & 0x01) ^ ((EPxR(ep) >> 4) & 0x01))
		val |= 1 << 4;
	if (((v >> 1) & 0x01) ^ ((EPxR(ep) >> 5) & 0x01))
		val |= 1 << 5;
	EPxR(ep) = val;
}
void USB_set_STAT_RX(uint8_t ep, uint8_t v)
{
	uint16_t val = EPxR(ep) & ~USB_EPxR_MASK;
	if (((v >> 0) & 0x01) ^ ((EPxR(ep) >> 12) & 0x01))
		val |= (1 << 12);
	if (((v >> 1) & 0x01) ^ ((EPxR(ep) >> 13) & 0x01))
		val |= (1 << 13);
	EPxR(ep) = val;
}

static inline void USB_clear_CTR_RX(uint8_t ep)
{
	EPxR(ep) = EPxR(ep) & ~USB_EPxR_MASK & ~USB_EP0R_CTR_RX;
}
static inline void USB_clear_CTR_TX(uint8_t ep)
{
	EPxR(ep) = EPxR(ep) & ~USB_EPxR_MASK & ~USB_EP0R_CTR_TX;
}
static inline uint8_t USB_get_CTR_RX(uint8_t ep)
{
	return EPxR(ep) & USB_EP0R_CTR_RX ? 1 : 0;
}
static inline uint8_t USB_get_CTR_TX(uint8_t ep)
{
	return EPxR(ep) & USB_EP0R_CTR_TX ? 1 : 0;
}
static inline uint8_t USB_get_SETUP(uint8_t ep)
{
	return EPxR(ep) & USB_EP0R_SETUP ? 1 : 0;
}
static inline void USB_setEPType(uint8_t ep, uint8_t type)
{
	EPxR(ep) = EPxR(ep) & ~USB_EPxR_MASK & ~(0x03 << 9) | (type << 9);
}
static inline void USB_setEPKind(uint8_t ep, uint8_t kind)
{
	EPxR(ep) = EPxR(ep) & ~USB_EPxR_MASK & ~(0x01 << 8) | (kind << 8);
}
static inline void USB_setEPNum(uint8_t ep, uint8_t num)
{
	EPxR(ep) = EPxR(ep) & ~USB_EPxR_MASK & ~0x0f | num;
}
static inline void USB_setEPNumTypeKind(uint8_t ep, uint8_t num, uint8_t type, uint8_t kind)
{
	EPxR(ep) = num | (type << 9) | (kind << 8);
}

// Endpoint buffer info management
static inline void USB_setEPTxAddr(uint8_t ep, uint16_t addr)
{
	EPxR_TX_ADDR(ep) = addr;
}
static inline void USB_setEPTxCount(uint8_t ep, uint8_t count)
{
	EPxR_TX_COUNT(ep) = count;
}
static inline void USB_setEPRxAddr(uint8_t ep, uint16_t addr)
{
	EPxR_RX_ADDR(ep) = addr;
}
static inline void USB_setEPRxBlocks2(uint8_t ep, uint8_t blocks)
{
	EPxR_RX_COUNT(ep) = blocks << 10;
}
static inline void USB_setEPRxBlocks32(uint8_t ep, uint8_t blocks)
{
	EPxR_RX_COUNT(ep) = 0x8000 | (blocks << 10);
}
static inline uint8_t USB_getEPRxCount(uint8_t ep)
{
	return EPxR_RX_COUNT(ep) & 0x01ff;
}
static inline uint8_t USB_getEPTxAddr(uint8_t ep)
{
	return EPxR_TX_ADDR(ep);
}
static inline uint8_t USB_getEPRxAddr(uint8_t ep)
{
	return EPxR_RX_ADDR(ep);
}

// Transfer handling
void USB_setDataToSend(uint8_t* data, uint16_t len)
{
	usbData = data;
	USB_dataLen = len;
}
void USB_send0LengthFrame()
{
	USB_setEPTxCount(0, 0);
	USB_set_STAT_TX(0, USB_EP_VALID);
}

#endif
