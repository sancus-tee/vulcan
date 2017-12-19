#include "sm_gateway.h"
#include <sancus_support/sm_io.h>
#include <sancus_support/led_digits.h>

// Trigger warning light on instrument cluster on _private_ CAN bus
#define ic_ican_send ican_send
#include "../../drivers/inst-cluster.h"

#define IC_CAN_SS       1
#define MSP_CAN_SS      2

// Untrusted authenticated CAN interface
DECLARE_VULCAN_ICAN(msp_ican, MSP_CAN_SS, CAN_500_KHZ, IC_CAN_ID_IND, IC_CAN_ID_RPM);

#if !defined(VULCAN_SM) || !defined(CAN_DRV_SM)
    #error Secure gateway requires protected CAN interface and driver..
#endif

// Protected gateway module
DECLARE_SM(sm_gateway, 0x1234);

// Trusted private _protected_ CAN interface
DECLARE_ICAN_SIMPLEX(ic_ican, IC_CAN_SS, IC_BAUDRATE);

SM_DATA(sm_gateway) ican_link_info_t ican_connections[2];
SM_DATA(sm_gateway) const uint8_t gateway_key[SANCUS_KEY_SIZE] = 
        {0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
         0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

SM_DATA(sm_gateway) int sm_gateway_init = 0;

void SM_FUNC(sm_gateway) sm_gateway_do_init( void )
{
    int rv;

    ican_connections[0].id = IC_CAN_ID_IND;
    ican_connections[0].k_i = &gateway_key[0];

    ican_connections[1].id = IC_CAN_ID_RPM;
    ican_connections[1].k_i = &gateway_key[0];

    rv = vulcan_init(&msp_ican, ican_connections, 2);
    ASSERT(rv >= 0);
    rv = ican_init(&ic_ican);
    ASSERT(rv >= 0);
    
    sm_gateway_init = 1;
}

// Untrusted (unprotected) LED output for visualisation purposes
void __attribute__( ( noinline ) ) led_accept(void)
{
     led_digits_update(' ', 't', 'P', 'E', 'C', 'C', 'A', ' ');
}

void __attribute__( ( noinline ) ) led_reject(void)
{
     led_digits_update(' ', 't', 'C', 'E', 'J', 'E', 'r', ' ');
}

void SM_ENTRY(sm_gateway) sm_gateway_run( void )
{
    uint16_t msg_id;
    uint8_t msg[CAN_PAYLOAD_SIZE];
    int i, j, len, turn = 0, blink = 0, rpm = 0;

    if (!sm_gateway_init)
        sm_gateway_do_init();
    
    while (1)
    {
        /* receive expected authenticated CAN message on MSP interface */
        for (i = 0; i < 8; i++) msg[i] = 0x00;
        pr_info2("busy waiting for recv (ID=0x%03x/0x%03x)..\n",
                 IC_CAN_ID_RPM, IC_CAN_ID_IND);
        
        /* forward successfully authenticated message on trusted CAN */
        if ((len = vulcan_recv(&msp_ican, &msg_id, msg,/*block=*/1)) >= 0)
        {
            ican_send(&ic_ican, msg_id, msg, len, /*block=*/1);
            led_accept();
        }
        else
        {
           ic_ind(&ic_ican, 0, 0, 1);
           led_reject();
        }
    }
}
