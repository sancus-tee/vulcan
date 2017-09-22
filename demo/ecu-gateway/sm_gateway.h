#ifndef SM_GATEWAY_H_INC
#define SM_GATEWAY_H_INC

#include <sancus/sm_support.h>

extern struct SancusModule sm_gateway, sm_mmio_spi, sm_mmio_led;

void SM_ENTRY(sm_gateway) sm_gateway_run( void );

#endif
