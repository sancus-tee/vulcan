sllvm_pm_sm_tcs_data_start = .;
*(.sllvm.data.sm_tcs)
*(.sllvm.data.lr1addr.sm_tcs) /* Make sure the pointer to local sp is last */
sllvm_pm_sm_tcs_data_end = .;
