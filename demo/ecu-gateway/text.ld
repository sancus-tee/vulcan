sllvm_pm_sm_gateway_text_start = .;
*(.sllvm.text.dispatch.sm_gateway)  /* Make sure the dispatcher is put first */
*(.sllvm.text.sm_gateway)
*(.sllvm.text.sm_gateway.*)         /* Internalized runtime functions */
sllvm_pm_sm_gateway_text_end = .;

sllvm_pm_sm_mmio_spi_text_start = .;
*(.sllvm.text.dispatch.sm_mmio_spi)  /* Make sure dispatcher is put first */
*(.sllvm.text.sm_mmio_spi)
sllvm_pm_sm_mmio_spi_text_end = .;

/* Wrap information */
*(.sllvm.wrap.*)

/* Symbols for secure linking */
*(.sllvm.slink.*)
