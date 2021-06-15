#include "i2c.h"

#include <hardware.h>
#include <settings.h>
#include <delay.h>
#include <myprintf.h>

void i2cInit100kHz(I2C_TypeDef* I2C_OBJ)
{
	I2C_OBJ->CR1 &= ~I2C_CR1_PE;
	I2C_OBJ->CR2 = (PCLK1_FREQ / 1000000);
	I2C_OBJ->CCR = 40 * (PCLK1_FREQ / 8000000);
	I2C_OBJ->TRISE = (PCLK1_FREQ / 1000000) + 1;
	I2C_OBJ->CR1 |= I2C_CR1_PE;
	
	// myprintf("cr2 0x%02x\r\nccr = 0x%02x\r\ntrise = 0x%02x\r\n", I2C_OBJ->CR2, I2C_OBJ->CCR, I2C_OBJ->TRISE);
}
void i2cInit200kHz(I2C_TypeDef* I2C_OBJ)
{
	I2C_OBJ->CR1 &= ~I2C_CR1_PE;
	I2C_OBJ->CR2 = (PCLK1_FREQ / 1000000);
	I2C_OBJ->CCR = 40 * (PCLK1_FREQ / 8000000 / 2);
	I2C_OBJ->TRISE = (PCLK1_FREQ / 1000000) + 1;
	I2C_OBJ->CR1 |= I2C_CR1_PE;
	
	// myprintf("cr2 0x%02x\r\nccr = 0x%02x\r\ntrise = 0x%02x\r\n", I2C_OBJ->CR2, I2C_OBJ->CCR, I2C_OBJ->TRISE);
}
void i2cDeinit(I2C_TypeDef* I2C_OBJ)
{
	I2C_OBJ->CR1 = 0;
	
	// IO_OPEN_DRAIN(SCL);
	// IO_OPEN_DRAIN(SDA);
	// IO_HIGH(SCL);
	// IO_HIGH(SDA);
	
	// int i;
	// for (i = 0; i < 10; i++)
	// {
	// IO_TOGGLE(SCL);
	// _delay_us (10);
	// }
	
	// IO_HIGH(SCL);
	// IO_HIGH(SDA);
}

// #define I2CD(x) myprintf(x"\r\n");

#ifndef I2CD
#define I2CD(x)
#endif

uint8_t i2cStart(I2C_TypeDef* I2C_OBJ, uint8_t addr)
{
	I2C_OBJ->CR1 |= I2C_CR1_START;
	uint16_t timeout = 10000;
	for (;;)
	{
		I2CD("s1");
		if (I2C_OBJ->SR1 & I2C_SR1_SB)
		{
			volatile uint8_t s2 = I2C_OBJ->SR2;
			break;
		}
		if (timeout-- == 0)
			return I2C_ERROR_START;
	}
	
	I2C_OBJ->DR = addr;
	timeout = 10000;
	for (;;)
	{
		I2CD("s2");
		if (I2C_OBJ->SR1 & I2C_SR1_ADDR)
		{
			volatile uint8_t s2 = I2C_OBJ->SR2;
			return I2C_SUCCESS;
		}
		if (I2C_OBJ->SR1 & I2C_SR1_AF)
		{
			I2C_OBJ->SR1 &= ~I2C_SR1_AF;
			i2cSetStop(I2C_OBJ);
			i2cWaitUntilStop(I2C_OBJ);
			return I2C_SLAVE_NOT_FOUND;
		}
		if (I2C_OBJ->SR1 & I2C_SR1_BERR)
		{
			return I2C_ERROR_ADDR;
		}
		if (timeout-- == 0)
			return I2C_ERROR_ADDR;
	}
	return I2C_ERROR_ADDR;
}
uint8_t i2cWrite(I2C_TypeDef* I2C_OBJ, uint8_t data)
{
	I2C_OBJ->DR = data;
	uint16_t timeout = 10000;
	while (!(I2C_OBJ->SR1 & I2C_SR1_TXE))
	{
		I2CD("w1");
		if (timeout-- == 0)
			return I2C_ERROR;
	}
	return I2C_SUCCESS;
}
uint8_t i2cRead(I2C_TypeDef* I2C_OBJ, uint8_t* data)
{
	uint16_t timeout = 10000;
	while (!(I2C_OBJ->SR1 & I2C_SR1_RXNE))
	{
		I2CD("r1");
		if (timeout-- == 0)
			return I2C_ERROR;
	}
	*data = I2C_OBJ->DR;
	return I2C_SUCCESS;
}
inline void i2cSetACK(I2C_TypeDef* I2C_OBJ)
{
	I2C_OBJ->CR1 |= I2C_CR1_ACK;
}
inline void i2cSetNACK(I2C_TypeDef* I2C_OBJ)
{
	I2C_OBJ->CR1 &= ~I2C_CR1_ACK;
}
inline void i2cSetStop(I2C_TypeDef* I2C_OBJ)
{
	I2C_OBJ->CR1 |= I2C_CR1_STOP;
}
inline uint8_t i2cWaitUntilStop(I2C_TypeDef* I2C_OBJ)
{
	uint16_t timeout = 10000;
	while ((I2C_OBJ->SR2 & I2C_SR2_MSL))
	{
		I2CD("u1");
		if (timeout-- == 0)
			return I2C_ERROR;
	}
	return I2C_SUCCESS;
}

