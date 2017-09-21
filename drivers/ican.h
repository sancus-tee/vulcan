/*
 * An abstraction of a CAN network interface.
 */
#ifndef ICAN_H_INC
#define ICAN_H_INC

#include <stdint.h>

/* Data type definitions */

typedef enum
{
    CAN_10_KHZ,
    CAN_20_KHZ,
    CAN_50_KHZ,
    CAN_100_KHZ,
    CAN_125_KHZ,
    CAN_250_KHZ,
    CAN_500_KHZ,
    CAN_800_KHZ,
    CAN_1000_KHZ
} can_baudrate_t;

#define CAN_PAYLOAD_SIZE                8
#define CAN_SID_SIZE                    2
#define ICAN_SID_MASK                   0x7FF

#define ICAN_MASK_RECEIVE_ALL           0x0000
#define ICAN_MASK_RECEIVE_SINGLE        0xFFFF
// We use (ID+1) for the authentication frame
#define ICAN_MASK_RECEIVE_SINGLE_AUTH   0xFFFE
#define ICAN_FILTER_EXTENDED            0x8000

#define ICAN_IOCTL_ABORT                1

// NOTE: const to allow secure initialization in text section
typedef const struct {
    int             spi_dev;
    can_baudrate_t  baudrate;
    uint16_t        rxm0;
    uint16_t        rxm1;
    uint16_t        rxf0;
    uint16_t        rxf1;
    uint16_t        rxf2;
    uint16_t        rxf3;
    uint16_t        rxf4;
    uint16_t        rxf5;
} ican_t;

typedef union {
    struct {
        uint16_t can_id; // can_id[10:0] = id[10:0]; can_id[15:14] = eid[17:16]
        uint16_t ext_id; // eid[15:0]
    };
    uint32_t big;
} ican_eid_t;

/* Public interface */

// An _untrusted_ CAN interface is normally managed by an unprotected driver. A
// secure CAN gateway should place these functions in an SM.
#include <sancus/sm_support.h>
#ifndef CAN_DRV_SM
    #define CAN_DRV_FUNC
    #define CAN_DRV_DATA
#else
    #define CAN_DRV_FUNC   SM_FUNC(CAN_DRV_SM)
    #define CAN_DRV_DATA   SM_DATA(CAN_DRV_SM)
#endif

int CAN_DRV_FUNC ican_init(ican_t *ican);
int CAN_DRV_FUNC ican_send(ican_t *ican, uint16_t id, uint8_t *buf,
                           uint8_t len, int block);
int CAN_DRV_FUNC ican_send_ext(ican_t *ican, ican_eid_t eid, uint8_t *buf,
                           uint8_t len, int block);
int CAN_DRV_FUNC ican_recv(ican_t *ican, uint16_t *id, uint8_t *buf,
                           int block);
int CAN_DRV_FUNC ican_recv_ext(ican_t *ican, ican_eid_t *eid, uint8_t *buf,
                           int block);
int CAN_DRV_FUNC ican_ioctl(ican_t *ican, uint8_t option, uint8_t val);

#define DECLARE_ICAN( ican, spi, rate, m0, m1, f0, f1, f2, f3, f4, f5)      \
    CAN_DRV_DATA ican_t ican = {                                            \
        .spi_dev = spi,                                                     \
        .baudrate = rate,                                                   \
        .rxm0 = m0,                                                         \
        .rxm1 = m1,                                                         \
        .rxf0 = f0,                                                         \
        .rxf1 = f1,                                                         \
        .rxf2 = f2,                                                         \
        .rxf3 = f3,                                                         \
        .rxf4 = f4,                                                         \
        .rxf5 = f5,                                                         \
    }

#define DECLARE_ICAN_SIMPLEX( ican, spi, rate ) \
    DECLARE_ICAN( ican, spi, rate, ICAN_MASK_RECEIVE_SINGLE,                \
                  ICAN_MASK_RECEIVE_SINGLE, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0)

#endif
