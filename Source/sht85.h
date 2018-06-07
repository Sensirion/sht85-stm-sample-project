//==============================================================================
// S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT85 Sample Code
// File      :  sht85.h
// Author    :  RFU
// Date      :  17-Mai-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.25.2.0
// Compiler  :  Armcc
// Brief     :  Sensor Layer: Definitions of commands and functions for sensor
//                            access.
//==============================================================================

#ifndef SHT85_H
#define SHT85_H

#include "i2c_hal.h"
#include "system.h"
#include <stdint.h>
#include <stdbool.h>

// Sensor Commands
typedef enum {
  CMD_READ_SERIALNBR = 0x3780, // read serial number
  CMD_READ_STATUS    = 0xF32D, // read status register
  CMD_CLEAR_STATUS   = 0x3041, // clear status register
  CMD_HEATER_ENABLE  = 0x306D, // enabled heater
  CMD_HEATER_DISABLE = 0x3066, // disable heater
  CMD_SOFT_RESET     = 0x30A2, // soft reset
  CMD_MEAS_SINGLE_H  = 0x2400, // single meas., high repeatability
  CMD_MEAS_SINGLE_M  = 0x240B, // single meas., medium repeatability
  CMD_MEAS_SINGLE_L  = 0x2416, // single meas., low repeatability
  CMD_MEAS_PERI_05_H = 0x2032, // periodic meas. 0.5 mps, high repeatability
  CMD_MEAS_PERI_05_M = 0x2024, // periodic meas. 0.5 mps, medium repeatability
  CMD_MEAS_PERI_05_L = 0x202F, // periodic meas. 0.5 mps, low repeatability
  CMD_MEAS_PERI_1_H  = 0x2130, // periodic meas. 1 mps, high repeatability
  CMD_MEAS_PERI_1_M  = 0x2126, // periodic meas. 1 mps, medium repeatability
  CMD_MEAS_PERI_1_L  = 0x212D, // periodic meas. 1 mps, low repeatability
  CMD_MEAS_PERI_2_H  = 0x2236, // periodic meas. 2 mps, high repeatability
  CMD_MEAS_PERI_2_M  = 0x2220, // periodic meas. 2 mps, medium repeatability
  CMD_MEAS_PERI_2_L  = 0x222B, // periodic meas. 2 mps, low repeatability
  CMD_MEAS_PERI_4_H  = 0x2334, // periodic meas. 4 mps, high repeatability
  CMD_MEAS_PERI_4_M  = 0x2322, // periodic meas. 4 mps, medium repeatability
  CMD_MEAS_PERI_4_L  = 0x2329, // periodic meas. 4 mps, low repeatability
  CMD_MEAS_PERI_10_H = 0x2737, // periodic meas. 10 mps, high repeatability
  CMD_MEAS_PERI_10_M = 0x2721, // periodic meas. 10 mps, medium repeatability
  CMD_MEAS_PERI_10_L = 0x272A, // periodic meas. 10 mps, low repeatability
  CMD_FETCH_DATA     = 0xE000, // readout measurements for periodic mode
  CMD_BREAK          = 0x3093, // stop periodic measurement
} etCommands;

// Single Shot Measurement Repeatability
typedef enum {
  SINGLE_MEAS_LOW        = CMD_MEAS_SINGLE_L, // low repeatability
  SINGLE_MEAS_MEDIUM     = CMD_MEAS_SINGLE_M, // medium repeatability
  SINGLE_MEAS_HIGH       = CMD_MEAS_SINGLE_H  // high repeatability
} etSingleMeasureModes;

// Periodic Measurement Configurations
typedef enum {
  PERI_MEAS_LOW_05_HZ    = CMD_MEAS_PERI_05_L,
  PERI_MEAS_LOW_1_HZ     = CMD_MEAS_PERI_1_L,
  PERI_MEAS_LOW_2_HZ     = CMD_MEAS_PERI_2_L,
  PERI_MEAS_LOW_4_HZ     = CMD_MEAS_PERI_4_L,
  PERI_MEAS_LOW_10_HZ    = CMD_MEAS_PERI_10_L,
  PERI_MEAS_MEDIUM_05_HZ = CMD_MEAS_PERI_05_M,
  PERI_MEAS_MEDIUM_1_HZ  = CMD_MEAS_PERI_1_M,
  PERI_MEAS_MEDIUM_2_HZ  = CMD_MEAS_PERI_2_M,
  PERI_MEAS_MEDIUM_4_HZ  = CMD_MEAS_PERI_4_M,
  PERI_MEAS_MEDIUM_10_HZ = CMD_MEAS_PERI_10_M,
  PERI_MEAS_HIGH_05_HZ   = CMD_MEAS_PERI_05_H,
  PERI_MEAS_HIGH_1_HZ    = CMD_MEAS_PERI_1_H,
  PERI_MEAS_HIGH_2_HZ    = CMD_MEAS_PERI_2_H,
  PERI_MEAS_HIGH_4_HZ    = CMD_MEAS_PERI_4_H,
  PERI_MEAS_HIGH_10_HZ   = CMD_MEAS_PERI_10_H,
} etPeriodicMeasureModes;

//==============================================================================
// Initializes the I2C bus for communication with the sensor.
//------------------------------------------------------------------------------
void SHT85_Init(void);


//==============================================================================
// Reads the serial number from sensor.
//------------------------------------------------------------------------------
// input: serialNumber  pointer to serialNumber
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_ReadSerialNumber(uint32_t* serialNumber);


//==============================================================================
// Reads the status register from the sensor.
//------------------------------------------------------------------------------
// input: status        pointer to status
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_ReadStatus(uint16_t* status);


//==============================================================================
// Clears all alert flags in status register from sensor.
//------------------------------------------------------------------------------
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_ClearAllAlertFlags(void);


//==============================================================================
// Gets the temperature [°C] and the relative humidity [%RH] from the sensor.
// This function polls every 1ms until measurement is ready.
//------------------------------------------------------------------------------
// input: temperature   pointer to temperature
//        humiditiy     pointer to humidity
//        measureMode   repeatability for the measurement [low, medium, high]
//        timeout       polling timeout in milliseconds
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_SingleMeasurment(float* temperature, float* humidity,
                               etSingleMeasureModes measureMode,
                               uint8_t timeout);


//==============================================================================
// Starts periodic measurement.
//------------------------------------------------------------------------------
// input: measureMode   defines the repeatability for the measurement and the
//                      measurement frequency [0.5, 1, 2, 4, 10] Hz
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_StartPeriodicMeasurment(etPeriodicMeasureModes measureMode);


//==============================================================================
// Stops periodic measurement.
//------------------------------------------------------------------------------
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_StopPeriodicMeasurment(void);


//==============================================================================
// Reads last measurement from the sensor buffer
//------------------------------------------------------------------------------
// input: temperature   pointer to temperature
//        humidity      pointer to humidity
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_ReadMeasurementBuffer(float* temperature, float* humidity);


//==============================================================================
// Enables the heater on sensor
//------------------------------------------------------------------------------
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_EnableHeater(void);


//==============================================================================
// Disables the heater on sensor
//------------------------------------------------------------------------------
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_DisableHeater(void);


//==============================================================================
// Calls the soft reset mechanism that forces the sensor into a well-defined
// state without removing the power supply.
//------------------------------------------------------------------------------
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      NO_ERROR       = no error
//------------------------------------------------------------------------------
etError SHT85_SoftReset(void);


#endif
