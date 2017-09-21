#include <sancus_support/sm_io.h>
#include "sm_eval.h"

int main (void)
{
    msp430_io_init();

    sancus_enable(&sm_eval);
    pr_sm_info(&sm_eval);

    #ifdef CAN_DRV_SM
        extern struct SancusModule sm_mmio_spi;
        sancus_enable(&sm_mmio_spi);
        pr_sm_info(&sm_mmio_spi);
    #endif

    eval_run();
    EXIT();
}
