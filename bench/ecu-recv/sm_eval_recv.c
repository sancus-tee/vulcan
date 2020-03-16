#include "../common/sm_eval.h"
//#include "../../drivers/irq_can.h"
#include <sancus_support/sm_io.h>
#include <errno.h>

/* Authenticated CAN interface, managed by an _unprotected_ driver. */
DECLARE_VULCAN_ICAN(msp_ican, CAN_SPI_SS, CAN_500_KHZ, CAN_ID_PING, CAN_ID_AEC_SEND);

void CAN_DRV_FUNC __attribute__((noinline)) sync_send(void)
{
    uint16_t msg_id;
    uint8_t msg[CAN_PAYLOAD_SIZE] = {0xff};

    ican_send(&msp_ican, CAN_ID_PONG, msg, CAN_PAYLOAD_SIZE, /*block=*/1);
}

void VULCAN_ENTRY eval_run(void)
{
    uint16_t i, msg_id;
    uint8_t msg_ping[CAN_PAYLOAD_SIZE] = {0x00};
    uint8_t msg_pong[CAN_PAYLOAD_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

    eval_do_init(/*own=*/ CAN_ID_AEC_RECV, /*listen=*/ CAN_ID_AEC_SEND);

    /* Synchronize with sender */
    sync_send();

    pr_progress("Listening for ping messages");
    /* Block wait for (authenticated) ping message */
    while ((do_recv(&msp_ican, &msg_id, msg_ping, /*block=*/1) != -EINVAL) && msg_ping[0])
    {
        /* Reply with (authenticated) pong message(s) */
        for (i = 0; i < msg_ping[0]; i++)
            vulcan_send_iat(&msp_ican, CAN_ID_PONG, msg_pong, CAN_PAYLOAD_SIZE, /*block=*/1);
    }
    pr_progress("Stop signal received; exiting");
}
