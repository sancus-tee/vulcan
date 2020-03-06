#include "irq_can.h"

void* __isr_sp = (void*) &__can_isr_stack[CAN_ISR_STACK_SIZE-1];
