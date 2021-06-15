#ifndef __USB_DEFS_H__
#define __USB_DEFS_H__

#define USB_BASE  0x40005C00  /* USB_IP Peripheral Registers base address */
#define PMA_ADDR  0x40006000  /* USB_IP Packet Memory Area base address   */

#include "../stm32f10x.h"

typedef struct
{
	__IO uint16_t EP0R;
  uint16_t  RESERVED5;
	__IO uint16_t EP1R;
  uint16_t  RESERVED6;
	__IO uint16_t EP2R;
  uint16_t  RESERVED7;
	__IO uint16_t EP3R;
  uint16_t  RESERVED8;
	__IO uint16_t EP4R;
  uint16_t  RESERVED9;
	__IO uint16_t EP5R;
  uint16_t  RESERVED10;
	__IO uint16_t EP6R;
  uint16_t  RESERVED11;
	__IO uint16_t EP7R;
  uint16_t  RESERVED12;

  uint8_t  RESERVED13[32];

  __IO uint16_t CNTR;
  uint16_t  RESERVED0;
  __IO uint16_t ISTR;
  uint16_t  RESERVED1;
  __IO uint16_t FNR;
  uint16_t  RESERVED2;
  __IO uint16_t DADDR;
  uint16_t  RESERVED3;
  __IO uint16_t BTABLE;
  uint16_t  RESERVED4;
} USB_TypeDef;

#define USB ((USB_TypeDef*)(USB_BASE))

#define EPxR(x) *(uint16_t*)(USB_BASE + (x) * 4)
#define EPxR_TX_ADDR(ep) *(uint16_t*)(PMA_ADDR + (ep) * 16)
#define EPxR_TX_COUNT(ep) *(uint16_t*)(PMA_ADDR + (ep) * 16 + 4)
#define EPxR_RX_ADDR(ep) *(uint16_t*)(PMA_ADDR + (ep) * 16 + 8)
#define EPxR_RX_COUNT(ep) *(uint16_t*)(PMA_ADDR + (ep) * 16 + 12)

#define USB_EP_DISABLED 0x00
#define USB_EP_STALL    0x01
#define USB_EP_NAK      0x02
#define USB_EP_VALID    0x03

#define USB_EP_BULK      0x00
#define USB_EP_CONTROL   0x01
#define USB_EP_ISO       0x02
#define USB_EP_INTERRUPT 0x03

#endif
