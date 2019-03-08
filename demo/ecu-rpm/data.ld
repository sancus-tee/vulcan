sllvm_pm_sm_rpm_data_start = .;
*(.sllvm.data.sm_rpm)
*(.sllvm.data.lr1addr.sm_rpm) /* Make sure pointer to local sp is last */
sllvm_pm_sm_rpm_data_end = .;
