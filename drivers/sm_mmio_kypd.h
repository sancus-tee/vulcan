#ifndef SM_MMIO_KYPD_H_INC
#define SM_MMIO_KYPD_H_INC

#include <stdint.h>
#include <sancus/sm_support.h>

extern struct SancusModule sm_mmio_kypd;

// bitmap indicating which keys are down
typedef uint16_t key_state_t;
#define KYPD_NB_KEYS    16

key_state_t SM_MMIO_ENTRY(sm_mmio_kypd) sm_mmio_kypd_poll(void);
void SM_MMIO_ENTRY(sm_mmio_kypd) sm_mmio_kypd_init(void);

#endif
