-include ../../Makefile.config

SOURCES             = $(shell ls *.c ../common/*.c)
APP                 = main
FPGA_DEV            = /dev/ttyUSB0

ifdef CAN_DRV_SM
    SOURCES += ../../drivers/sm_mmio_spi.c
endif

OBJECTS             = $(SOURCES:.c=.o) $(LIBVULCAN).o
TARGET              = $(APP).elf
TARGET_NO_MAC       = no_mac_$(TARGET)
TARGET_SIM          = $(APP)_sim.elf
TARGET_SIM_NO_MAC   = no_mac_$(TARGET_SIM)

all: $(TARGET) $(TARGET_SIM)

#custom targets since intra-SM libraries cannot be shared
ifndef SLLVM_INSTALL_DIR
$(LIBVULCAN).o: $(CAN_AUTH_DIR)/$(LIBVULCAN).c
	$(CC) $(CFLAGS) -c $< -o $@

can-drv.o: ../../drivers/mcp2515.c
	$(CC) $(CFLAGS) -c $< -o $@

can-sim.o: ../../drivers/ican_sim.c
	$(CC) $(CFLAGS) -c $< -o $@
else
$(LIBVULCAN).ll: $(CAN_AUTH_DIR)/$(LIBVULCAN).c
	$(CC) $(CFLAGS) -S $< -o $@

can-drv.ll     : ../../drivers/mcp2515.c
	$(CC) $(CFLAGS) -S $< -o $@

can-sim.ll     : ../../drivers/ican_sim.c
	$(CC) $(CFLAGS) -S $< -o $@
endif

$(TARGET_NO_MAC): $(OBJECTS) can-drv.o
	$(LD) $^ $(LDFLAGS) -o $@

$(TARGET_SIM_NO_MAC): $(OBJECTS) can-sim.o
	$(LD) $^ $(LDFLAGS) -o $@

%.elf: no_mac_%.elf
	$(SANCUS_CRYPTO) $(CRYPTOFLAGS) -o $@ $<

load: $(TARGET)
	$(SANCUS_LOAD) $(LOADFLAGS) $<

sim: $(TARGET_SIM)
	$(SANCUS_SIM) $(SIMFLAGS) --fileio-in ../can-ping.bin --fileio-out ../can-pong.bin $<

background-sim: $(TARGET_SIM)
	$(SANCUS_SIM) $(SIMFLAGS) --fileio-in ../can-ping.bin --fileio-out ../can-pong.bin $< > sim-recv-out.txt 2>&1 &

clean:
	$(RM) *.elf *.o *.bin *.fst $(OBJECTS)
	$(RM) $(SOURCES:.c=.ll) $(SOURCES:.c=.sllvm.ll) $(SOURCES:.c=.sancus.ll)
	$(RM) $(SOURCES:.c=.s)
	$(RM) can-drv.s can-drv.ll can-drv.sllvm.ll can-drv.sancus.ll
	$(RM) can-sim.s can-sim.ll can-sim.sllvm.ll can-sim.sancus.ll
	$(RM) vatican.s vatican.ll vatican.sllvm.ll vatican.sancus.ll
