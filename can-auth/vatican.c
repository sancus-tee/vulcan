/*
 * vatican.c: A vatiCAN-compliant authenticated CAN implementation leveraging
 *            Sancus' hardware-level crypto primitives.
 *
 * See <https://distrinet.cs.kuleuven.be/software/vulcan/>
 *     <https://www.iacr.org/cryptodb/data/paper.php?pubkey=27855>
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
#include "vatican.h"
#include "../drivers/irq_can.h"
#include <sancus_support/sm_io.h>
#include <sancus_support/timer.h>
#include <sancus_support/tsc.h>

#define VATITACAN	1

// ============ VATICAN HELPER FUNCTIONS ============

VULCAN_DATA ican_link_info_t  *vatican_connections;
VULCAN_DATA size_t             vatican_nb_connections;
VULCAN_DATA ican_link_info_t  *vatican_cur;

// IAT channel variables
VULCAN_DATA uint8_t	       sleep;
VULCAN_DATA uint32_t	       delta = 4000; /* 2000 cycles */
VULCAN_DATA uint64_t	       iat_timings[8];
VULCAN_DATA uint8_t	       int_counter = 0;

// timer for measuring IAT values
DECLARE_TSC_TIMER(iat_timer);

// timer for measuring MAC calculation
DECLARE_TSC_TIMER(mac_timer);

// NOTE: this approach is currently _not_ thread-safe
void VULCAN_FUNC vatican_commit_nonce_increment(void)
{
    if (vatican_cur)
    {
        // NOTE: should request new random session key from trusted global
        // Attestation Server on nonce counter overflow (see VulCAN paper)
        ASSERT(vatican_cur->c != 0xFFFFFFFF);
        vatican_cur->c = vatican_cur->c + 1;
    }
    vatican_cur = NULL;
}

/* 
 * Calculates a MAC over (id | msg | nonce).
 * 
 * \arg id: 11-bit CAN identifier of message to authenticate
 * \arg msg: pointer to first byte of message to authenticate
 * \arg len: number of bytes of message to authenticate (max 8)
 * \arg mac: pointer to a 64-bit output buffer to hold the resulting MAC
 *
 * \ret: a value >= 0 when a MAC for the connection has been created
 */
int VULCAN_FUNC vatican_mac_create(uint8_t *mac, uint16_t id, uint8_t* msg,
                                   size_t len)
{
    ican_tag_t tag;
    vatican_ad_t ad;
    ican_buf_t *mac_out = (ican_buf_t*) mac;
    int i;

    // locate vatiCAN protocol info struct
    vatican_cur = NULL;
    for (i = 0; i < vatican_nb_connections; i++)
        if (vatican_connections[i].id == id)
        {
            vatican_cur = &vatican_connections[i];
            break;
        }
    if (!vatican_cur) return -EINVAL;

    // construct associated data (zero-pad msg); wait for final nonce increment
    // NOTE: use union type to avoid bit shifts and compile better code
    ad.doubles[0] = vatican_cur->c + 1;
    ad.doubles[1] = id;
    for (i=0; i < CAN_PAYLOAD_SIZE; i++)
        ad.bytes[VATICAN_NONCE_SIZE+CAN_SID_SIZE+i] = (i < len) ? msg[i] : 0x00;
    pr_debug_buf(ad.bytes, VATICAN_AD_SIZE, INFO_STR("AD"));
 
    // request MAC from hardware
    MAC_TIMER_START();
    i = sancus_tag_with_key(vatican_cur->k_i, ad.bytes,
                            VATICAN_AD_SIZE, tag.bytes);
    MAC_TIMER_END();
    ASSERT(i);
    pr_debug_buf(tag.bytes, SANCUS_TAG_SIZE, INFO_STR("Sancus TAG"));

    // truncate MAC to 64 bit output
    // NOTE: we discard LSB and keep the MSB part to adhere to AUTOSAR42
    mac_out->quad = tag.quads[1];
    pr_info_buf(mac, CAN_PAYLOAD_SIZE, INFO_STR("truncated MAC"));

    return 0;
}

// _unprotected_ helper function for transmitting data to untrusted CAN driver
#ifndef CAN_DRV_SM
    uint8_t u_msg_buf[CAN_PAYLOAD_SIZE];
    uint16_t u_id;

    int __attribute__((noinline)) u_can_send(ican_t *ican, uint16_t id,
                                             uint8_t len, int block)
    {
        int rv;
        while ((rv = ican_send(ican, id, u_msg_buf, len, block)) == -EAGAIN);
        return rv;
    }
