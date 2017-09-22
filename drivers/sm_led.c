#include "sm_led.h"

#define LD_X    0x00

// NOTE: constant lookup tables are securely stored in SM text section
const LED_DATA uint8_t led_digit_map[] = {
    /*0=*/ 0x3f, 0x06, 0x5b, 0x4f, 0x66,
    /*5=*/ 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

const LED_DATA uint8_t led_alpha_map[] = {
    /*A=*/ 0x77, 0x7c, 0x39, 0x5e, 0x79,
    /*F=*/ 0x71, 0x3d, 0x74, 0x30, 0x1e,
    /*K=*/ LD_X, 0x38, LD_X, 0x54, 0x5c,
    /*P=*/ 0x73, LD_X, 0x50, 0x6d, 0x78,
    /*U=*/ 0x1c, LD_X, LD_X, LD_X, 0x6e,
    /*Z=*/ LD_X
};

uint8_t LED_FUNC led_from_char(char c)
{
    if (c >= '0' && c <= '9')
        return led_digit_map[c-'0'];

    if (c >= 'A' && c <= 'Z')
        c = 'a' + (c-'A'); 

    if (c >= 'a' && c <= 'z')
        return led_alpha_map[c-'a'];

    return LD_X;
}

/*
 * Since the MSP430 calling conventions only feature 4 argument registers, we
 * securely encode two LEDs in one 16-bit argument.
 */
void LED_FUNC led_update(char c1, char c2, char c3, char c4,
                         char c5, char c6, char c7, char c8)
{
    // volatile to work around compiler bug..
    volatile uint16_t arg1 = (led_from_char(c2) << 8) | led_from_char(c1);
    uint16_t arg2 = (led_from_char(c4) << 8) | led_from_char(c3);
    uint16_t arg3 = (led_from_char(c6) << 8) | led_from_char(c5);
    uint16_t arg4 = (led_from_char(c8) << 8) | led_from_char(c7);
    
    sm_mmio_led_write(arg1, arg2, arg3, arg4);
}
