sllvm_pm_sm_rpm_text_start = .;
*(.sllvm.text.dispatch.sm_rpm)  /* Make sure the dispatcher is put first */
*(.sllvm.text.sm_rpm)
*(.sllvm.text.sm_rpm.*)         /* Internalized runtime functions */
sllvm_pm_sm_rpm_text_end = .;

sllvm_pm_sm_mmio_led_text_start = .;
*(.sllvm.text.dispatch.sm_mmio_led)  /* Make sure dispatcher is put first */
*(.sllvm.text.sm_mmio_led)
sllvm_pm_sm_mmio_led_text_end = .;

/* Wrap information */
*(.sllvm.wrap.*)

/* Hashes for secure linking */
*(.sllvm.hash.*)
