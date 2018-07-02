/*
 * leia.c: A LeiA-compliant authenticated CAN implementation leveraging Sancus'
 *         hardware-level crypto primitives.
 *
 * See <https://distrinet.cs.kuleuven.be/software/vulcan/>
 *     <https://www.cs.bham.ac.uk/~garciaf/publications/leia.pdf>
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
#include "leia.h"
#include <sancus_support/sm_io.h>

// ============ LEIA HELPER FUNCTIONS ============

VULCAN_DATA ican_link_info_t  *leia_connections;
VULCAN_DATA size_t             leia_nb_connections;
VULCAN_DATA ican_link_info_t  *leia_cur;
VULCAN_DATA ican_link_info_t  *leia_aec;

/* 
 * Calculates a MAC over (id | msg | nonce).
 * 
 * \arg id: 11-bit CAN identifier of message to authenticate
 * \arg msg: pointer to first byte of message to authenticate
 * \arg len: number of bytes of message to authenticate (max 8)
 * \arg mac: pointer to a 64-bit output buffer to hold the resulting MAC
 */
void VULCAN_FUNC leia_mac_create(uint8_t *mac, uint16_t id, uint8_t* msg,
                                 int len, uint16_t counter)
{
    leia_ad_t ad;
    ican_tag_t tag;
    ican_buf_t *mac_out = (ican_buf_t*) mac;
    int i;
    ASSERT(leia_cur);

    // construct associated data (zero-pad msg)
    // NOTE: use union type to avoid bit shifts and compile better code
    // NOTE: include msg ID such that multiple connections can share same K_i
    ad.words[0] = counter;
    ad.words[1] = id;
    for (i=0; i < CAN_PAYLOAD_SIZE; i++)
        ad.bytes[LEIA_COUNTER_SIZE+CAN_SID_SIZE+i] = (i < len) ? msg[i] : 0x00;
    pr_debug_buf(ad.bytes, LEIA_AD_SIZE, INFO_STR("AD"));
 
    // request MAC from hardware
    MAC_TIMER_START();
    i = sancus_tag_with_key(leia_cur->k_e, ad.bytes,
                                LEIA_AD_SIZE, tag.bytes);
    MAC_TIMER_END();
    ASSERT(i);
    pr_debug_buf(tag.bytes, SANCUS_TAG_SIZE, INFO_STR("Sancus TAG"));

    // truncate MAC to 64 bit output
    // NOTE: we discard LSB and keep the MSB part to adhere to AUTOSAR42
    mac_out->quad = tag.quads[1];
    pr_info_buf(mac, CAN_PAYLOAD_SIZE, INFO_STR("truncated MAC"));
}

void VULCAN_FUNC leia_session_key_gen(void)
{
    int rv;
    ASSERT(leia_cur);

    // 1. increment epoch
    // NOTE: should request new random session key from trusted global
    // Attestation Server on epoch counter overflow (see VulCAN paper)
    ASSERT(leia_cur->epoch != LEIA_EPOCH_MAX);
    leia_cur->epoch++;

    // 2. apply the MAC algorithm on the epoch
    rv = sancus_tag_with_key(leia_cur->k_i, &leia_cur->epoch,
                             LEIA_EPOCH_SIZE, leia_cur->k_e);
    ASSERT(rv);

    // 3. reset counter (zero reserved to await AUTH_FAIL response)
    leia_cur->c = 0x1;

    pr_verbose_buf((uint8_t*)leia_cur, sizeof(ican_link_info_t), INFO_STR("info_t"));
}

void VULCAN_FUNC leia_update_counters(void)
{
    if (!leia_cur) return;

    if (leia_cur->c == LEIA_COUNT_MAX)
    {
        leia_session_key_gen();
    }
    else
        leia_cur->c++;

    leia_cur = NULL;
}

// _unprotected_ helper function for transmitting data to untrusted CAN driver
#ifndef CAN_DRV_SM
    uint8_t u_msg_buf[CAN_PAYLOAD_SIZE];
    ican_eid_t u_eid;

    int __attribute__((noinline)) u_can_send_ext(ican_t *ican, uint8_t len, 
                                                 int block)
    {
        int rv;
        while ((rv = ican_send_ext(ican, u_eid, u_msg_buf, len, block)) < 0);
        return rv;
    }
#endif

