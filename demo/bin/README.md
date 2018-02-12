# Instructions to bring up the VulCAN Demo (on the "sancus" Raspberry Pi)

## Connect the Pi and the USB devices
- screen / data projector
- Pi power supply
- keyboard, mouse
- 2 USB hubs from the demo

## Power on USB hubs and dashboards
- this will be noisy unless you recently gave the engine an oil change
- first the small USB hub (power supply for the FPGAs)
- then the large USB hub (serial ports)
- finally the dashboards (separate 12V supply)

## Switch on USB ports on the big hub
- slowly and in order
- you should get the following serial devices:
```bash
$ ls /dev/ttyACM* /dev/ttyUSB*
/dev/ttyACM0  /dev/ttyUSB2  /dev/ttyUSB5  /dev/ttyUSB8
/dev/ttyUSB0  /dev/ttyUSB3  /dev/ttyUSB6  /dev/ttyUSB9
/dev/ttyUSB1  /dev/ttyUSB4  /dev/ttyUSB7
```

## Bring up USBtin:
```bash
$ cd Desktop/VulCAN
$ java -jar USBtinViewer_v1.3.jar
```
Select /dev/ttyACM0 and 500kbps; select "Monitor" tab/mode for a better overview; there should be lots of messages from the dashboards passing through.

## Load software images:
```bash
$ cd git/vulcan/demo/bin
$ make
```

If this fails, reset the ECU (little white button next to the USB power supply)
and repeat the upload. You can bring up individual ECUs by doing
```bash
$ cd git/vulcan/demo/bin
$ make ecs        # central
$ make gateway    # gateway
$ make rpm_sec    # left wheels
$ make rpm_plain  # right wheels
$ make screen     # serial debug terminal on central
```

All should be working now. There are demo videos and slides in
`Desktop/VulCAN/`.


------------------------------------------------------------------------------
If you ever feel like building the firmware images from source:

```bash
$ cd git/vulcan/demo
$ make -C ecu-tcs/ load
$ make -C ecu-gateway/ load
$ make -C ecu-rpm/ load
```

To bring up a network interface to work with CAN, first disable the
`USBtinViewer` then:
```bash
# slcan_attach -f -s6 -o /dev/ttyACM1
# slcand ttyACM1 slcan1
# ifconfig slcan1 up
# candump slcan1
```
