#ifndef __DELAY_H__
#define __DELAY_H__

#include "public.h"
#include "stm32f10x.h"
#include "../settings.h"
#define F_CPU 8000000
#ifndef F_CPU
#	error F_CPU must be defined
#endif

// TIM2, TIM3, ...
#ifndef DELAY_TIMER
#	error DELAY_TIMER must be defined
#endif
// APB1ENR, ...
#ifndef DELAY_TIMER_APB
#	error DELAY_TIMER_APB must be defined
#endif
// APB1ENR_TIM2EN, ...
#ifndef DELAY_TIMER_APBEN
#	error DELAY_TIMER_APBEN must be defined
#endif

static inline void _delay_init ()
{
	RCC->DELAY_TIMER_APB |= DELAY_TIMER_APBEN;
}

void _delay_100us (uint16_t d);
void _delay_ms (uint16_t d);
void _delay_us (uint16_t d);

#endif
