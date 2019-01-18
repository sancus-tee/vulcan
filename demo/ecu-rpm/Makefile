-include ../../Makefile.config

SOURCES = $(shell ls *.c) ../../drivers/mcp2515.c ../../drivers/sm_mmio_led.c
TARGET = main.elf
DEV = /dev/ttyUSB4
DEV2 = /dev/ttyUSB6

DEV3 = /dev/ttyUSB7
DEV4 = /dev/ttyUSB8

# calculate and verify MACs in sm_rpm
CFLAGS += -DVULCAN_SM=sm_rpm -DLED_SM=sm_rpm

# NOTE: include (insecure) vatican nonce generator for demostration purposes
#       omit LeiA's AUTH_FAIL mechanism to save CAN IDs in demo scenario
ifeq ($(LIBVULCAN), vatican)
    CFLAGS += -DVATICAN_INCLUDE_NONCE_GENERATOR
else
    CFLAGS += -DLEIA_OMIT_AUTH_FAIL
endif

# hack to simulate unprotected legacy devices
ifneq ($(NO_RPM_AUTH),1)
    CFLAGS += -DRPM_AUTH=1 -DCAN_ID_RPM=0x20
else
    CFLAGS += -DCAN_ID_RPM=0x22
endif

TARGET_NO_MAC = no_mac_$(TARGET)
OBJECTS = $(SOURCES:.c=.o) sm_led.o $(LIBVULCAN).o

all: $(TARGET)

#custom targets since intra-SM libraries cannot be shared
ifndef SLLVM_INSTALL_DIR
$(LIBVULCAN).o: $(CAN_AUTH_DIR)/$(LIBVULCAN).c
	$(CC) $(CFLAGS) -c $< -o $@

sm_led.o: ../../drivers/sm_led.c
	$(CC) $(CFLAGS) -c $< -o $@
else
$(LIBVULCAN).ll: $(CAN_AUTH_DIR)/$(LIBVULCAN).c
	$(CC) $(CFLAGS) -S $< -o $@

sm_led.ll: ../../drivers/sm_led.c
	$(CC) $(CFLAGS) -S $< -o $@

../../drivers/sm_mmio_led.sancus.ll: \
	SANCUS_TRANSFORM_FLAGS += --fix-data-section
endif

$(TARGET_NO_MAC): $(OBJECTS)
	$(LD) $^ $(LDFLAGS) -o $@

$(TARGET): $(TARGET_NO_MAC)
	$(SANCUS_CRYPTO) $(CRYPTOFLAGS) -o $@ $<

load: clean $(TARGET)
	$(SANCUS_LOAD) -device $(DEV) -baudrate 115200 $(TARGET)
	$(SANCUS_LOAD) -device $(DEV2) -baudrate 115200 $(TARGET)
	$(MAKE) clean all NO_RPM_AUTH=1
	$(SANCUS_LOAD) -device $(DEV3) -baudrate 115200 $(TARGET)
	$(SANCUS_LOAD) -device $(DEV4) -baudrate 115200 $(TARGET)

clean:
	$(RM) $(TARGET) $(TARGET_NO_MAC) $(OBJECTS) *.o
	$(RM) $(SOURCES:.c=.ll) $(SOURCES:.c=.sllvm.ll) $(SOURCES:.c=.sancus.ll)
	$(RM) $(SOURCES:.c=.s)
	$(RM) can-drv.s can-drv.ll can-drv.sllvm.ll can-drv.sancus.ll
	$(RM) vatican.s vatican.ll vatican.sllvm.ll vatican.sancus.ll
	$(RM) sm_led.s sm_led.ll sm_led.sllvm.ll sm_led.sancus.ll