int VULCAN_FUNC leia_send(ican_t *ican, uint16_t id, uint16_t cmd,
                          uint16_t counter, uint8_t *buf, uint8_t len,
                          int block, int is_aec)
{
    int i, rv;
    ican_eid_t eid;
    if (is_aec) cmd += 2; // hack to work around compiler bug

    eid.can_id = (cmd << 14) | (id & ICAN_SID_MASK);
    eid.ext_id = counter;
    pr_info3("sending CAN message: ID=0x%04x; eID=0x%04x; len=%d\n", \
             eid.can_id, eid.ext_id, len);

    #if defined(VULCAN_SM) && !defined(CAN_DRV_SM)
        for (i = 0; i < len; i++)
            u_msg_buf[i] = buf[i];
        u_eid = eid;
        return u_can_send_ext(ican, len, block);
    #else
        while ((rv = ican_send_ext(ican, eid, buf, len, block)) < 0);
        return rv;
    #endif
}

int VULCAN_FUNC leia_receive(ican_t *ican, uint16_t *id, uint8_t *buf,
                             uint8_t *cmd, uint16_t *counter, int block)
{
    int i, rv;
    ican_eid_t eid;

    #if defined(VULCAN_SM) && !defined(CAN_DRV_SM)
        rv = ican_recv_ext(ican, &u_eid, u_msg_buf, block);
        eid = u_eid;
        for (i = 0; i < rv; i++)
            buf[i] = u_msg_buf[i];
    #else
        rv = ican_recv_ext(ican, &eid, buf, block);
    #endif

    if (rv >= 0)
    {
        *id = eid.can_id & ICAN_SID_MASK;
        *cmd = eid.can_id >> 14;
        *counter = eid.ext_id;

        pr_info3("CAN message received: ID=0x%03x; cmd=0x%02x; cnt=0x%04x\n", \
                 *id, *cmd, *counter);
        pr_info_buf(buf, rv, INFO_STR("data"));
    }

    return rv;
}

int VULCAN_FUNC leia_find_connection(uint16_t id)
{
    int i;
    leia_cur = NULL;

    for (i = 0; i < leia_nb_connections; i++)
        if (leia_connections[i].id == id)
        {
            leia_cur = &leia_connections[i];
            return 1;
        }

    return 0;
}

// ============ AUTHENTICATED CAN NETWORK INTERFACE ============

int VULCAN_FUNC vulcan_init(ican_t *ican, ican_link_info_t connections[],
                            size_t nb_connections)
{
    int i;

    // At least one connection (last one) for Authentication Error Channel
    if (!connections || (nb_connections < 1)) return -EINVAL;
    leia_connections = connections;
    leia_nb_connections = nb_connections;
    leia_aec = &leia_connections[leia_nb_connections-1];

    // NOTE: we assume fresh random session keys K_i have been securely
    // distributed by a trusted global Attestation Server at boot time (see
    // VulCAN paper), such that we can simply reset nonces here.
    for (i = 0; i < leia_nb_connections; i++)
    {
        leia_cur = &leia_connections[i];
        leia_cur->epoch = 0x0;

        leia_session_key_gen();
    }

    i = ican_init(ican);
    ASSERT(i >= 0);
    pr_info("CAN controller initialized");

    return 0;
}

int VULCAN_FUNC leia_auth_send(ican_t *ican, uint16_t id, uint8_t *buf,
                               uint8_t len, int block, int is_aec)
{
    int rv = 0;
    uint8_t mac[CAN_PAYLOAD_SIZE];
    uint16_t c = 0x0;
    uint16_t mac_id = (id == leia_aec->id) ? id : id + 1;

    /* 0. traverse registered connections list */
    if (leia_find_connection(id)) c = leia_cur->c;

    /* 1. send extended CAN message (ID | cmd | counter | payload) */
    // NOTE: do not block for ACK, such that we can start the MAC computation
    rv = leia_send(ican, id, LEIA_CMD_DATA, c, buf, len, /*block=*/0, is_aec);

    /* 2. authenticated connection ? send CAN authentication frame */
    if ((rv >= 0) && leia_cur)
    {
        leia_mac_create(mac, id, buf, len, leia_cur->c);
        rv = leia_send(ican, mac_id, LEIA_CMD_MAC, c, mac, CAN_PAYLOAD_SIZE,
                       block, is_aec);

        // NOTE: we only update counters _after_ sending such that receivers
        // can update after verification succeeded; this is safe as
        // vulcan_init initializes counters and session key
        leia_update_counters();
    }

    return rv;
}

int VULCAN_FUNC vulcan_send(ican_t *ican, uint16_t id, uint8_t *buf,
                            uint8_t len, int block)
{
    return leia_auth_send(ican, id, buf, len, block, /*is_aec=*/0);
}

