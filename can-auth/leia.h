/*
 * leia.h: A vulcanized LeiA CAN authentication protocol implementation.
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
#ifndef LEIA_H_INC
#define LEIA_H_INC

#include <stdint.h>
#include <sancus/sm_support.h>

#define LEIA_EPOCH_SIZE     7
#define LEIA_KEY_SIZE       16
#define LEIA_COUNTER_SIZE   2
#define LEIA_AD_SIZE        (CAN_SID_SIZE+CAN_PAYLOAD_SIZE+LEIA_COUNTER_SIZE)

#define LEIA_CMD_MASK       0x03
#define LEIA_CMD_DATA       0x00
#define LEIA_CMD_MAC        0x01
#define LEIA_CMD_AEC_EPOCH  0x02
#define LEIA_CMD_AEC_MAC    0x03

#define LEIA_COUNT_MAX      0xFFFF
#define LEIA_EPOCH_MAX      0xFFFFFFFFFFFFFF

typedef struct {
    uint16_t        id;
    const uint8_t   *k_i;
    uint64_t        epoch;
    uint8_t         k_e[LEIA_KEY_SIZE];
    uint16_t        c;
} protocol_info_t;

#include "vulcan.h"

typedef union {
    uint8_t  bytes[LEIA_AD_SIZE];
    uint16_t words[LEIA_AD_SIZE/2];
    uint32_t doubles[LEIA_AD_SIZE/4];
} leia_ad_t;

#if (SANCUS_TAG_SIZE != 16) || (LEIA_KEY_SIZE != SANCUS_KEY_SIZE)
    #error Sancus+LeiA requires 128-bit MACs
#endif

#endif