#endif

int VULCAN_FUNC vatican_send(ican_t *ican, uint16_t id, uint8_t *buf,
                             uint8_t len, int block)
{
    int i, rv;
    pr_info3("sending CAN message: ID=0x%03x; len=%d; block=%d\n", \
             id, len, block);

    #if defined(VULCAN_SM) && !defined(CAN_DRV_SM)
        for (i = 0; i < len; i++)
            u_msg_buf[i] = buf[i];
        return u_can_send(ican, id, len, block);
    #else
        while ((rv = ican_send(ican, id, buf, len, block)) == -EAGAIN);
        return rv;
    #endif
}

void VULCAN_FUNC vatican_nonce_reset (uint32_t nonce)
{
    int i;
    pr_info2("resetting nonces to %#2x/%#2x (high/low)\n", nonce >> 16, nonce);

    for (i = 0; i < vatican_nb_connections; i++)
    {
        vatican_connections[i].c = nonce;
        pr_verbose_buf((uint8_t*) &vatican_connections[i],
                       sizeof(ican_link_info_t), INFO_STR("info_t"));
    }
}

int VULCAN_FUNC vatican_receive(ican_t *ican, uint16_t *id, uint8_t *buf,
                                int block)
{
    int i, rv;
    uint32_t tmp, nonce;

    #if defined(VULCAN_SM) && !defined(CAN_DRV_SM)
        rv = ican_recv(ican, &u_id, u_msg_buf, block);
        *id = u_id;
        for (i = 0; i < rv; i++)
            buf[i] = u_msg_buf[i];
    #else
        rv = ican_recv(ican, id, buf, block);
    #endif

    if (rv >= 0)
    {
        pr_info1("CAN message received: ID=0x%03x\n", *id);
        pr_info_buf(buf, rv, INFO_STR("data"));

        #ifdef VATICAN_INCLUDE_NONCE_GENERATOR
            // Unauthenticated Global Nonce Generator hack for testing..
            if (*id == VATICAN_ID_NONCE_GENERATOR)
            {
                pr_info("Nonce Generator message received..");
                ASSERT(rv == 4);

                nonce  = (tmp = buf[0]) << 24;
                nonce |= (tmp = buf[1]) << 16;
                nonce |= (tmp = buf[2]) << 8;
                nonce |= buf[3];

                vatican_nonce_reset(nonce);
                return -1;
            }
        #endif
    }

    return rv;
}

// ============ AUTHENTICATED CAN NETWORK INTERFACE ============

int VULCAN_FUNC vulcan_init(ican_t *ican, ican_link_info_t connections[],
                            size_t nb_connections)
{
    int rv;

    if (!connections) return -EINVAL;

    // NOTE: we assume fresh random session keys K_i have been securely
    // distributed by a trusted global Attestation Server at boot time (see
    // VulCAN paper), such that we can simply reset nonces here.
    vatican_connections = connections;
    vatican_nb_connections = nb_connections;
    vatican_nonce_reset(0x0);

    rv = ican_init(ican);
    ASSERT(rv >= 0);
    pr_info("CAN controller initialized");

    // Initialize timer for IAT nonces
    timer_init();

    #if VATITACAN
        ican_irq_init(ican);
	asm("eint\n\t");
    #endif

    return 0;
}

long encode_iat(uint32_t nonce);

int VULCAN_FUNC vulcan_send(ican_t *ican, uint16_t id, uint8_t *buf,
                            uint8_t len, int block)
{
    int rv = 0;
    uint8_t mac[CAN_PAYLOAD_SIZE];

    /* 1. send legacy CAN message (ID | payload) */
    // NOTE: do not block for ACK, such that we can start the MAC computation
    rv = vatican_send(ican, id, buf, len, /*block=*/0);

    /* 2. known authenticated connection ? send CAN authentication frame */
    if ((rv >= 0) && (vatican_mac_create(mac, id, buf, len) >= 0))
    {	
	#if VATITACAN

	    // Delay authentication message
            sleep = 0x1;
            timer_irq(encode_iat(vatican_cur->c+1));
            while (sleep)
            {
            	asm("nop"); // Prevent inlining
            }

	#endif

        rv = vatican_send(ican, id+1, mac, CAN_PAYLOAD_SIZE, block);
    }

    vatican_commit_nonce_increment();
    return rv;
}

