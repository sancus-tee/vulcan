#ifndef SM_TCS_KYPD_H_INC
#define SM_TCS_KYPD_H_INC

#include <sancus/sm_support.h>

typedef enum
{
    Key_0 =  3,
    Key_1 =  0,
    Key_2 =  4,
    Key_3 =  8,
    Key_4 =  1,
    Key_5 =  5,
    Key_6 =  9,
    Key_7 =  2,
    Key_8 =  6,
    Key_9 = 10,
    Key_A = 12,
    Key_B = 13,
    Key_C = 14,
    Key_D = 15,
    Key_E = 11,
    Key_F =  7,
} PmodKypdKey;

void SM_FUNC(sm_tcs) tcs_keypad_init(void);

char SM_FUNC(sm_tcs) tcs_key_to_char(PmodKypdKey key);

int SM_FUNC(sm_tcs) tcs_keypad_poll(void);

void SM_FUNC(sm_tcs) tcs_handle_key_press(PmodKypdKey key);
void SM_FUNC(sm_tcs) tcs_handle_key_release(PmodKypdKey key);

#endif
