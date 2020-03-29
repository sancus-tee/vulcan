/* based on https://github.com/spirilis/mcp2515 */

/* MCP2515 SPI CAN device driver for MSP430
 */

#include <msp430.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sancus_support/sm_io.h>
#include "mcp2515.h"

/* ================= SPI I/O ================= */

#ifndef CAN_DRV_SM
    #include <sancus_support/spi.h>
    #define CAN_CS_LOW(id)      spi_select(id);
    #define CAN_CS_HIGH()       spi_deselect();
    #define spi_transfer(cmd)   spi_write_read_byte( cmd )
    #define spi_init()          spi_init(SpiCpol0, SpiCpha0, 1);
#else
    #include "../drivers/sm_mmio_spi.h"
    #define CAN_CS_LOW(id)      sm_mmio_spi_select(id)
    #define CAN_CS_HIGH()       sm_mmio_spi_deselect();
    #define spi_transfer(cmd)   sm_mmio_spi_write_read_byte( cmd )
    #define spi_init()          sm_mmio_spi_init();
#endif

void CAN_DRV_FUNC can_spi_command(ican_t *ican, uint8_t cmd)
{
	CAN_CS_LOW(ican->spi_dev);
	spi_transfer(cmd);
	CAN_CS_HIGH();
}

uint8_t CAN_DRV_FUNC can_spi_query(ican_t *ican, uint8_t cmd)
{
	uint8_t ret;

	CAN_CS_LOW(ican->spi_dev);
	spi_transfer(cmd);
	ret = spi_transfer(0xFF);
	CAN_CS_HIGH();
	return ret;
}

void CAN_DRV_FUNC can_r_reg(ican_t *ican, uint8_t addr, void *buf, uint8_t len)
{
	uint16_t i;
	uint8_t *sbuf = (uint8_t *)buf;

	CAN_CS_LOW(ican->spi_dev);
	spi_transfer(MCP2515_SPI_READ);
	spi_transfer(addr);
	for (i=0; i < len; i++) {
		sbuf[i] = spi_transfer(0xFF);
	}
	CAN_CS_HIGH();
}

// Retrieve standard/extended ID, and payload + clear IF (to allow next receive)
int CAN_DRV_FUNC can_r_rxb(ican_t *ican, uint8_t sidh, uint16_t *id,
                           uint16_t *eid, uint8_t *buf, uint8_t intf)
{
    uint8_t data[2];
    int len = 0x0;

	can_r_reg(ican, sidh, data, 2);
    *id = (data[0] << 3) | (data[1] >> 5);

    if (eid)
    {
        ASSERT(data[1] & 0x08); // IDE flag enabled ?
        *id |= data[1] << 14;
	    can_r_reg(ican, sidh+2, data, 2);
        *eid = (data[0] << 8) | (data[1]);
    }

    can_r_reg(ican, sidh+4, &len, 1);
    if (len> 8) len = 8;
    can_r_reg(ican, sidh+5, buf, len);

    can_w_bit(ican, MCP2515_CANINTF, intf, 0x00);
    return len;
}

void CAN_DRV_FUNC can_w_reg(ican_t *ican, uint8_t addr, void *buf, uint8_t len)
{
	uint16_t i;
	uint8_t *sbuf = (uint8_t *)buf;

	CAN_CS_LOW(ican->spi_dev);
	spi_transfer(MCP2515_SPI_WRITE);
	spi_transfer(addr);
	for (i=0; i < len; i++) {
		spi_transfer(sbuf[i]);
	}
	CAN_CS_HIGH();
}

void CAN_DRV_FUNC can_w_sid(ican_t *ican, uint8_t addr_low, uint8_t addr_high,
                            uint16_t sid, uint8_t sidl_mask)
{
    uint8_t data;

    data = (sid & 0x7F8) >> 3;
    can_w_reg(ican, addr_high, &data, 1); 
   	data = ((sid & 0x7) << 5) | sidl_mask;
    can_w_reg(ican, addr_low, &data, 1); 
}

