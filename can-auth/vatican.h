/*
 * vatican.h: A vulcanized vatiCAN authentication protocol implementation.
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
#ifndef VATICAN_H_INC
#define VATICAN_H_INC

#include <stdint.h>
#include <sancus/sm_support.h>

#define VATICAN_NONCE_SIZE  4
#define VATICAN_AD_SIZE     (CAN_SID_SIZE+CAN_PAYLOAD_SIZE+VATICAN_NONCE_SIZE)

/* VatiTACAN */
#define VATITACAN               1
#define VATITACAN_NONCE_SIZE    2
#define VATITACAN_DELTA         1200

typedef struct {
    uint16_t        id;
    const uint8_t *k_i;
    uint32_t       c;
} protocol_info_t;

#include "vulcan.h"

#ifdef VATICAN_INCLUDE_NONCE_GENERATOR
    #warning vatiCAN's global nonce generator scheme is vulnerable to advanced \
             replay attacks; use only for demonstration purposes (!)

    #define VATICAN_ID_NONCE_GENERATOR  0xaa
    #undef  ICAN_VULCAN_F0_MASK
    #define ICAN_VULCAN_F0_MASK         VATICAN_ID_NONCE_GENERATOR
#endif

typedef union {
    uint8_t  bytes[VATICAN_AD_SIZE];
    uint16_t words[VATICAN_AD_SIZE/2];
    uint32_t doubles[VATICAN_AD_SIZE/4];
} vatican_ad_t;

#if SANCUS_TAG_SIZE != 16
    #error Sancus+vatiCAN requires 128-bit MACs
#endif

#endif
