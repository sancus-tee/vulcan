#ifndef SM_MMIO_LED_H_INC
#define SM_MMIO_LED_H_INC

#include <stdint.h>
#include <sancus/sm_support.h>

extern struct SancusModule sm_mmio_led;

void SM_MMIO_ENTRY(sm_mmio_led) sm_mmio_led_write(uint16_t, uint16_t,
                                                  uint16_t, uint16_t);

#endif