void CAN_DRV_FUNC can_w_bit(ican_t *ican, uint8_t addr, uint8_t mask, uint8_t val)
{
	CAN_CS_LOW(ican->spi_dev);
	spi_transfer(MCP2515_SPI_BITMOD);
	spi_transfer(addr);
	spi_transfer(mask);
	spi_transfer(val);
	CAN_CS_HIGH();
}

/* ================= MAINTENANCE FUNCTIONS ================= */

/*
 Adjusted from $(can-calc-bit-timing mcp251x):
 (NOTE: corrected: 8MHz output should be 16MHz)
 
 Bit timing parameters for mcp251x with 16.000000 MHz ref clock
 nominal                                 real Bitrt   nom  real SampP
 Bitrate TQ[ns] PrS PhS1 PhS2 SJW BRP Bitrate Error SampP SampP Error CNF1 CNF2 CNF3
 1000000    125   2    3    2   1   1 1000000  0.0% 75.0% 75.0%  0.0% 0x00 0x91 0x01
  800000    125   3    4    2   1   1  800000  0.0% 80.0% 80.0%  0.0% 0x00 0x9a 0x01
  500000    125   6    7    2   1   1  500000  0.0% 87.5% 87.5%  0.0% 0x00 0xb5 0x01
  250000    250   6    7    2   1   2  250000  0.0% 87.5% 87.5%  0.0% 0x01 0xb5 0x01
  125000    500   6    7    2   1   4  125000  0.0% 87.5% 87.5%  0.0% 0x03 0xb5 0x01
  100000    625   6    7    2   1   5  100000  0.0% 87.5% 87.5%  0.0% 0x04 0xb5 0x01
   50000   1250   6    7    2   1  10   50000  0.0% 87.5% 87.5%  0.0% 0x09 0xb5 0x01
   20000   3125   6    7    2   1  25   20000  0.0% 87.5% 87.5%  0.0% 0x18 0xb5 0x01
   10000   6250   6    7    2   1  50   10000  0.0% 87.5% 87.5%  0.0% 0x31 0xb5 0x01
*/

int CAN_DRV_FUNC can_speed(ican_t *ican, can_baudrate_t baudrate)
{
    uint8_t cnf1, cnf2, cnf3;
    if (!ican) return -EINVAL;

    switch (baudrate)
    {
        case CAN_10_KHZ:
            cnf1 = 0x31;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

        case CAN_20_KHZ:
            cnf1 = 0x18;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

        case CAN_50_KHZ:
            cnf1 = 0x09;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

        case CAN_100_KHZ:
            cnf1 = 0x04;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

        case CAN_125_KHZ:
            cnf1 = 0x03;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

        case CAN_250_KHZ:
            cnf1 = 0x01;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

        case CAN_500_KHZ:
            cnf1 = 0x00;
            cnf2 = 0xb5;
            cnf3 = 0x01;
            break;

//XXX does not seem to work with USBtin (termination or PhS2 issue ?)
#if 0
        case CAN_800_KHZ:
            cnf1 = 0x00;
            cnf2 = 0x9a;
            cnf3 = 0x01;
            break;

        case CAN_1000_KHZ:
            cnf1 = 0x00;
            cnf2 = 0x91;
            cnf3 = 0x01;
            break;
#endif

        default:
            return -EINVAL;
    }

 	can_w_reg(ican, MCP2515_CNF1, &cnf1, 1);
 	can_w_reg(ican, MCP2515_CNF2, &cnf2, 1);
 	can_w_reg(ican, MCP2515_CNF3, &cnf3, 1);
    return 0;
}

