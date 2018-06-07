//==============================================================================
// S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT85 Sample Code
// File      :  sht85.c
// Author    :  RFU
// Date      :  17-Mai-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.25.2.0
// Compiler  :  Armcc
// Brief     :  Sensor Layer: Implementation of functions for sensor access.
//==============================================================================

#include "sht85.h"
#include "i2c_hal.h"
#include "system.h"

#define CRC_POLYNOMIAL  0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001
#define I2C_ADDR        0x44

static etError StartWriteAccess(void);
static etError StartReadAccess(void);
static void StopAccess(void);
static etError WriteCommand(etCommands command);
static etError Read2BytesAndCrc(uint16_t* data, bool finAck, uint8_t timeout);
static uint8_t CalcCrc(uint8_t data[], uint8_t nbrOfBytes);
static etError CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum);
static float CalcTemperature(uint16_t rawValue);
static float CalcHumidity(uint16_t rawValue);

//------------------------------------------------------------------------------
void SHT85_Init(void)
{
  I2c_Init(); // init I2C
}

//------------------------------------------------------------------------------
etError SHT85_ReadSerialNumber(uint32_t* serialNumber)
{
  etError error; // error code
  uint16_t serialNumWords[2];
  
  error = StartWriteAccess();
  
  // write "read serial number" command
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_READ_SERIALNBR);
  }
  
  // if no error, start read access
  if(error == NO_ERROR) {
    error = StartReadAccess();
  }
  
  // if no error, read first serial number word
  if(error == NO_ERROR) {
    error = Read2BytesAndCrc(&serialNumWords[0], true, 100);
  }
  
  // if no error, read second serial number word
  if(error == NO_ERROR) {
    error = Read2BytesAndCrc(&serialNumWords[1], false, 0);
  }
  
  StopAccess();
  
  // if no error, calc serial number as 32-bit integer
  if(error == NO_ERROR) {
    *serialNumber = (serialNumWords[0] << 16) | serialNumWords[1];
  }
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_ReadStatus(uint16_t* status)
{
  etError error; // error code

  error = StartWriteAccess();
  
  // if no error, write "read status" command
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_READ_STATUS);
  }
  
  // if no error, start read access
  if(error == NO_ERROR) {
    error = StartReadAccess();
  }
  
  // if no error, read status
  if(error == NO_ERROR) {
    error = Read2BytesAndCrc(status, false, 0);
  }
  
  StopAccess();
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_ClearAllAlertFlags(void)
{
  etError error; // error code
  
  error = StartWriteAccess();
  
  // if no error, write clear status register command
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_CLEAR_STATUS);
  }
  
  StopAccess();
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_SingleMeasurment(float* temperature, float* humidity,
                               etSingleMeasureModes measureMode,
                               uint8_t timeout)
{
  etError  error;           // error code
  uint16_t rawValueTemp;    // temperature raw value from sensor
  uint16_t rawValueHumi;    // humidity raw value from sensor
  
  error  = StartWriteAccess();
  
  // if no error
  if(error == NO_ERROR) {
    // start measurement
    error = WriteCommand((etCommands)measureMode);
  }
  
  // if no error, wait until measurement ready
  if(error == NO_ERROR) {
    // poll every 1ms for measurement ready until timeout
    while(timeout--) {
      // check if the measurement has finished
      error = StartReadAccess();
      
      // if measurement has finished -> exit loop
      if(error == NO_ERROR) break;
      
      // delay 1ms
      System_DelayUs(1000);
    }
    
    // check for timeout error
    if(timeout == 0) {
      error = TIMEOUT_ERROR;
    }
  }
  
  // if no error, read temperature and humidity raw values
  if(error == NO_ERROR) {
    error |= Read2BytesAndCrc(&rawValueTemp, true, 0);
    error |= Read2BytesAndCrc(&rawValueHumi, false, 0);
  }
  
  StopAccess();
  
  // if no error, calculate temperature in °C and humidity in %RH
  if(error == NO_ERROR) {
    *temperature = CalcTemperature(rawValueTemp);
    *humidity = CalcHumidity(rawValueHumi);
  }
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_StartPeriodicMeasurment(etPeriodicMeasureModes measureMode)
{
  etError error; // error code
  
  error = StartWriteAccess();
  
  // if no error, start periodic measurement 
  if(error == NO_ERROR) {
    error = WriteCommand((etCommands)measureMode);
  }
  
  StopAccess();
  
  return error;
}


//------------------------------------------------------------------------------
etError SHT85_StopPeriodicMeasurment(void)
{
  etError error; // error code
  
  error = StartWriteAccess();
  
  // if no error, write breake command
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_BREAK);
  }
  
  StopAccess();
  
  return error;
}


