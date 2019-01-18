/* TODO: LED = 0x0150 (or something similar) should be provided by 
 *                        msp430sancus_symbols.ld */
PROVIDE(sllvm_pm_sm_mmio_led_data_start = 0x0090);
PROVIDE(sllvm_pm_sm_mmio_led_data_end   = 0x0098);
