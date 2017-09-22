#include <sancus_support/sm_io.h>
#include "sm_rpm.h"

int main (void)
{
    msp430_io_init();

    sancus_enable(&sm_rpm);
    pr_sm_info(&sm_rpm);

    sancus_enable(&sm_mmio_led);
    pr_sm_info(&sm_mmio_led);

    while (1)
    {
        rpm_run();
    }
}
