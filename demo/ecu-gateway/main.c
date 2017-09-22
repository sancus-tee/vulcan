#include <sancus_support/sm_io.h>
#include "sm_gateway.h"

int main (void)
{
    msp430_io_init();

    sancus_enable(&sm_gateway);
    pr_sm_info(&sm_gateway);

    sancus_enable(&sm_mmio_spi);
    pr_sm_info(&sm_mmio_spi);

    while (1)
    {
        sm_gateway_run();
    }
}