int VULCAN_FUNC leia_auth_fail_send(ican_t *ican, uint16_t id)
{
    int rv;
    ican_buf_t msg;
    ASSERT(leia_cur && leia_aec);
    pr_info1("\tauthentication failure ID 0x%03x; sending AUTH_FAIL\n", id);

    // zero count to indicate connection awaits AUTH_FAIL response
    leia_cur->c = 0x0;

    // send authenticated AUTH_FAIL message: (id_failed | aec_epoch)
    // NOTE: we currently do not implement AEC resynchronisation (in case
    // authentication fails for the AUTH_FAIL message itself). This could be
    // resolved locally, as the AUTH_FAIL signal includes the epoch.
    msg.quad = leia_aec->epoch;
    msg.words[(CAN_PAYLOAD_SIZE/2)-1] = id;
    leia_auth_send(ican, leia_aec->id, msg.bytes, CAN_PAYLOAD_SIZE,
                   /*block=*/1, /*is_aec=*/1);

    return -EAGAIN;
}

int VULCAN_FUNC leia_auth_fail_receive(uint16_t id, uint64_t *epoch)
{
    ASSERT(leia_cur);
    // NOTE: new epoch should be strictly higher to prevent replay attacks
    if (*epoch <= leia_cur->epoch) return -EINVAL;

    pr_info1("\treceived valid AUTH_FAIL response for ID 0x%03x\n", id);
    leia_cur->epoch = *epoch-1;
    leia_session_key_gen(); 

    return 0;
}

int VULCAN_FUNC leia_auth_fail_send_response(ican_t *ican, uint16_t id)
{
    int rv;
    pr_info1("\treceived valid AUTH_FAIL signal for ID 0x%03x\n", id);

    rv = leia_find_connection(id);
    ASSERT(rv);
    leia_session_key_gen();
    leia_auth_send(ican, id, (uint8_t*) &leia_cur->epoch, CAN_PAYLOAD_SIZE,
                   /*block=*/1, /*is_aec=*/1);

    // NOTE: sender should back off and re-send message after receiver has
    // processed auth fail response
    return -EINPROGRESS;
}

int VULCAN_FUNC vulcan_recv(ican_t *ican, uint16_t *id, uint8_t *buf, int block)
// __attribute__((optnone))
{
    ican_buf_t mac_me, mac_recv;
    volatile uint8_t cmd, cmd_expected, cmd_mac;
    // ^^volatile to work around compiler bug
    uint16_t c, id_mac, id_mac_expected;
    int rv, len_mac, auth_fail_sig, auth_fail_resp = 0, fail = 0;

    /* 1. receive any extended CAN message (ID | cmd | counter | payload) */
    if (((rv = leia_receive(ican, id, buf, (uint8_t*) &cmd, &c, block)) < 0) ||
         ((cmd != LEIA_CMD_DATA) && (cmd != LEIA_CMD_AEC_EPOCH)))
        return -EINVAL;

    /* 2. authenticated connection ? process AUTH_FAIL response, if any */
    if (!leia_find_connection(*id)) return rv;
    if ((auth_fail_resp = ((cmd == LEIA_CMD_AEC_EPOCH) && !leia_cur->c)))
    {
        ASSERT(rv == CAN_PAYLOAD_SIZE);
        leia_auth_fail_receive(*id, (uint64_t*) buf);
    }

    /* 3. calculate and verify MAC */
    if (!(fail |= (c < leia_cur->c)))
    {
        leia_mac_create(mac_me.bytes, *id, buf, rv, c);
        len_mac = leia_receive(ican, &id_mac, mac_recv.bytes, (uint8_t*)
                               &cmd_mac, &c, /*block=*/1);
    }

    /* 4. authentication failed ? trigger resynchronisation procedure */
    cmd_expected = (cmd==LEIA_CMD_AEC_EPOCH) ? LEIA_CMD_AEC_MAC : LEIA_CMD_MAC;
    auth_fail_sig = (cmd==LEIA_CMD_AEC_EPOCH) && !auth_fail_resp;
    id_mac_expected = auth_fail_sig ? *id : *id + 1;

    if (fail || (cmd_mac != cmd_expected) || (id_mac != id_mac_expected) ||
       (len_mac != CAN_PAYLOAD_SIZE) || (mac_me.quad != mac_recv.quad))
    {
        #ifndef LEIA_OMIT_AUTH_FAIL
            return leia_auth_fail_send(ican, *id);
        #else
            pr_info("rejecting invalid message");
            return -EINVAL;
        #endif
    }
    
    /* 5. only update counters after successful message verification */
    leia_update_counters();

    /* 6. respond to valid received AUTH_FAIL signal, if any */
    if (auth_fail_sig)
    {
        c = (uint16_t) buf[CAN_PAYLOAD_SIZE-2];
        return leia_auth_fail_send_response(ican, c);
    }

    return rv;
}
