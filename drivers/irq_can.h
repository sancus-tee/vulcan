#ifndef IRQ_CAN_H_INC
#define IRQ_CAN_H_INC

#include <msp430.h>
#include <stdint.h>

#define CAN_IRQ_VECTOR    	4 /* For P1, 6 for P2 */
#define CAN_ISR_STACK_SIZE 	(512)

uint16_t __can_isr_stack[CAN_ISR_STACK_SIZE];
extern void* __isr_sp;

#define CAN_ISR_ENTRY(fct)                                        \
__attribute__((naked)) __attribute__((interrupt(CAN_IRQ_VECTOR))) \
void can_isr_entry(void)                                         \
{                                                                   \
    __asm__ __volatile__(                                           \
            "cmp #0x0, r1\n\t"                                      \
            "jne 1f\n\t"                                            \
            "mov &__isr_sp, r1\n\t"                                 \
            "push r15\n\t"                                          \
            "push r2\n\t"                                           \
            "1: push r15\n\t"                                       \
            "push r14\n\t"                                          \
            "push r13\n\t"                                          \
            "push r12\n\t"                                          \
            "push r11\n\t"                                          \
            "push r10\n\t"                                          \
            "push r9\n\t"                                           \
            "push r8\n\t"                                           \
            "push r7\n\t"                                           \
            "push r6\n\t"                                           \
            "push r5\n\t"                                           \
            "push r4\n\t"                                           \
            "push r3\n\t"                                           \
            "call #" #fct "\n\t"                                    \
            "pop r3\n\t"                                            \
            "pop r4\n\t"                                            \
            "pop r5\n\t"                                            \
            "pop r6\n\t"                                            \
            "pop r7\n\t"                                            \
            "pop r8\n\t"                                            \
	    "pop r9\n\t"                                            \
            "pop r10\n\t"                                           \
            "pop r11\n\t"                                           \
            "pop r12\n\t"                                           \
            "pop r13\n\t"                                           \
            "pop r14\n\t"                                           \
            "pop r15\n\t"                                           \
            "reti\n\t"                                              \
            :::);                                                   \
}

#endif
