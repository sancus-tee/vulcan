#include "sm_rpm.h"
#include <sancus_support/sm_io.h>
#include "../../drivers/sm_led.h"

#define CAN_SPI_SS          1

/*
 * SM that receives messages from central TCS system, over an
 * authenticated CAN interface, managed by an _unprotected_ driver.
 */
DECLARE_SM(sm_rpm, 0x1234);

DECLARE_VULCAN_ICAN(msp_ican, CAN_SPI_SS, CAN_500_KHZ, CAN_ID_RPM, 0x0); 

/*
 * Securely store const connection initialization info in SM text section
 * (protect key secrecy via confidential loading).
 */
SM_DATA(sm_rpm) const uint8_t rpm_key[SANCUS_KEY_SIZE] =
         {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
          0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

SM_DATA(sm_rpm) int rpm_init = 0;
SM_DATA(sm_rpm) int rpm_cur;
SM_DATA(sm_rpm) ican_link_info_t rpm_connection;

void SM_FUNC(sm_rpm) rpm_update(int val, int ok)
{
    // avoid unsigned modulo operations
    volatile int v = ok ? (rpm_cur = val) : rpm_cur;
    char v1 = '0' + (v % 10);
    char v2 = '0' + ((v/10) % 10);

    if (ok)
    {
        #ifdef RPM_AUTH
            led_update(v1, v2, ' ', ' ', 'd', 'o', 'o', 'G');
        #else
            led_update(v1, v2, ' ', 'C', 'E', 'S', 'o', 'n');
        #endif
    }
    else
        led_update(v1, v2, ' ', ' ', 'd', 'A', 'b', ' ');
}

void SM_FUNC(sm_rpm) rpm_do_init(void)
{
    int i;
    pr_info("initializing RPM module");

    rpm_connection.id = CAN_ID_RPM;
    rpm_connection.k_i = &rpm_key[0];

    i = vulcan_init(&msp_ican, &rpm_connection, 1);
    ASSERT(i >= 0);
    rpm_update(0, 1);
    rpm_init = 1;
}

void SM_ENTRY(sm_rpm) rpm_run(void)
{
    uint16_t msg_id;
    uint8_t msg[CAN_PAYLOAD_SIZE];
    int i, len;

    if (!rpm_init)
        rpm_do_init();

    while (1)
    {
        /* receive expected authenticated CAN message */
        for (i = 0; i < 8; i++) msg[i] = 0x00;
        pr_info1("busy waiting for recv (ID=0x%03x)..\n", CAN_ID_RPM);
        if ((len = vulcan_recv(&msp_ican, &msg_id, msg, /*block=*/1)) > 0)
            rpm_update(msg[0], 1);
        else
        {
            //HACK to simulate unprotected legacy devices..
            #ifdef RPM_AUTH
                rpm_update(msg[0], 0);
            #else
                rpm_update(msg[0], 1);
            #endif
        }
    }
}
