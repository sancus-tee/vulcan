#include "sm_mmio_kypd.h"
#include <msp430.h>

DECLARE_EXCLUSIVE_MMIO_SM(sm_mmio_kypd,
                          /*[secret_start, end[=*/ P3IN_, P4IN_,
                          /*caller_id=*/ 1,
                          /*vendor_id=*/ 0x1234);

key_state_t SM_MMIO_ENTRY(sm_mmio_kypd) sm_mmio_kypd_poll(void)
{
    asm(
    "clr r15\n\t"           /* return value key_state bitmask */              
    "clr r14\n\t"           /* loop counter */                                
    "mov.b #0xfe,r13\n\t"   /* column pin selector */                         
                                                                              
    "1: cmp #4, r14                                                     \n\t" 
    "jge 4f                                                             \n\t" 
                                                                              
    /* pull the column pin low */                                             
    "mov.b r13, &%0                                                     \n\t" 
    /* if we don't wait a couple of cycles, we sometimes seem to read old     
       values from the row pins */                                            
    "nop                                                                \n\t" 
    "nop                                                                \n\t" 
    "nop                                                                \n\t" 
    "nop                                                                \n\t" 
    "nop                                                                \n\t" 
                                                                              
    /* read state of current column (row pins) and shift right into 4 LSBs */ 
    "mov.b &%1, r12                                                     \n\t" 
    "clrc                                                               \n\t" 
    "rrc.b r12                                                          \n\t" 
    "rra.b r12                                                          \n\t" 
    "rra.b r12                                                          \n\t" 
    "rra.b r12                                                          \n\t" 
                                                                              
    /* shift current column state left into key_state return bitmask */       
    "clr r11                                                            \n\t" 
    "2: cmp r14, r11                                                    \n\t" 
    "jge 3f                                                             \n\t" 
    "rla r12                                                            \n\t" 
    "rla r12                                                            \n\t" 
    "rla r12                                                            \n\t" 
    "rla r12                                                            \n\t" 
    "inc r11                                                            \n\t" 
    "jmp 2b                                                             \n\t" 
    "3: bis r12, r15                                                    \n\t" 
                                                                              
    "inc r14                                                            \n\t" 
    "setc                                                               \n\t" 
    "rlc.b r13                                                          \n\t" 
    "jmp 1b                                                             \n\t" 
                                                                              
    /* since the row pins are active low, we invert the state to get a more   
       natural representation*/                                               
    "4: inv r15                                                         \n\t"
    ::"m"(P3OUT), "m"(P3IN):
    );
}

void SM_MMIO_ENTRY(sm_mmio_kypd) sm_mmio_kypd_init(void)
{
    asm(
    /* column pins are mapped to P3OUT[0:3] and the row pins to P3IN[4:7] */  
    "mov.b #0x00, &%0                                                   \n\t" 
    "mov.b #0x0f, &%1                                                   \n\t"
    ::"m"(P3SEL), "m"(P3DIR):
    );
}
