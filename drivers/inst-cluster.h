/*
 * CAN driver for a Volkswagen/Seat instrument cluster 6J0920906/A2C53332250.
 *
 * \See https://hackaday.io/project/6288-volkswagen-can-bus-gaming
 * \See http://www.wikican.uni-bayreuth.de/wikican/index.php?title=VW_Passat_B6
 */
#ifndef INST_CLUSTER_H_INC
#define INST_CLUSTER_H_INC

#include <stdint.h>

/*
 * By default use an authenticated CAN interface.
 *
 * NOTE: functions below are always inlined so they can be used in different
 * protection domains
 */
#ifndef ic_ican_send
    #define ic_ican_send vulcan_send
#endif

/* IC definitions */

#define IC_BAUDRATE         CAN_500_KHZ

#define IC_CAN_ID_RPM       0x280
#define IC_CAN_ID_IND       0x470

/* Function prototypes */

#define lo8(x) ((uint8_t)(x) & 0xFF)
#define hi8(x) ((uint8_t)(x) >> 8)

__always_inline int ic_rpm(ican_t *ican, int rpm)
{
    volatile uint8_t buf[8];
    rpm += rpm/2 + rpm/20; // instrument cluster scale (1.55)

    // NOTE: explicit initialization to avoid memset invocation by compiler
    buf[0] = 0x49;
    buf[1] = 0x0E;
    buf[2] = hi8(rpm);
    buf[3] = lo8(rpm);
    buf[4] = 0x0E;
    buf[5] = 0x00;
    buf[6] = 0x1B;
    buf[7] = 0x0E;

    return ic_ican_send(ican, IC_CAN_ID_RPM, (uint8_t*) buf, 8, /*block=*/1);
}

__always_inline int ic_ind(ican_t *ican, int blink_left, int blink_right, int warn)
{
    /* 
     * NOTE: IC_CAN_ID_IND is also used for other indicators: CanSend(0x470,
     * temp_battery_warning + temp_turning_lights, temp_trunklid_open +
     * door_open, backlight, 0x00, temp_check_lamp + temp_clutch_control,
     * temp_keybattery_warning, 0x00, lightmode);
     */
    uint8_t turn = (blink_left != 0) | ((blink_right != 0) << 1);
    volatile uint8_t buf[8];

    // NOTE: explicit initialization to avoid memset invocation by compiler
    buf[0] = turn;
    buf[1] = (warn != 0); //TODO "door open"; find a nicer warning indicator..
    buf[2] = 0x01;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;

    return ic_ican_send(ican, IC_CAN_ID_IND, (uint8_t*) buf, 8, /*block=*/1);
}

#endif
