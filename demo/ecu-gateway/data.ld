sllvm_pm_sm_gateway_data_start = .;
*(.sllvm.data.sm_gateway)
*(.sllvm.data.lr1addr.sm_gateway) /* Make sure pointer to local sp is last */
sllvm_pm_sm_gateway_data_end = .;
