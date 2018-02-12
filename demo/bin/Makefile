
all: tcs gateway rpm_sec rpm_plain

tcs: tcs.elf
	sancus-loader -device /dev/ttyUSB0 tcs.elf

gateway: gateway.elf
	sancus-loader -device /dev/ttyUSB2 gateway.elf

rpm_sec: rpm_secure.elf
	sancus-loader -device /dev/ttyUSB4 rpm_secure.elf
	sancus-loader -device /dev/ttyUSB6 rpm_secure.elf

rpm_plain: rpm_insecure.elf
	sancus-loader -device /dev/ttyUSB7 rpm_insecure.elf
	sancus-loader -device /dev/ttyUSB8 rpm_insecure.elf

screen:
	screen /dev/ttyUSB1 115200

