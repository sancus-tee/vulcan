sllvm_pm_sm_eval_data_start = .;
*(.sllvm.data.sm_eval)
*(.sllvm.data.lr1addr.sm_eval) /* Make sure the pointer to local sp is last */
sllvm_pm_sm_eval_data_end = .;
