#include "sm_tcs_kypd.h"
#include "../../drivers/sm_mmio_kypd.h"

key_state_t SM_DATA(sm_tcs) tcs_current_key_state;

void SM_FUNC(sm_tcs) tcs_keypad_init(void)
{
    sm_mmio_kypd_init();
    tcs_current_key_state = 0x0;
}

// Securely store constant initialized keymap in SM text section
const char SM_DATA(sm_tcs) keymap[] = {
    '1', '4', '7', '0',
    '2', '5', '8', 'F',
    '3', '6', '9', 'E',
    'A', 'B', 'C', 'D', 'X'
};

char SM_FUNC(sm_tcs) tcs_key_to_char(PmodKypdKey key)
{
    return keymap[key];
}


int SM_FUNC(sm_tcs) tcs_key_is_pressed(key_state_t key_state, PmodKypdKey key)
{
    return key_state & (1 << key);
}


/* 
 * Fetch and process key state from keypad driver SM.
 * NOTE: we do _not_ early out the for loop below upon detecting a key access
 * to prevent side-channel leakage of key presses via execution time.
 */
int SM_FUNC(sm_tcs) tcs_keypad_poll(void)
{
    int rv = 0;
    key_state_t new_key_state = sm_mmio_kypd_poll();

    for (int key = 0; key < KYPD_NB_KEYS; key++)
    {
        int was_pressed = tcs_key_is_pressed(tcs_current_key_state, key);
        int is_pressed = tcs_key_is_pressed(new_key_state, key);

        if (!was_pressed && is_pressed)
        {
            rv++;
            tcs_handle_key_press(key);
        }
        else if (was_pressed && !is_pressed)
            tcs_handle_key_release(key);
    }

    tcs_current_key_state = new_key_state;
    return rv;
}
