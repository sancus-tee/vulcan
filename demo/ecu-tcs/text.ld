sllvm_pm_sm_tcs_text_start = .;
*(.sllvm.text.dispatch.sm_tcs)  /* Make sure the dispatcher is put first */
*(.sllvm.text.sm_tcs)
*(.sllvm.text.sm_tcs.*)         /* Internalized runtime functions */
sllvm_pm_sm_tcs_text_end = .;

sllvm_pm_sm_mmio_kypd_text_start = .;
*(.sllvm.text.dispatch.sm_mmio_kypd)  /* Make sure dispatcher is put first */
*(.sllvm.text.sm_mmio_kypd)
sllvm_pm_sm_mmio_kypd_text_end = .;

/* Wrap information */
*(.sllvm.wrap.*)

/* Hashes for secure linking */
*(.sllvm.hash.*)
