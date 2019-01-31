sllvm_pm_sm_eval_text_start = .;
*(.sllvm.text.dispatch.sm_eval)  /* Make sure the dispatcher is put first */
*(.sllvm.text.sm_eval)
*(.sllvm.text.sm_eval.*)         /* Internalized runtime functions */
sllvm_pm_sm_eval_text_end = .;

/* Wrap information */
*(.sllvm.wrap.*)

/* Symbols for secure linking */
*(.sllvm.slink.*)
