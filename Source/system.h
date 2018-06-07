//==============================================================================
// S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT85 Sample Code
// File      :  system.h
// Author    :  RFU
// Date      :  17-Mai-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.25.2.0
// Compiler  :  Armcc
// Brief     :  System functions, global definitions
//==============================================================================

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include "stm32f10x.h" // controller register definitions

// Error codes
typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
  CHECKSUM_ERROR = 0x02, // checksum mismatch error
  TIMEOUT_ERROR  = 0x04, // timeout error
} etError;

//==============================================================================
void SystemInit(void);
//==============================================================================
// Initializes the system
//------------------------------------------------------------------------------

//==============================================================================
void System_DelayUs(uint32_t nbrOfUs);
//==============================================================================
// Wait function for small delays.
//------------------------------------------------------------------------------
// input:  nbrOfUs   wait x times approx. one micro second (fcpu = 8MHz)
// return: -
// remark: smallest delay is approx. 15us due to function call

#endif