//------------------------------------------------------------------------------
etError SHT85_ReadMeasurementBuffer(float* temperature, float* humidity)
{
  etError  error;        // error code
  uint16_t rawValueTemp; // raw temperature from sensor
  uint16_t rawValueHumi; // raw humidity from sensor
  
  error = StartWriteAccess();
  
  // if no error, read measurements
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_FETCH_DATA);
  }
  
  if(error == NO_ERROR) {
    error = StartReadAccess();  
  }
  
  if(error == NO_ERROR) {
    error = Read2BytesAndCrc(&rawValueTemp, true, 0);
  }
  
  if(error == NO_ERROR) {
    error = Read2BytesAndCrc(&rawValueHumi, false, 0);
  }
  
  // if no error, calculate temperature in °C and humidity in %RH
  if(error == NO_ERROR) {
    *temperature = CalcTemperature(rawValueTemp);
    *humidity = CalcHumidity(rawValueHumi);
  }
  
  StopAccess();
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_EnableHeater(void)
{
  etError error; // error code
  
  error = StartWriteAccess();
  
  // if no error, write heater enable command
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_HEATER_ENABLE);
  }
  
  StopAccess();
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_DisableHeater(void)
{
  etError error; // error code
  
  error = StartWriteAccess();
  
  // if no error, write heater disable command
  if(error == NO_ERROR) {
    error = WriteCommand(CMD_HEATER_DISABLE);
  }
  
  StopAccess();
  
  return error;
}

//------------------------------------------------------------------------------
etError SHT85_SoftReset(void)
{
  etError error; // error code
  
  error = StartWriteAccess();
  
  // write reset command
  if(error == NO_ERROR) {
    error  = WriteCommand(CMD_SOFT_RESET);
  }
  
  StopAccess();
  
  // if no error, wait 50 ms after reset
  if(error == NO_ERROR) {
    System_DelayUs(50000);
  }
  
  return error;
}

//------------------------------------------------------------------------------
static etError StartWriteAccess(void)
{
  etError error; // error code
  
  // write a start condition
  I2c_StartCondition();
  
  // write the sensor I2C address with the write flag
  error = I2c_WriteByte(I2C_ADDR << 1);
  
  return error;
}

//------------------------------------------------------------------------------
static etError StartReadAccess(void)
{
  etError error; // error code
  
  // write a start condition
  I2c_StartCondition();
  
  // write the sensor I2C address with the read flag
  error = I2c_WriteByte(I2C_ADDR << 1 | 0x01);
  
  return error;
}

//------------------------------------------------------------------------------
static void StopAccess(void)
{
  // write a stop condition
  I2c_StopCondition();
}

//------------------------------------------------------------------------------
static etError WriteCommand(etCommands command)
{
  etError error; // error code
  
  // write the upper 8 bits of the command to the sensor
  error = I2c_WriteByte(command >> 8);
  
  // write the lower 8 bits of the command to the sensor
  error |= I2c_WriteByte(command & 0xFF);
  
  return error;
}

//------------------------------------------------------------------------------
static etError Read2BytesAndCrc(uint16_t* data, bool finAck, uint8_t timeout)
{
  etError error;    // error code
  uint8_t bytes[2]; // read data array
  uint8_t checksum; // checksum byte
  
  // read two data bytes and one checksum byte
  bytes[0] = I2c_ReadByte(ACK);
  bytes[1] = I2c_ReadByte(ACK);
  checksum = I2c_ReadByte(finAck ? ACK : NO_ACK);

  // verify checksum
  error = CheckCrc(bytes, 2, checksum);
  
  // combine the two bytes to a 16-bit value
  *data = (bytes[0] << 8) | bytes[1];
  
  return error;
}

//------------------------------------------------------------------------------
static uint8_t CalcCrc(uint8_t data[], uint8_t nbrOfBytes)
{
  uint8_t bit;        // bit mask
  uint8_t crc = 0xFF; // calculated checksum
  uint8_t byteCtr;    // byte counter
  
  // calculates 8-Bit checksum with given polynomial
  for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit) {
      if(crc & 0x80) {
        crc = (crc << 1) ^ CRC_POLYNOMIAL;
      } else {
        crc = (crc << 1);
      }
    }
  }
  
  return crc;
}

//------------------------------------------------------------------------------
static etError CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
  // calculates 8-Bit checksum
  uint8_t crc = CalcCrc(data, nbrOfBytes);
  
  // verify checksum
  return (crc != checksum) ? CHECKSUM_ERROR : NO_ERROR;
}

//------------------------------------------------------------------------------
static float CalcTemperature(uint16_t rawValue)
{
  // calculate temperature [°C]
  // T = -45 + 175 * rawValue / (2^16-1)
  return 175.0f * (float)rawValue / 65535.0f - 45.0f;
}

//------------------------------------------------------------------------------
static float CalcHumidity(uint16_t rawValue)
{
  // calculate relative humidity [%RH]
  // RH = rawValue / (2^16-1) * 100
  return 100.0f * (float)rawValue / 65535.0f;
}
