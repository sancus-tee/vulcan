#include <sancus_support/sm_io.h>
#include "sm_eval.h"

int main (void)
{
    msp430_io_init();

    #ifdef TRAVIS_QUIET
        pr_info("travis_ci build: not enabling SM protection "
                "to avoid timeout");
    #else
        #ifdef VULCAN_SM
            sancus_enable(&sm_eval);
            pr_sm_info(&sm_eval);
        #endif

        #ifdef CAN_DRV_SM
            extern struct SancusModule sm_mmio_spi;
            sancus_enable(&sm_mmio_spi);
            pr_sm_info(&sm_mmio_spi);
        #endif
    #endif

    eval_run();
    EXIT();
}
