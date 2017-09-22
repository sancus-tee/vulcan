-include ../../Makefile.config

SOURCES = $(shell ls *.c) ../../drivers/sm_mmio_spi.c
TARGET = main.elf
DEV = /dev/ttyUSB2

# calculate/verify MACs and include driver in gateway SM
CFLAGS += -DVULCAN_SM=sm_gateway -DCAN_DRV_SM=sm_gateway

# NOTE: include (insecure) vatican nonce generator for demostration purposes
#       omit LeiA's AUTH_FAIL mechanism to save CAN IDs in demo scenario
ifeq ($(LIBVULCAN), vatican)
    CFLAGS += -DVATICAN_INCLUDE_NONCE_GENERATOR
else
    CFLAGS += -DLEIA_OMIT_AUTH_FAIL
endif

TARGET_NO_MAC = no_mac_$(TARGET)
OBJECTS = $(SOURCES:.c=.o) can-drv.o $(LIBVULCAN).o

all: $(TARGET)

#custom targets since intra-SM libraries cannot be shared
$(LIBVULCAN).o: $(CAN_AUTH_DIR)/$(LIBVULCAN).c
	$(CC) $(CFLAGS) -c $< -o $@

can-drv.o: ../../drivers/mcp2515.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_NO_MAC): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET): $(TARGET_NO_MAC)
	$(SANCUS_CRYPTO) $(CRYPTOFLAGS) -o $@ $<

load: $(TARGET)
	$(SANCUS_LOAD) -device $(DEV) -baudrate 115200 $<

clean:
	$(RM) $(TARGET) $(TARGET_NO_MAC) $(OBJECTS) *.o

