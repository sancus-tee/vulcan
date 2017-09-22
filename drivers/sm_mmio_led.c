#include "sm_mmio_led.h"

// MMIO registers for led digit segments:
// (MSB-1) g f e d c b a (LSB)
#define LED_BASE    0x0090
#define LED1        *(volatile uint8_t*)(LED_BASE+0)
#define LED2        *(volatile uint8_t*)(LED_BASE+1)
#define LED3        *(volatile uint8_t*)(LED_BASE+2)
#define LED4        *(volatile uint8_t*)(LED_BASE+3)
#define LED5        *(volatile uint8_t*)(LED_BASE+4)
#define LED6        *(volatile uint8_t*)(LED_BASE+5)
#define LED7        *(volatile uint8_t*)(LED_BASE+6)
#define LED8        *(volatile uint8_t*)(LED_BASE+7)

DECLARE_EXCLUSIVE_MMIO_SM(sm_mmio_led,
                          /*[secret_start, end[=*/ LED_BASE, LED_BASE+8,
                          /*caller_id=*/ 1,
                          /*vendor_id=*/ 0x1234);

/*
 * This function expects the following ABI:
 *
 * \arg r15[0:7]  led1
 * \arg r15[8:15] led2
 * \arg r14[0:7]  led3
 * \arg r14[8:15] led4
 * \arg r13[0:7]  led5
 * \arg r13[8:15] led6
 * \arg r12[0:7]  led7
 * \arg r12[8:15] led8
 */
void SM_MMIO_ENTRY(sm_mmio_led) sm_mmio_led_write(uint16_t a1, uint16_t a2,
                                                   uint16_t a3, uint16_t a4)
{
    asm(
    "mov.b r15, %0  \n\t"
    "swpb r15       \n\t"
    "mov.b r15, %1  \n\t"
    "mov.b r14, %2  \n\t"
    "swpb r14       \n\t"
    "mov.b r14, %3  \n\t"
    "mov.b r13, %4  \n\t"
    "swpb r13       \n\t"
    "mov.b r13, %5  \n\t"
    "mov.b r12, %6  \n\t"
    "swpb r12       \n\t"
    "mov.b r12, %7  \n\t"
    ::"m"(LED1), "m"(LED2), "m"(LED3), "m"(LED4),
      "m"(LED5), "m"(LED6), "m"(LED7), "m"(LED8):
    );
}