int CAN_DRV_FUNC ican_init(ican_t *ican)
{
	int rv; uint8_t data = 0x00;
    if (!ican) return -EINVAL;

    // Initialize SPI interface and reset CAN controller
    spi_init();
	can_spi_command(ican, MCP2515_SPI_RESET);
	msp_sleep(160000);

    // Enter configuration mode
	data = MCP2515_CANCTRL_REQOP_CONFIGURATION;
	can_w_reg(ican, MCP2515_CANCTRL, &data, 1);

    // Zero-initialize some registers to be sure
    data = 0x0;
    can_w_reg(ican, MCP2515_CANINTE, &data, 1);
    can_w_reg(ican, MCP2515_TXRTSCTRL, &data, 1);
    can_w_reg(ican, MCP2515_BFPCTRL, &data, 1); 

    // Receive only valid messages with standard/extended identifiers
    data = MCP2515_RXB0CTRL_MODE_RECV_STD_OR_EXT;
    can_w_reg(ican, MCP2515_RXB0CTRL, &data, 1);
    data = MCP2515_RXB1CTRL_MODE_RECV_STD_OR_EXT;
    can_w_reg(ican, MCP2515_RXB1CTRL, &data, 1);

    // Initialize SID mask registers; extended ID masks are already reset to zero
    can_w_sid(ican, MCP2515_RXM0SIDL, MCP2515_RXM0SIDH, ican->rxm0, /*mask=*/0x00);
    can_w_sid(ican, MCP2515_RXM1SIDL, MCP2515_RXM1SIDH, ican->rxm1, /*mask=*/0x00);

    // Initialize SID filter registers; enable extended ID filters, if any
    can_w_sid(ican, MCP2515_RXF0SIDL, MCP2515_RXF0SIDH, ican->rxf0, 
              /*mask=*/(ican->rxf0 & ICAN_FILTER_EXTENDED) ? 0x08 : 0x00);
    can_w_sid(ican, MCP2515_RXF1SIDL, MCP2515_RXF1SIDH, ican->rxf1,
              /*mask=*/(ican->rxf1 & ICAN_FILTER_EXTENDED) ? 0x08 : 0x00);
    can_w_sid(ican, MCP2515_RXF2SIDL, MCP2515_RXF2SIDH, ican->rxf2,
              /*mask=*/(ican->rxf2 & ICAN_FILTER_EXTENDED) ? 0x08 : 0x00);
    can_w_sid(ican, MCP2515_RXF3SIDL, MCP2515_RXF3SIDH, ican->rxf3,
              /*mask=*/(ican->rxf3 & ICAN_FILTER_EXTENDED) ? 0x08 : 0x00);
    can_w_sid(ican, MCP2515_RXF4SIDL, MCP2515_RXF4SIDH, ican->rxf4,
              /*mask=*/(ican->rxf4 & ICAN_FILTER_EXTENDED) ? 0x08 : 0x00);
    can_w_sid(ican, MCP2515_RXF5SIDL, MCP2515_RXF5SIDH, ican->rxf5,
              /*mask=*/(ican->rxf5 & ICAN_FILTER_EXTENDED) ? 0x08 : 0x00);

    // Initialize requested CAN baudrate
    rv = can_speed(ican, ican->baudrate);

    // Enter normal mode
	data = MCP2515_CANCTRL_REQOP_NORMAL;
	can_w_reg(ican, MCP2515_CANCTRL, &data, 1);
    
    return rv;
}

/* ================= CAN MESSAGE TRANSMISSION ================= */

