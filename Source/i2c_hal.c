//==============================================================================
// S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT85 Sample Code
// File      :  i2c_hal.c
// Author    :  RFU
// Date      :  17-Mai-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.25.2.0
// Compiler  :  Armcc
// Brief     :  I2C hardware abstraction layer
//==============================================================================

#include "i2c_hal.h"
#include "system.h"

//-- Defines for IO-Pins -------------------------------------------------------
// SDA on port B, bit 9
/* -- adapt this code for your platform -- */
#define SDA_LOW()  (GPIOB->BSRR = 0x02000000) // set SDA to low
#define SDA_OPEN() (GPIOB->BSRR = 0x00000200) // set SDA to open-drain
#define SDA_READ   (GPIOB->IDR  & 0x0200)     // read SDA

// SCL on port B, bit 8
/* -- adapt this code for your platform -- */
#define SCL_LOW()  (GPIOB->BSRR = 0x01000000) // set SCL to low
#define SCL_OPEN() (GPIOB->BSRR = 0x00000100) // set SCL to open-drain
#define SCL_READ   (GPIOB->IDR  & 0x0100)     // read SCL

//------------------------------------------------------------------------------
/* -- adapt this code for your platform -- */
void I2c_Init(void)
{
  RCC->APB2ENR |= 0x00000008;  // I/O port B clock enabled

  SDA_OPEN();                  // I2C-bus idle mode SDA released
  SCL_OPEN();                  // I2C-bus idle mode SCL released

  // SDA on port B, bit 9
  // SCL on port B, bit 8
  GPIOB->CRH   &= 0xFFFFFF00;  // set open-drain output for SDA and SCL
  GPIOB->CRH   |= 0x00000055;  //
}

//------------------------------------------------------------------------------
void I2c_StartCondition(void)
{
  SDA_OPEN();
  System_DelayUs(2);
  SCL_OPEN();
  System_DelayUs(2);
  SDA_LOW();
  System_DelayUs(10);
  SCL_LOW();
  System_DelayUs(10);
}

//------------------------------------------------------------------------------
void I2c_StopCondition(void)
{
  SCL_LOW();
  System_DelayUs(2);
  SDA_LOW();
  System_DelayUs(2);
  SCL_OPEN();
  System_DelayUs(10);
  SDA_OPEN();
  System_DelayUs(10);
}

//------------------------------------------------------------------------------
etError I2c_WriteByte(uint8_t txByte)
{
  etError error = NO_ERROR;

  for(uint8_t mask = 0x80; mask > 0; mask >>= 1) {
    // masking txByte
    if((mask & txByte) == 0) {
      SDA_LOW();  // write 0 to SDA-Line
    } else {
      SDA_OPEN(); // write 1 to SDA-Line
    }

    // data set-up time
    System_DelayUs(2);

    // generate clock pulse on SCL
    SCL_OPEN();
    System_DelayUs(10);
    SCL_LOW();

    // data hold time
    System_DelayUs(2);
  }

  // release SDA-line
  SDA_OPEN();

  // ack reading
  SCL_OPEN();
  System_DelayUs(2);

  // check ack from i2c slave
  if(SDA_READ) {
    error = ACK_ERROR;
  }

  SCL_LOW();
  
  // wait to see byte package on scope
  System_DelayUs(20);

  return error;
}

//------------------------------------------------------------------------------
uint8_t I2c_ReadByte(etI2cAck ack)
{
  uint8_t rxByte = 0;

  // release SDA-line
  SDA_OPEN();

  for(uint8_t mask = 0x80; mask > 0; mask >>= 1) {
    SCL_OPEN();
    System_DelayUs(6);

    // read bit
    if(SDA_READ) {
      rxByte = rxByte | mask;
    }

    SCL_LOW();
    System_DelayUs(2);
  }

  // send acknowledge if necessary
  if(ack == ACK) {
    SDA_LOW();
  } else {
    SDA_OPEN();
  }

  // data set-up time
  System_DelayUs(2);

  // generate clock pulse on SCL
  SCL_OPEN();
  System_DelayUs(10);
  SCL_LOW();

  // release SDA-line
  SDA_OPEN();
  
  // wait to see byte package on scope
  System_DelayUs(20);

  return rxByte;
}

//------------------------------------------------------------------------------
etError I2c_GeneralCallReset(void)
{
  etError error;
  
  I2c_StartCondition();
  
  error = I2c_WriteByte(0x00);
  
  if(error == NO_ERROR) {
    error = I2c_WriteByte(0x06);
  }
  
  return error;
}
