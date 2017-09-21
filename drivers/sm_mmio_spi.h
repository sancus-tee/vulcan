#ifndef SM_MMIO_SPI_H_INC
#define SM_MMIO_SPI_H_INC

#include <stdint.h>
#include <sancus/sm_support.h>

extern struct SancusModule sm_mmio_spi;

void SM_MMIO_ENTRY(sm_mmio_spi) sm_mmio_spi_init(void);

void SM_MMIO_ENTRY(sm_mmio_spi) sm_mmio_spi_select(int dev);
#define sm_mmio_spi_deselect() sm_mmio_spi_select(0x0)

uint8_t SM_MMIO_ENTRY(sm_mmio_spi) sm_mmio_spi_write_read_byte(uint8_t data);

#endif
