#include "sm_eval.h"
#include "../../drivers/mcp2515.h"
#include <sancus_support/sm_io.h>

#ifndef __SANCUS_IO_BENCH
    #warning benchmarking with debug print statements..
#endif

DECLARE_SM(sm_eval, 0x1234);

/*
 * Securely store const connection initialization info in SM text section
 * NOTE: key secrecy could be protected via confidential loading
 */
VULCAN_DATA const uint8_t eval_key_aec[SANCUS_KEY_SIZE] =
         {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe,
          0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
VULCAN_DATA const uint8_t eval_key_ping[SANCUS_KEY_SIZE] =
         {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
          0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
VULCAN_DATA const uint8_t eval_key_pong[SANCUS_KEY_SIZE] =
         {0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

VULCAN_DATA int eval_init = 0;
VULCAN_DATA ican_link_info_t eval_connections[EVAL_NB_CONNECTIONS];

void VULCAN_FUNC eval_do_init(uint16_t aec_own, uint16_t aec_listen)
{
    if (eval_init) return;

    eval_connections[0].id = CAN_ID_PING;
    eval_connections[0].k_i = &eval_key_ping[0];
    eval_connections[1].id = CAN_ID_PONG;
    eval_connections[1].k_i = &eval_key_pong[0];
    eval_connections[2].id = aec_listen;
    eval_connections[2].k_i = &eval_key_aec[0];
    eval_connections[3].id = aec_own;
    eval_connections[3].k_i = &eval_key_aec[0];

    vulcan_init(&msp_ican, eval_connections, EVAL_NB_CONNECTIONS);
    eval_init = 1;
}

void SM_ENTRY(sm_eval) dummy_entry(void)
{
    return;
}
