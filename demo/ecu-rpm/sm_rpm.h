#ifndef SM_RPM_H_INC
#define SM_RPM_H_INC

#include <sancus/sm_support.h>

extern struct SancusModule sm_rpm;
extern struct SancusModule sm_mmio_led;

void SM_ENTRY(sm_rpm) rpm_run(void);

#endif
