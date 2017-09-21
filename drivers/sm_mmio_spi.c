#include "sm_mmio_spi.h"

#define SPI_BASE            0x0150
#define SPI_DATA            *(volatile uint8_t*)(SPI_BASE)
#define SPI_CNTRL           *(volatile uint8_t*)(SPI_BASE+1)
#define SPI_STATUS          *(volatile uint8_t*)(SPI_BASE+2)

#define SPI_CPOL            0
#define SPI_CPHA            1
#define SPI_SELECT          2
#define SPI_CLK_DIV         4

#define SPI_BUSY            0x01

DECLARE_EXCLUSIVE_MMIO_SM(sm_mmio_spi,
                          /*[secret_start, end[=*/ SPI_BASE, SPI_BASE+3,
                          /*caller_id=*/ 1,
                          /*vendor_id=*/ 0x1234);

void SM_MMIO_ENTRY(sm_mmio_spi) sm_mmio_spi_init(void)
{
    asm(
    /* Hard-code cpol=SpiCpol0, cpha=SpiCpha0, clk_div=1 */  
    "mov.b #0x10, %0                                                    \n\t" 
    ::"m"(SPI_CNTRL):
    );
}

void SM_MMIO_ENTRY(sm_mmio_spi) sm_mmio_spi_select(int dev)
{
    asm(
    /* two-bit device id in CNTRL[3:2] */
    "and.b #0x3, r15                                                    \n\t"
    "rla.b r15                                                          \n\t"
    "rla.b r15                                                          \n\t"
    "bis.b r15, %0                                                      \n\t"
    ::"m"(SPI_CNTRL):
    );
}

uint8_t SM_MMIO_ENTRY(sm_mmio_spi) sm_mmio_spi_write_read_byte(uint8_t data)
{
    asm(
    /* SPI_DATA = data; */
    "mov.b r15, %0                                                      \n\t"
    /* while (SPI_STATUS & SPI_BUSY) {} */
    "1: tst.b %1                                                        \n\t"
    "jnz 1b                                                             \n\t"
    /* return SPI_DATA; */
    "mov.b %0, r15                                                      \n\t"
    ::"m"(SPI_DATA), "m"(SPI_STATUS):
    );
}
