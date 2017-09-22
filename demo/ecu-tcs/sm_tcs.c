#include "sm_tcs.h"
#include <sancus_support/sm_io.h>
#include "sm_tcs_kypd.h"

#ifdef VATICAN_INCLUDE_NONCE_GENERATOR
    #include "nonce_generator.h"
    extern void VULCAN_FUNC vatican_nonce_reset (uint32_t nonce);
#endif

#include "../../drivers/inst-cluster.h"

#define CAN_ID_WHEEL_LEFT   0x20
#define CAN_ID_WHEEL_RIGHT  0x22
#define TCS_NB_CONNECTIONS  4
#define CAN_SPI_SS          1

/*
 * SM that sends messages over an _authenticated_ CAN interface, managed by an
 * _untrusted_ driver, based on secure MMIO keypad input.
 */
DECLARE_SM(sm_tcs, 0x1234);

DECLARE_VULCAN_ICAN(msp_ican, CAN_SPI_SS, CAN_500_KHZ, 0x0, 0x0);

SM_DATA(sm_tcs) int tcs_turn, tcs_turn_left, tcs_turn_right, tcs_blink,
                    tcs_turn_count, tcs_rpm, tcs_rpm_count, tcs_key_pressed;

void SM_FUNC(sm_tcs) tcs_send_msg(uint16_t id, uint8_t m)
{
    int rv;
    rv = vulcan_send(&msp_ican, id, &m, 1, /*block=*/1);
    ASSERT(rv >= 0);
}

void SM_FUNC(sm_tcs) tcs_rpm_update(int rpm)
{
    int rv, rpm_left, rpm_right;

    if (rpm <= 80 && rpm >= 0)
        tcs_rpm = rpm;
    tcs_rpm_count = 1000;
    tcs_blink = !tcs_blink;

    // simulate slightly more traction on outer wheels when turning
    rpm_left = tcs_turn_right ? tcs_rpm+5 : tcs_rpm;
    rpm_right = tcs_turn_left ? tcs_rpm+5 : tcs_rpm;

    tcs_send_msg(CAN_ID_WHEEL_LEFT, rpm_left);
    tcs_send_msg(CAN_ID_WHEEL_RIGHT, rpm_right);
    rv = ic_rpm(&msp_ican, tcs_rpm);
    ASSERT(rv >= 0);

    if (tcs_turn_left)
        rv = ic_ind(&msp_ican, tcs_blink, 0, /*warn=*/0);
    else if (tcs_turn_right)
        rv = ic_ind(&msp_ican, 0, tcs_blink, /*warn=*/0);
    ASSERT(rv >= 0);
}

void SM_FUNC(sm_tcs) tcs_handle_key_press(PmodKypdKey key)
{
    int rv;
    uint32_t nonce;
    pr_info1("key '%c' pressed!\n", tcs_key_to_char(key));

    switch (key)
    {
        case Key_D:
            tcs_turn = !tcs_turn;
            tcs_turn_count = 1;
            rv = ic_ind(&msp_ican, 0, 0, /*warn=*/0);
            ASSERT(rv >= 0);
            break;

        case Key_3:
            tcs_rpm_update(tcs_rpm + 10);
            break;

        case Key_2:
            tcs_rpm_update(tcs_rpm - 10);
            break;

        case Key_5:
            tcs_turn_left = 1;
            break;

        case Key_6:
            tcs_turn_right = 1;
            break;

        case Key_0:
            #if VATICAN_INCLUDE_NONCE_GENERATOR
                nonce = nonce_generator_run(&msp_ican);
                vatican_nonce_reset(nonce);
            #endif
            break;

        default:
            break;
    }
}

void SM_FUNC(sm_tcs) tcs_handle_key_release(PmodKypdKey key)
{
    pr_info1("key '%c' released!\n", tcs_key_to_char(key));
    tcs_turn_left = tcs_turn_right = 0;
}
/*
 * Securely store const connection initialization info in SM text section
 * (protect key secrecy via confidential loading).
 */
SM_DATA(sm_tcs) const uint8_t tcs_key_gateway[SANCUS_KEY_SIZE] =
        {0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
         0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

SM_DATA(sm_tcs) const uint8_t tcs_key_wheel[SANCUS_KEY_SIZE] =
        {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

SM_DATA(sm_tcs) int tcs_init = 0;
SM_DATA(sm_tcs) ican_link_info_t tcs_connections[TCS_NB_CONNECTIONS];

void SM_FUNC(sm_tcs) tcs_do_init(void)
{
    int rv;

    pr_info("initializing TCS module");
    tcs_keypad_init();
    #if VATICAN_INCLUDE_NONCE_GENERATOR
        nonce_generator_init();
    #endif

    // initialize connection info list
    tcs_connections[0].id  = CAN_ID_WHEEL_LEFT;
    tcs_connections[0].k_i = &tcs_key_wheel[0];
    tcs_connections[1].id  = IC_CAN_ID_IND;
    tcs_connections[1].k_i = &tcs_key_gateway[0];
    tcs_connections[2].id  = IC_CAN_ID_RPM;
    tcs_connections[2].k_i = &tcs_key_gateway[0];
    tcs_connections[3].id  = CAN_ID_WHEEL_RIGHT;
    tcs_connections[3].k_i = &tcs_key_wheel[0];

    // initialize authenticated CAN interface
    rv = vulcan_init(&msp_ican, tcs_connections, TCS_NB_CONNECTIONS);
    ASSERT(rv >= 0);

    tcs_turn = 0;
    tcs_turn_right = 0;
    tcs_turn_left = 0;
    tcs_key_pressed = 0;
    tcs_blink = 0;
    tcs_turn_count = 0;
    tcs_rpm = 0;
    tcs_rpm_count = 0;
    tcs_init = 1;
}

int SM_ENTRY(sm_tcs) tcs_run(void)
{
    int rv, i;

    if (!tcs_init)
        tcs_do_init();

    rv = tcs_keypad_poll();

    if (tcs_turn && !tcs_turn_count--)
    {
        tcs_turn_count = 2000;
        tcs_blink = !tcs_blink;
        i = ic_ind(&msp_ican, tcs_blink, tcs_blink, /*warn=*/0);
        ASSERT(i >= 0);
    }
    if (tcs_rpm && !tcs_rpm_count--)
        tcs_rpm_update(tcs_rpm);

    return rv;
}
