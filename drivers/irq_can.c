#include "irq_can.h"

void* __isr_sp = (void*) &__can_isr_stack[CAN_ISR_STACK_SIZE-1];

void ican_irq_init(ican_t *ican)
{
    // CAN module enable interrupt
    can_w_bit(&msp_ican, MCP2515_CANINTE,  MCP2515_CANINTE_RX0IE, 0x01);
    can_w_bit(&msp_ican, MCP2515_CANINTE,  MCP2515_CANINTE_RX1IE, 0x02);

    // MSP P1.0 enable interrupt on negative edge
    P1IE = 0x01;
    P1IES = 0x01;
    P1IFG = 0x00;
}