// 18-bit extended ID encoded in (MSB) id[15:14] eid[15:10] (LSB)
int CAN_DRV_FUNC can_send(ican_t *ican, uint16_t id, uint16_t eid, int is_ext,
                           uint8_t *buf, uint8_t len, int block)
{
    uint8_t prio = 0x03, txb0ctrl = 0x00, data = 0x00;

	if (len > 8 || !ican)
		return -EINVAL;

    // Send buffer available ?
    do {
        can_r_reg(ican, MCP2515_TXB0CTRL, &txb0ctrl, 1);
    } while (txb0ctrl & MCP2515_TXBCTRL_TXREQ);

    // Load 11-bit standard ID; and MSBs extended ID if any
    can_w_sid(ican, MCP2515_TXB0SIDL, MCP2515_TXB0SIDH, id,
              /*mask=*/ is_ext ? (0x08 | (id >> 14)) : 0x00);

    // Load remaining 16 extended ID bits, if any
    if (is_ext)
    {
        data = eid >> 8;
        can_w_reg(ican, MCP2515_TXB0EID8, &data, 1); 
        data = eid;
        can_w_reg(ican, MCP2515_TXB0EID0, &data, 1); 
    }
	
    // Load data payload
	can_w_reg(ican, MCP2515_TXB0DLC, &len, 1);
	can_w_reg(ican, MCP2515_TXB0D0, buf, len); // sequential registers

    // Always configure highest priority
	can_w_reg(ican, MCP2515_TXB0CTRL, &prio, 1);

    // Initiate transmission
    can_w_bit(ican, MCP2515_TXB0CTRL, MCP2515_TXBCTRL_TXREQ, 0x8);

    // Block waiting for transmission ACK ?
    if (!block) return 0;

    while (1)
    {
      can_r_reg(ican, MCP2515_TXB0CTRL, &txb0ctrl, 1);
      if ( !(txb0ctrl & MCP2515_TXBCTRL_TXREQ) )
          return 0;
    }
}

int CAN_DRV_FUNC ican_send(ican_t *ican, uint16_t id, uint8_t *buf, uint8_t len, int block)
{
    return can_send(ican, id, /*ext_id=*/ 0x0, /*is_ext=*/ MCP2515_FORCE_EID, buf, len, block);
}

int CAN_DRV_FUNC ican_send_ext(ican_t *ican, ican_eid_t eid, uint8_t *buf,
                           uint8_t len, int block)
{
    return can_send(ican, eid.can_id, eid.ext_id, /*is_ext=*/ 1, buf, len, block);
}

// Returns length of packet or -1 if nothing to read
// buf should be able to hold up to 8 bytes
int CAN_DRV_FUNC can_recv(ican_t *ican, uint16_t *id, uint16_t *eid, uint8_t *buf, int block)
{
	uint8_t canintf = 0x0, len = -EAGAIN;
    int rx = 0;
    if (!ican) return -EINVAL;

    // Unread data in RXB0/RXB1?
    do {
        can_r_reg(ican, MCP2515_CANINTF, &canintf, 1); 
	} while (!(canintf & (MCP2515_CANINTF_RX0IF | MCP2515_CANINTF_RX1IF)) && block);

    if (canintf & MCP2515_CANINTF_RX0IF)
        len = can_r_rxb(ican, MCP2515_RXB0SIDH, id, eid, buf, MCP2515_CANINTF_RX0IF);
    else if (canintf & MCP2515_CANINTF_RX1IF)
        len = can_r_rxb(ican, MCP2515_RXB1SIDH, id, eid, buf, MCP2515_CANINTF_RX1IF);
        
    return len;
}

int CAN_DRV_FUNC ican_recv(ican_t *ican, uint16_t *id, uint8_t *buf, int block)
{
    #if MCP2515_FORCE_EID
        uint16_t eid;
        return can_recv(ican, id, /*ext_id=*/ &eid, buf, block);
    #else
        return can_recv(ican, id, /*ext_id=*/ NULL, buf, block);
    #endif
}

int CAN_DRV_FUNC ican_recv_ext(ican_t *ican, ican_eid_t *eid, uint8_t *buf, int block)
{
    return can_recv(ican, &(eid->can_id), /*ext_id=*/ &(eid->ext_id), buf, block);
}

/* ================= MISCELLANEOUS OPTION SETTING  ================= */

