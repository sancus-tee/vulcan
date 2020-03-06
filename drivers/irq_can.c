#include "irq_can.h"

void* __isr_sp = (void*) &__isr_stack[ISR_STACK_SIZE-1];
