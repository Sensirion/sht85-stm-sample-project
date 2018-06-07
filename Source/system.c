//==============================================================================
// S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT85 Sample Code
// File      :  system.c
// Author    :  RFU
// Date      :  17-Mai-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.25.2.0
// Compiler  :  Armcc
// Brief     :  System functions
//==============================================================================

#include "system.h"

//------------------------------------------------------------------------------
void System_Init(void) 
{
  // no initialization required
}

//------------------------------------------------------------------------------
void System_DelayUs(uint32_t nbrOfUs)   /* -- adapt this delay for your uC -- */
{
  uint32_t i;
  for(i = 0; i < nbrOfUs; i++) {  
    __nop();  // nop's may be added or removed for timing adjustment
    __nop();
    __nop();
    __nop();
  }
}
