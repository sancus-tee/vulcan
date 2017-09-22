#ifdef VATICAN_INCLUDE_NONCE_GENERATOR

#include "nonce_generator.h"
#include <stdlib.h>
#include <sancus_support/sm_io.h>

void nonce_generator_init(void)
{
    srand(100);
}

/*
 * NOTE: we use an _unprotected_ random source, as VatiCAN's nonce renewal
 * scheme supposes a HW-assisted spoofing protection mechanism anyway..
 */
uint32_t nonce_generator_run(ican_t *ican)
{
    int rv;
    uint32_t tmp, nonce;
    uint8_t buf[4];

    /* 1. Generate new random global nonce. */
    nonce = (tmp = rand()) << 16;
    nonce |= rand();

    /* 2. Broadcast nonce update. */
    buf[0] = nonce >> 24;
    buf[1] = nonce >> 16;
    buf[2] = nonce >> 8;
    buf[3] = nonce;
    rv = ican_send(ican, VATICAN_ID_NONCE_GENERATOR, buf, 4,
           /*block=*/1);
    ASSERT(rv >= 0);

    return nonce;
}

#endif
