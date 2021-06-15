#ifndef __PUBLIC_L_H__
#define __PUBLIC_L_H__

#include "stm32l1xx.h"

#define BITBAND_SYSTEM(addr,bit) (uint32_t*)(0x42000000 + ((uint32_t)(addr) - 0x40000000) * 32 + (bit) * 4)

// IO
#define PORTCR_FROM_PORTPIN(port,pin) *(uint32_t*)((uint32_t)port + 0x04 * (pin / 8))

#define IO_SPEED_400kHZ 0b00
#define IO_SPEED_2MHZ   0b01
#define IO_SPEED_10MHZ  0b10
#define IO_SPEED_50MHZ  0b11
static void IO_SET_MODE_TYPE_PP (GPIO_TypeDef* port, int pin, uint32_t mode, uint32_t type, uint32_t pp)
{
	port->MODER &= ~(0b11 << ((pin) * 2));
	port->MODER |= mode << ((pin) * 2);

	port->OTYPER &= ~(0b1 << pin);
	port->OTYPER |= type << pin;

	port->PUPDR &= ~(0b11 << ((pin) * 2));
	port->PUPDR |= pp << ((pin) * 2);

	port->OSPEEDR &= ~(0b11 << ((pin) * 2));
	port->OSPEEDR |= IO_SPEED_400kHZ << ((pin) * 2);
}
static void IO_SET_MODE (GPIO_TypeDef* port, int pin, uint32_t mode)
{
	port->MODER &= ~(0b11 << ((pin) * 2));
	port->MODER |= mode << ((pin) * 2);
}

static inline void IO_PUSH_PULL (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b01, 0, 0b00);
}
static inline void IO_PUSH_PULL_MODE (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE (port, pin, 0b01);
}
static inline void IO_OPEN_DRAIN (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b01, 1, 0b00);
}
static inline void IO_ALT_PUSH_PULL (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b10, 0, 0b00);
}
static inline void IO_ALT_PUSH_PULL_MODE (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE (port, pin, 0b10);
}
static inline void IO_ALT_OPEN_DRAIN (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b10, 1, 0b00);
}
static inline void IO_ALT_SET (GPIO_TypeDef* port, int pin, uint32_t AF)
{
	port->AFR[pin / 8] &= ~((0b1111) << ((pin % 8) * 4));
	port->AFR[pin / 8] |= AF << ((pin % 8) * 4);
}
static inline void IO_ANALOG (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b11, 0, 0b00);
}
static inline void IO_INPUT (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b00, 0, 0b00);
}
static inline void IO_INPUT_PU (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b00, 0, 0b01);
}
static inline void IO_INPUT_PD (GPIO_TypeDef* port, int pin)
{
	IO_SET_MODE_TYPE_PP (port, pin, 0b00, 0, 0b10);
}

static inline void IO_HIGH (GPIO_TypeDef* port, int pin)
{
	// port->ODR |= (1 << pin);
	
	//volatile uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	//*addr = 1;
	
	port->BSRRL = 1 << pin;
}
static inline void IO_LOW (GPIO_TypeDef* port, int pin)
{
	// port->ODR &= ~(1 << pin);
	
	//volatile uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	//*addr = 0;
	
	port->BSRRH = 1 << pin;
}
static inline void IO_TOGGLE (GPIO_TypeDef* port, int pin)
{
	//port->ODR ^= (1 << pin);

	volatile uint32_t *addr = (volatile uint32_t*)(0x42000000 + ((uint32_t)&port->ODR - 0x40000000) * 32 + pin * 4);
	*addr = *addr ^ 1;
}

static inline uint8_t IO_IS_HIGH (GPIO_TypeDef* port, int pin)
{
	return (port->IDR & (1 << pin)) ? 1 : 0;
	// uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->IDR - 0x40000000) * 32 + pin * 4);
	// return *addr == 0 ? 0 : 1;
}
static inline uint8_t IO_IS_LOW (GPIO_TypeDef* port, int pin)
{
	return (port->IDR & (1 << pin)) == 0;
	// uint32_t *addr = (uint32_t*)(0x42000000 + ((uint32_t)&port->IDR - 0x40000000) * 32 + pin * 4);
	// return *addr == 0 ? 1 : 0;
}

#define EXTI_PORTA 0b0000
#define EXTI_PORTB 0b0001
#define EXTI_PORTC 0b0010
#define EXTI_PORTD 0b0011
#define EXTI_PORTE 0b0100
#define EXTI_PORTF 0b0110
#define EXTI_PORTG 0b0111
#define EXTI_PORTH 0b0101

#define SET_EXTICR(irq,port) { \
	SYSCFG->EXTICR[(irq) / 4] &= ~(0b1111 << (((irq) % 4) * 4)); \
	SYSCFG->EXTICR[(irq) / 4] |= (port) << (((irq) % 4) * 4); }

#endif
