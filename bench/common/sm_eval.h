#ifndef SM_RPM_H_INC
#define SM_RPM_H_INC

#include <sancus/sm_support.h>
#include "../../drivers/ican.h"

#define CAN_SPI_SS          1
#define CAN_ID_PING         0xf0
#define CAN_ID_PONG         0xf8
#define CAN_ID_AEC_SEND     0xaa
#define CAN_ID_AEC_RECV     0xbb
#define EVAL_NB_CONNECTIONS 4

#ifdef NOAUTH
    #define do_recv     ican_recv
    #define do_send     ican_send
#else
    #define do_recv     vulcan_recv
    #define do_send     vulcan_send
#endif

#define pr_progress(s)  puts("\n*** " s)

extern struct SancusModule sm_eval;
extern ican_t msp_ican;
extern VULCAN_DATA ican_link_info_t eval_connections[];

void VULCAN_ENTRY eval_run(void);

void SM_ENTRY(sm_eval) dummy_entry(void);

void VULCAN_FUNC eval_do_init(uint16_t aec_own, uint16_t aec_listen);

#endif
