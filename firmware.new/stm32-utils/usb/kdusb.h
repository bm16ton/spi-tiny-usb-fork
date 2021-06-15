#ifndef __KDUSB_LIB_H__
#define __KDUSB_LIB_H__

#include <stdint.h>

#include "kdusb/kdusb_lib.h"

typedef union
{
	uint8_t bytes[2];
	uint16_t word;
} USBWord;

typedef struct
{
	uint8_t bmRequestType, bRequest;
	USBWord wValue;
	USBWord wIndex;
	uint16_t wLength;
} USB_Request;

uint8_t usbEP;
extern uint8_t *usbData;
extern USB_Request usbRequest;

extern void _errorloop();

void usbHandleLP();

void usbInit();
void usbDisconnect();
void usbConnect();
uint16_t usbFunctionSetup();
uint8_t usbPrepareUserData(uint16_t max);
void usbHandleData(uint8_t size);
void usbSetInterruptData(uint8_t ep, uint8_t* data, uint8_t len);

#endif
