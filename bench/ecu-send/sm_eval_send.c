#include "../common/sm_eval.h"
#include <sancus_support/sm_io.h>
#include <sancus_support/tsc.h>
#include <errno.h>

#define BENCH_SEND          0
#define BENCH_DEMO          0
#define BENCH_RTT           1

/* Authenticated CAN interface, managed by an _unprotected_ driver. */
DECLARE_VULCAN_ICAN(msp_ican, CAN_SPI_SS, CAN_500_KHZ, CAN_ID_PONG, CAN_ID_AEC_RECV);
DECLARE_TSC_TIMER(tsc_eval);

VULCAN_DATA const uint8_t msg_ping_init[CAN_PAYLOAD_SIZE] =
            {0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
VULCAN_DATA uint8_t msg_ping[CAN_PAYLOAD_SIZE];
VULCAN_DATA uint8_t msg_pong[CAN_PAYLOAD_SIZE];
VULCAN_DATA uint16_t msg_id;

void CAN_DRV_FUNC __attribute__((noinline)) sync_recv(void)
{
    uint16_t id = -1;
    uint8_t msg[CAN_PAYLOAD_SIZE] = {0x00};

    while ((ican_recv(&msp_ican, &id, msg, /*block=*/1) < 0) ||
            id != CAN_ID_PONG || !msg[0]);
}

#if defined (BENCH_RTT) || defined (BENCH_SEND)
uint64_t total;

void __attribute__ ((noinline)) dump_avg(void)
{

    total = total / 128;
    printf("average (128 samples) is %llu\n", total);
}

void __attribute__((noinline)) dump_timer(char *s)
{
    printf("%s: %llu\n", s, tsc_eval_get_interval());
}
#endif

#ifdef BENCH_SEND
void VULCAN_FUNC eval_send(void)
{
    int len, i, rv;
    ican_eid_t eid = {CAN_ID_PING, 0};

    pr_progress("measuring (authenticated) send/recv primitives");
    for (int i=0; i < 128;)
    {
        // make sure sender does not get out of sync
        if (i) while ((len = do_recv(&msp_ican, &msg_id, msg_pong, /*block=*/1)) < 0);
        TSC_TIMER_START(tsc_eval);
        //rv = ican_send_ext(&msp_ican, eid, msg_ping, CAN_PAYLOAD_SIZE, /*block=*/1);
        rv = do_send(&msp_ican, CAN_ID_PING, msg_ping, CAN_PAYLOAD_SIZE, /*block=*/1);
        TSC_TIMER_END(tsc_eval);
        if (rv >= 0)
        {
            //NOTE: minimal value represents case were MAC message could be
            //sent directly, without waiting for completion of payload message
            dump_timer("ican_send");
            i++;
            total += tsc_eval_get_interval();
        }
    }
    dump_avg();

    TSC_TIMER_START(tsc_eval);
    while ((len = do_recv(&msp_ican, &msg_id, msg_pong, /*block=*/1)) < 0);
    TSC_TIMER_END(tsc_eval);
    dump_timer("ican_recv");
}
#endif

#ifdef BENCH_DEMO
void VULCAN_FUNC eval_demo(void)
__attribute__((optnone)) /* work around compiler bug */
{
    int rv;

    pr_progress("testing authenticated ping-pong round-trip");
    msg_ping[0] = 2;
    rv = do_send(&msp_ican, CAN_ID_PING, msg_ping, CAN_PAYLOAD_SIZE,
                   /*block=*/1);
    ASSERT(rv >= 0);
    rv = do_recv(&msp_ican, &msg_id, msg_pong, /*block=*/1);
    ASSERT((rv >= 0) && (msg_id == CAN_ID_PONG));
    pr_progress("authenticated ping-pong test succeeded!");

    #ifndef NOAUTH
        pr_progress("testing pong authentication failure");

        // NOTE: vatiCAN returns -EINVAL; LeiA should automatically recover
        #ifdef VATICAN_NONCE_SIZE
            eval_connections[1].c++;
        #else
            eval_connections[1].k_e[0] = 0xff;
        #endif
        rv = do_recv(&msp_ican, &msg_id, msg_pong, /*block=*/1);
        ASSERT(rv < 0);
        while (rv == -EAGAIN)
            rv = do_recv(&msp_ican, &msg_id, msg_pong, /*block=*/1);

        ASSERT((rv == -EINVAL) || (rv >= 0 && (msg_id == CAN_ID_PONG)));
        pr_progress("authentication failure test succeeded!");
    #endif
}
#endif

#ifdef BENCH_RTT
void VULCAN_FUNC eval_rtt(void)
{
    int i, len;

    pr_progress("Measuring round-trip time (ping-pong)");
    total = 0;
    // keep the RTT experiment running for ever
    while (1)
    {
        msg_id = -1;
        /* Measure Round Trip Time for (authenticated) CAN messages */
        TSC_TIMER_START(tsc_eval);
        do_send(&msp_ican, CAN_ID_PING, msg_ping, CAN_PAYLOAD_SIZE, /*block=*/1);
        while ((len = do_recv(&msp_ican, &msg_id, msg_pong, /*block=*/1)) < 0);
        TSC_TIMER_END(tsc_eval);
        dump_timer("round-trip time (cycles)");
        total += tsc_eval_get_interval();
    }
    dump_avg();
}
#endif

#ifdef BENCH_MAC_COMPUTATION
void VULCAN_FUNC eval_mac(void)
{
    pr_progress("Benchmarking MAC computation cycles");
    do_send(&msp_ican, CAN_ID_PING, msg_ping, CAN_PAYLOAD_SIZE, /*block=*/0);
    EXIT();
}
#endif

void VULCAN_ENTRY eval_run(void)
{
    int i, rv;
    uint8_t stop_msg = 0x0;
    eval_do_init(/*own=*/ CAN_ID_AEC_SEND, /*listen=*/ CAN_ID_AEC_RECV);
    for (i=0; i < CAN_PAYLOAD_SIZE; i++)
        msg_ping[i] = msg_ping_init[i];

    #ifdef BENCH_MAC_COMPUTATION
        eval_mac();
    #endif

    pr_progress("waiting for receiver to come up");
    sync_recv();

    #if BENCH_SEND
        eval_send();
    #endif

    #if BENCH_DEMO
        eval_demo();
    #endif

    #if BENCH_RTT
        eval_rtt();
    #endif

    pr_progress("sending stop signal to receiver process");
    rv = do_send(&msp_ican, CAN_ID_PING, &stop_msg, 1, /*block=*/1);
    ASSERT(rv >= 0);
    pr_progress("exiting");
}
