//==============================================================================
// S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT85 Sample Code
// File      :  main.c
// Author    :  RFU
// Date      :  17-Mai-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.25.2.0
// Compiler  :  Armcc
// Brief     :  This code shows how to implement the basic commands for the
//              SHT85 sensor chip.
//              Due to compatibility reasons the I2C interface is implemented
//              as "bit-banging" on normal I/O's. This code is written for an
//              easy understanding and is neither optimized for speed nor code
//              size.
//
// Porting to a different microcontroller (uC):
//   - adapt the port functions / definitions for your uC     in i2c_hal.h/.c
//   - adapt the timing of the delay function for your uC     in system.c
//   - change the uC register definition file <stm32f10x.h>   in system.h
//   - adapt the led functions for your platform              in main.c
//==============================================================================

#include "sht85.h"
#include "system.h"
#include <stdint.h>
#include <stdbool.h>

static void LedInit(void);
static void LedBlue(bool on);
static void LedGreen(bool on);

//------------------------------------------------------------------------------
int main(void)
{
  etError  error;          // error code
  uint32_t serialNumber;   // serial number
  float    temperature;    // temperature [°C]
  float    humidity;       // relative humidity [%RH]
  
  LedInit();
  SHT85_Init();
  
  // wait 50ms after power on
  System_DelayUs(50000);    
  
  // demonstartion of SoftReset command
  error = SHT85_SoftReset();
  
  // demonstartion of ReadSerialNumber command
  error = SHT85_ReadSerialNumber(&serialNumber);
  
  // demonstration of the single shot measurement
  // measurement with high repeatability
  error = SHT85_SingleMeasurment(&temperature, &humidity, SINGLE_MEAS_HIGH, 50);
  
  // --- demonstration of the periodic measurement mode ---
  while(1) {
    // start periodic measurement, with high repeatability and 1 measurements
    // per second
    error = SHT85_StartPeriodicMeasurment(PERI_MEAS_HIGH_1_HZ);
    
    // if no error occurs, switch green LED on
    LedGreen(error == NO_ERROR);
    
    // loop while no error
    while(error == NO_ERROR) {
      // read measurment buffer
      error = SHT85_ReadMeasurementBuffer(&temperature, &humidity);
      
      if(error == NO_ERROR) {
        // if the Relative Humidity is over 50% -> the blue LED lights up
        LedBlue(humidity > 50);
      } else if (error == ACK_ERROR) {
        error = NO_ERROR;
        // there were no new values in the buffer -> ignore this error
      } else {
        // exit loop on all other errors
        break;
      }
      
      // wait 100ms
      System_DelayUs(100000);
    }
    
    // --- error handling ---
    // in case of an error, switch green LED off ...
    LedGreen(false);
    
    // ... and perfom a soft reset
    error = SHT85_SoftReset();
    
    // if the soft reset was not successful, perform an general call reset
    if(error != NO_ERROR) {
      error = I2c_GeneralCallReset();
    }
    
    // wait 100ms
    System_DelayUs(100000);
  }
}

//------------------------------------------------------------------------------
/* -- adapt this code for your platform -- */
static void LedInit(void)
{
  RCC->APB2ENR |= 0x00000010;  // I/O port C clock enabled
  GPIOC->CRH   &= 0xFFFFFF00;  // set general purpose output mode for LEDs
  GPIOC->CRH   |= 0x00000011;  //
  GPIOC->BSRR   = 0x03000000;  // LEDs off
}

//------------------------------------------------------------------------------
/* -- adapt this code for your platform -- */
static void LedBlue(bool on)
{
  if(on) {
    GPIOC->BSRR = 0x00000100;
  } else {
    GPIOC->BSRR = 0x01000000;
  }
}

//------------------------------------------------------------------------------
/* -- adapt this code for your platform -- */
static void LedGreen(bool on)
{
  if(on) {
    GPIOC->BSRR = 0x00000200;
  } else {
    GPIOC->BSRR = 0x02000000;
  }
}
