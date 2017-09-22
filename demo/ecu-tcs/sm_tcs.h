#ifndef SM_SEND_H_INC
#define SM_SEND_H_INC

#include <sancus/sm_support.h>

extern struct SancusModule sm_tcs, sm_mmio_kypd;

int SM_ENTRY(sm_tcs) tcs_run(void);

#endif