int CAN_DRV_FUNC ican_ioctl(ican_t *ican, uint8_t option, uint8_t val)
{
    if (!ican) return -EINVAL;

	switch (option) {
		case MCP2515_OPTION_ONESHOT:
			if (val) {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_OSM, MCP2515_CANCTRL_OSM);
			} else {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_OSM, 0);
			}
			break;

		// Abort all pending transmissions.
		case MCP2515_OPTION_ABORT:
		case ICAN_IOCTL_ABORT:
            can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_ABAT, 0x00);
            can_w_bit(ican, MCP2515_CANINTF, MCP2515_CANINTF_RX0IF, 0x00);
            can_w_bit(ican, MCP2515_CANINTF, MCP2515_CANINTF_RX1IF, 0x00);
			break;

		case MCP2515_OPTION_LOOPBACK:
			if (val) {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_REQOP_MASK, MCP2515_CANCTRL_REQOP_LOOPBACK);
			} else {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_REQOP_MASK, MCP2515_CANCTRL_REQOP_NORMAL);
			}
			break;

		case MCP2515_OPTION_LISTEN_ONLY:
			if (val) {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_REQOP_MASK, MCP2515_CANCTRL_REQOP_LISTEN_ONLY);
			} else {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_REQOP_MASK, MCP2515_CANCTRL_REQOP_NORMAL);
			}
			break;

		case MCP2515_OPTION_SLEEP:
			if (val) {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_REQOP_MASK, MCP2515_CANCTRL_REQOP_SLEEP);
			} else {
				can_w_bit(ican, MCP2515_CANCTRL, MCP2515_CANCTRL_REQOP_MASK, MCP2515_CANCTRL_REQOP_NORMAL);
			}
			break;

		default:
			return -EINVAL;
	}
	return 0;
}

#ifndef __SANCUS_IO_QUIET
    void CAN_DRV_FUNC can_dump_regs(ican_t *ican)
    {
      uint8_t buf = 0xff;

      printf1("\tican->spi_dev=%d\n", ican->spi_dev);

      can_r_reg(ican, MCP2515_CANSTAT, &buf, 1);
      printf1("\tCANSTAT=0x%02x ", buf);

      can_r_reg(ican, MCP2515_CANCTRL, &buf, 1);
      printf1("CANCTRL=0x%02x ", buf);

      can_r_reg(ican, MCP2515_CANINTE, &buf, 1);
      printf1("CANINTE=0x%02x ", buf);

      can_r_reg(ican, MCP2515_CANINTF, &buf, 1);
      printf1("CANINTF=0x%02x ", buf);

      can_r_reg(ican, MCP2515_EFLG, &buf, 1);
      printf1("EFLG=0x%02x ", buf);

      can_r_reg(ican, MCP2515_TXB0CTRL, &buf, 1);
      printf1("TXB0CTRL=0x%02x ", buf);

      can_r_reg(ican, MCP2515_RXF2SIDH, &buf, 1);
      printf1("RXF2SIDH=0x%02x ", buf);

      can_r_reg(ican, MCP2515_RXF2SIDL, &buf, 1);
      printf1("RXF2SIDL=0x%02x\n", buf);

      can_r_reg(ican, MCP2515_RXF0SIDH, &buf, 1);
      printf1("RXF0SIDH=0x%02x ", buf);

      can_r_reg(ican, MCP2515_RXF0SIDL, &buf, 1);
      printf1("RXF0SIDL=0x%02x\n", buf);
    }
#endif

void CAN_DRV_FUNC msp_sleep(volatile uint32_t n)
{
  while (n > 0) {
    n--;
  }
  return;
}

/* ================= IRQ CAN ================= */

void ican_irq_init(ican_t *ican)
{
    // CAN module enable interrupt on receive
    can_w_bit(ican, MCP2515_CANINTE,  MCP2515_CANINTE_RX0IE, 0x01);
    can_w_bit(ican, MCP2515_CANINTE,  MCP2515_CANINTE_RX1IE, 0x02);
    
    // MSP P1.0 enable interrupt on negative edge
    P1IE = 0x01;
    P1IES = 0x01;
    P1IFG = 0x00;
}
