/*
 * vulcan.h: An abstraction of a wrapped authenticated CAN network interface.
 *
 * See <https://distrinet.cs.kuleuven.be/software/vulcan/>
 *
 * This file is part of the VulCAN software stack.
 *
 * VulCAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * VulCAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with VulCAN. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef VULCAN_H_INC
#define VULCAN_H_INC

#include <stdint.h>
#include <sancus/sm_support.h>
#include "../drivers/ican.h"

/*
 * CAN authentication is normally performed in an SM to protect key material.
 *
 * \note: explicitly prevent compiler from inlining helper functions to avoid
 *        ugly/slow code generation and limit memory consumption.
 */
#ifndef VULCAN_SM
    #warning no VulCAN authentication SM specified; performing authentication in unprotected domain
    #define VULCAN_ENTRY
    #define VULCAN_FUNC   __attribute__((noinline))
    #define VULCAN_DATA
#else
    #define VULCAN_ENTRY  SM_ENTRY(VULCAN_SM)
    #define VULCAN_FUNC   __attribute__((noinline)) SM_FUNC(VULCAN_SM)
    #define VULCAN_DATA   SM_DATA(VULCAN_SM)
#endif

// Reduce simulation output to avoid Travis CI build timeouts
#ifdef TRAVIS
    #define pr_info_buf(buf, size, str)       dump_buf(buf, size, str)
    #define pr_debug_buf(buf, size, str)
    #define pr_verbose_buf(buf, size, str)
#else
    #define pr_info_buf(buf, size, str)       dump_buf(buf, size, str)
    #define pr_debug_buf(buf, size, str)      dump_buf(buf, size, str)
    #define pr_verbose_buf(buf, size, str)    dump_buf(buf, size, str)
#endif

#ifdef BENCH_MAC_COMPUTATION
    #include <stdio.h>
    #include <sancus_support/tsc.h>
    DECLARE_TSC_TIMER(mac_timer);
    #define MAC_TIMER_START() TSC_TIMER_START(mac_timer)
    #define MAC_TIMER_END()   TSC_TIMER_END(mac_timer); \
                              mac_timer_print_interval()
#else
    #define MAC_TIMER_START()
    #define MAC_TIMER_END()
#endif

// NOTE: protocol_info_t should be defined before this header
typedef protocol_info_t ican_link_info_t;

typedef union {
    uint8_t  bytes[SANCUS_TAG_SIZE];
    uint16_t words[SANCUS_TAG_SIZE/2];
    uint32_t doubles[SANCUS_TAG_SIZE/4];
    uint64_t quads[SANCUS_TAG_SIZE/8];
} ican_tag_t;

typedef union {
    uint8_t  bytes[CAN_PAYLOAD_SIZE];
    uint16_t words[CAN_PAYLOAD_SIZE/2];
    uint32_t doubles[CAN_PAYLOAD_SIZE/4];
    uint64_t quad;
} ican_buf_t;

int VULCAN_FUNC vulcan_init(ican_t *ican, ican_link_info_t connections[],
                              size_t nb_connections);

int VULCAN_FUNC vulcan_send(ican_t *ican, uint16_t id, uint8_t *buf,
                              uint8_t len, int block);

int VULCAN_FUNC vulcan_recv(ican_t *ican, uint16_t *id, uint8_t *buf,
                              int block);

int VULCAN_FUNC vulcan_send_iat(ican_t *ican, uint16_t id, uint8_t *buf,
                              uint8_t len, int block);

int VULCAN_FUNC vulcan_recv_iat(ican_t *ican, uint16_t *id, uint8_t *buf,
                              int block);

#define ICAN_VULCAN_F0_MASK    0x00

#define DECLARE_VULCAN_ICAN( ican, spi, rate, id0, id1 )            \
    DECLARE_ICAN( ican, spi, rate, ICAN_MASK_RECEIVE_SINGLE,        \
                  ICAN_MASK_RECEIVE_SINGLE_AUTH,                    \
                  0x0 | ICAN_VULCAN_F0_MASK, 0x00,                  \
                  id0 | ICAN_FILTER_EXTENDED, id0,                  \
                  id1 | ICAN_FILTER_EXTENDED, id1)

#endif
