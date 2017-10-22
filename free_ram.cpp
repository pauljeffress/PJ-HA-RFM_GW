//
// ================ freeRam()
// 
//
#include "PJ-HA-RFM_GW.h" // My global defines and extern variables to help multi file comilation.



int freeRam () 
{
  //elow is only relevant on AVR
  //extern int __heap_start, *__brkval; 
  //int v; 
  //return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
