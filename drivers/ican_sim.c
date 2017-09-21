/*
 * An elementary ican interface implementation using local file I/O to emulate
 * an untrusted CAN bus in the simulator.
 *
 * \note: The implementation is not meant to be complete, and cannot be used
 * for macrobenchmark timing experiments as real-world CAN driver and network
 * delays are of course _not_ incorporated.
 */

#include "ican.h"
#include <errno.h>
#include <sancus_support/fileio.h>
#include <sancus_support/sm_io.h>

#define READ_BYTE(b)                        \
    do {                                    \
        /* Always block wait for data */    \
        while(!fileio_available());         \
        b = fileio_getc();                  \
    }                                       \
    while(0)

#define WRITE_BYTE(b)                       \
    fileio_putc(b)

int CAN_DRV_FUNC ican_init(ican_t *ican)
{
    // Message filters are not simulated; any available data is returned
    return 0;
}

int CAN_DRV_FUNC can_send(ican_t *ican, uint16_t id, uint16_t eid,
                          int is_ext, uint8_t *buf, uint8_t len, int block)
{
    int i;

    // Simulated frame layout: (sID (2B) | [eID (2B)] | len (1B) | data (<=8B))
    WRITE_BYTE(id >> 8);
    WRITE_BYTE(id);

    if (is_ext)
    {
        WRITE_BYTE(eid >> 8);
        WRITE_BYTE(eid);
    }

    WRITE_BYTE(len);
    for (i=0; i < len; i++)
        WRITE_BYTE(buf[i]);

#ifdef DEBUG
    pr_info1("Sent simCAN frame with ID=0x%03x\n", id);
    dump_buf(buf, len, INFO_STR("data"));
#endif

    return 0;
}

int CAN_DRV_FUNC ican_send(ican_t *ican, uint16_t id, uint8_t *buf, uint8_t len, int block)
{
    return can_send(ican, id, /*ext_id=*/ 0x0, /*is_ext=*/ 0, buf, len, block);
}

int CAN_DRV_FUNC ican_send_ext(ican_t *ican, ican_eid_t eid, uint8_t *buf,
                           uint8_t len, int block)
{
    return can_send(ican, eid.can_id, eid.ext_id, /*is_ext=*/ 1, buf, len, block);
}

int CAN_DRV_FUNC can_recv(ican_t *ican, uint16_t *id, uint16_t *eid,
                          uint8_t *buf, int block)
{
    int i; uint16_t len = 0x0;

    // Simulated frame layout: (sID (2B) | [eID (2B)] | len (1B) | data (<=8B))
    READ_BYTE(((char*) id)[1]);
    READ_BYTE(((char*) id)[0]);

    if (eid)
    {
        READ_BYTE(((char*) eid)[1]);
        READ_BYTE(((char*) eid)[0]);
    }

    READ_BYTE(len);
    for (i=0; i < len; i++)
        READ_BYTE(buf[i]);

#ifdef DEBUG
    pr_info1("Received simCAN frame with ID=0x%03x\n", *id);
    dump_buf(buf, len, INFO_STR("data"));
#endif

    return len;
}

int CAN_DRV_FUNC ican_recv(ican_t *ican, uint16_t *id, uint8_t *buf, int block)
{
    return can_recv(ican, id, /*ext_id=*/ NULL, buf, block);
}

int CAN_DRV_FUNC ican_recv_ext(ican_t *ican, ican_eid_t *eid, uint8_t *buf, int block)
{
    return can_recv(ican, &(eid->can_id), /*ext_id=*/ &(eid->ext_id), buf, block);
}


int CAN_DRV_FUNC ican_ioctl(ican_t *ican, uint8_t option, uint8_t val)
{
    switch (option)
    {
        case ICAN_IOCTL_ABORT:
            while (fileio_available()) fileio_getc();
            break;
        default:
            return -EINVAL;
    }

    return 0;
}