uint32_t decode_iat(uint64_t iat);

int VULCAN_FUNC vulcan_recv(ican_t *ican, uint16_t *id, uint8_t *buf, int block)
{
    ican_buf_t mac_me;
    ican_buf_t mac_recv;
    uint16_t id_recv;
    uint32_t old_nonce;
    uint32_t iat_nonce;
    uint64_t mac_timing;
    int rv, recv_len, i, fail = 0;

    /* 1. receive any CAN message (ID | payload) */
    if ((rv = vatican_receive(ican, id, buf, block)) < 0)
        return rv;

    #if VATITACAN
        TSC_TIMER_START(mac_timer);
        if (vatican_mac_create(mac_me.bytes, *id, buf, rv) >= 0)
        {
            TSC_TIMER_END(mac_timer);
            mac_timing = mac_timer_get_interval();
            recv_len = vatican_receive(ican, &id_recv, mac_recv.bytes, /*block=*/1);
            iat_nonce = decode_iat(iat_timings[int_counter] - mac_timing);
            fail = (id_recv != *id + 1) || (recv_len != CAN_PAYLOAD_SIZE) ||
                (mac_me.quad != mac_recv.quad);
    	}
    #else
	/* 2. authenticated connection ? calculate and verify MAC */
        if (vatican_mac_create(mac_me.bytes, *id, buf, rv) >= 0)
        {
            recv_len = vatican_receive(ican, &id_recv, mac_recv.bytes, /*block=*/1);
            fail = (id_recv != *id + 1) || (recv_len != CAN_PAYLOAD_SIZE) ||
                (mac_me.quad != mac_recv.quad);
        }
    #endif

    #if VATITACAN
    	/* 3.0 Retry failed verification based on IAT */
    	if (fail)
    	{
	    old_nonce = vatican_cur->c;
		
	    // Enable only nonce increments
            if ((vatican_cur->c & 0x00000003) > iat_nonce)
            {
            	iat_nonce = iat_nonce + 4;
            }

	    // Retry authentication using IAT nonce
            vatican_cur->c = (vatican_cur->c & 0xffffffffc) + iat_nonce;
            if (vatican_mac_create(mac_me.bytes, *id, buf, rv) >= 0)
            {
           	fail = (id_recv != *id + 1) || (recv_len != CAN_PAYLOAD_SIZE) ||
                (mac_me.quad != mac_recv.quad);
            }

	    // Reset to original nonce if resynchronisation failed
	    if (fail)
	    {   
                vatican_cur->c = old_nonce;
            }
    	}
    #endif

    /* 3. drop messages with failed authentication; else increment nonce */
    if (fail)
    {
        // NOTE: VulCAN/vatiCAN nonce-resynchronisation strategy is left
        // application-dependent (see paper for details); VulCAN/LeiA
        // implementation comes with a resynchronisation mechanism.
        pr_info("ignoring wrongly authenticated message..");
        vatican_cur = NULL;
        return -EINVAL;
    }
    
    vatican_commit_nonce_increment();
    return rv;
}

/*******************************************************************/
/* IAT nonce synchronisation */
/*******************************************************************/

#ifdef VATITACAN
void VULCAN_FUNC iat_send_callback(void)
{
    timer_disable();
    sleep = 0x0;
}
#endif

#ifdef VATITACAN
long VULCAN_FUNC encode_iat(uint32_t nonce)
{
    uint32_t masked;

    // Retrieve last 2 nonce bits
    masked = nonce & 0x00000003;

    return (masked * delta);    
}
#endif

#ifdef VATITACAN
void VULCAN_FUNC iat_recv_callback(void)
{
    // Adjust message count
    int_counter = (int_counter+1)%8;    

    // Measure + store IAT
    TSC_TIMER_END(iat_timer);
    iat_timings[int_counter] = iat_timer_get_interval();
    TSC_TIMER_START(iat_timer);

    // Clear interrupt flag on MSP430
    P1IFG = P1IFG & 0xfc;
}
#endif

#ifdef VATITACAN
uint32_t VULCAN_FUNC decode_iat(uint64_t iat)
{
    uint32_t deltas;
    deltas = ((int)((iat + (delta/2))))/((int)(delta));
    return deltas;
}
#endif

#if VATITACAN
    CAN_ISR_ENTRY(iat_recv_callback);
    TIMER_ISR_ENTRY(iat_send_callback);
#endif
