#ifndef SM_LED_H_INC
#define SM_LED_H_INC

#include "sm_mmio_led.h"
#include <stdint.h>

#ifndef LED_SM
#error no LED application driver SM specified..
#else
    #define LED_FUNC    SM_FUNC(LED_SM)
    #define LED_DATA    SM_DATA(LED_SM)
#endif

void LED_FUNC led_update(char c1, char c2, char c3, char c4,
                         char c5, char c6, char c7, char c8);
#endif
