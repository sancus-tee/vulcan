-include ../Makefile.config

SOURCES = mcp2515.c sm_mmio_kypd.c sm_mmio_led.c sm_mmio_spi.c
OBJECTS = $(SOURCES:.c=.o)

all: $(OBJECTS)

ifdef SLLVM_INSTALL_DIR
sm_mmio_kypd.ll: SANCUS_TRANSFORM_FLAGS += --fix-data-section
sm_mmio_spi.ll : SANCUS_TRANSFORM_FLAGS += --fix-data-section
sm_mmio_led.ll : SANCUS_TRANSFORM_FLAGS += --fix-data-section
endif

clean:
	$(RM) $(SOURCES:.c=.ll) $(SOURCES:.c=.sllvm.ll) $(SOURCES:.c=.sancus.ll)
	$(RM) $(SOURCES:.c=.s)
