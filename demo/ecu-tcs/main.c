#include <sancus/sm_support.h>
#include <sancus_support/sm_io.h>
#include <stdio.h>
#include "sm_tcs.h"
#include "attacker.h"

int main (void)
{
  int rv_tcs = 1, rv_attacker = 1;

  msp430_io_init();

  sancus_enable(&sm_tcs);
  pr_sm_info(&sm_tcs);

  sancus_enable(&sm_mmio_kypd);
  pr_sm_info(&sm_mmio_kypd);

  while (1)
  {
#if 1
    if (rv_attacker)
        pr_info("polling attacker keypad..\n");
    rv_attacker = attacker_run();
#endif

    if (rv_tcs)
        pr_info("polling secure keypad..\n");
    rv_tcs = tcs_run();
  }
}
