#ifndef __I2C_H__
#define __I2C_H__

#include <public.h>

#define I2C_SUCCESS         0
#define I2C_ERROR           1
#define I2C_ERROR_START     2
#define I2C_ERROR_ADDR      3
#define I2C_SLAVE_NOT_FOUND 4

void i2cInit100kHz(I2C_TypeDef* I2C_OBJ);
void i2cInit200kHz(I2C_TypeDef* I2C_OBJ);
void i2cDeinit(I2C_TypeDef* I2C_OBJ);
  
uint8_t i2cStart(I2C_TypeDef* I2C_OBJ, uint8_t addr);
// uint8_t i2cStop ();
uint8_t i2cWrite(I2C_TypeDef* I2C_OBJ, uint8_t data);
uint8_t i2cRead(I2C_TypeDef* I2C_OBJ, uint8_t* data);
void i2cSetACK(I2C_TypeDef* I2C_OBJ);
void i2cSetNACK(I2C_TypeDef* I2C_OBJ);
void i2cSetStop(I2C_TypeDef* I2C_OBJ);
uint8_t i2cWaitUntilStop(I2C_TypeDef* I2C_OBJ);

uint8_t i2cWriteData(I2C_TypeDef* I2C_OBJ, uint8_t addr, uint8_t reg, const uint8_t *buffer, uint8_t cnt);
uint8_t i2cReadData(I2C_TypeDef* I2C_OBJ, uint8_t addr, uint8_t reg, uint8_t *buffer, uint8_t cnt);
uint8_t i2cWriteDataNoReg(I2C_TypeDef* I2C_OBJ, uint8_t addr, const uint8_t *buffer, uint8_t cnt);
uint8_t i2cReadDataNoReg(I2C_TypeDef* I2C_OBJ, uint8_t addr, uint8_t *buffer, uint8_t cnt);

#endif