uint8_t i2cWriteData(I2C_TypeDef* I2C_OBJ, uint8_t addr, uint8_t reg, const uint8_t *buffer, uint8_t cnt)
{
	if (i2cStart(I2C_OBJ, addr | 0)) return I2C_ERROR;
	i2cSetACK(I2C_OBJ);
	if (i2cWrite(I2C_OBJ, reg))      return I2C_ERROR;
	while (cnt--)
	{
		if (i2cWrite(I2C_OBJ, *buffer++)) return I2C_ERROR;
	}
	i2cSetStop(I2C_OBJ);
	if (i2cWaitUntilStop(I2C_OBJ)) return I2C_ERROR;
	return I2C_SUCCESS;
}
uint8_t i2cReadData(I2C_TypeDef* I2C_OBJ, uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t cnt)
{
	if (i2cStart(I2C_OBJ, addr | 0))   return I2C_ERROR;
	if (i2cWrite(I2C_OBJ, reg))        return I2C_ERROR;
	
	if (cnt == 1)
	{
		if (i2cStart(I2C_OBJ, addr | 1)) return I2C_ERROR;
		i2cSetNACK(I2C_OBJ);
		i2cSetStop(I2C_OBJ);
		if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
		if (i2cWaitUntilStop(I2C_OBJ)) return I2C_ERROR;
		return I2C_SUCCESS;
	}
	else
	{
		if (i2cStart(I2C_OBJ, addr | 1)) return I2C_ERROR;
		i2cSetACK(I2C_OBJ);
		for (;;)
		{
			if (cnt == 2)
			{
				if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
				i2cSetNACK(I2C_OBJ);
				i2cSetStop(I2C_OBJ);
				buffer++;
				if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
				if (i2cWaitUntilStop(I2C_OBJ)) return I2C_ERROR;
				return I2C_SUCCESS;
			}
			else
			{
				if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
				buffer++;
				cnt--;
			}
		}
	}
}
uint8_t i2cWriteDataNoReg(I2C_TypeDef* I2C_OBJ, uint8_t addr, const uint8_t *buffer, uint8_t cnt)
{
	if (i2cStart(I2C_OBJ, addr | 0)) return I2C_ERROR;
	i2cSetACK(I2C_OBJ);
	while (cnt--)
	{
		if (i2cWrite(I2C_OBJ, *buffer++)) return I2C_ERROR;
	}
	i2cSetStop(I2C_OBJ);
	if (i2cWaitUntilStop(I2C_OBJ)) return I2C_ERROR;
	return I2C_SUCCESS;
}
uint8_t i2cReadDataNoReg(I2C_TypeDef* I2C_OBJ, uint8_t addr, uint8_t *buffer, uint8_t cnt)
{
	if (cnt == 1)
	{
		if (i2cStart(I2C_OBJ, addr | 1)) return I2C_ERROR;
		i2cSetNACK(I2C_OBJ);
		i2cSetStop(I2C_OBJ);
		if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
		if (i2cWaitUntilStop(I2C_OBJ)) return I2C_ERROR;
		return I2C_SUCCESS;
	}
	else
	{
		if (i2cStart(I2C_OBJ, addr | 1)) return I2C_ERROR;
		i2cSetACK(I2C_OBJ);
		for (;;)
		{
			if (cnt == 2)
			{
				if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
				i2cSetNACK(I2C_OBJ);
				i2cSetStop(I2C_OBJ);
				buffer++;
				if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
				if (i2cWaitUntilStop(I2C_OBJ)) return I2C_ERROR;
				return I2C_SUCCESS;
			}
			else
			{
				if (i2cRead(I2C_OBJ, buffer))    return I2C_ERROR;
				buffer++;
				cnt--;
			}
		}
	}
}
