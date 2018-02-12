Instructions to bring up the VulCAN Demo
(on the "sancus" Raspberry Pi)

Connect the Pi and the USB devices
- keyboard, mouse
- 2 USB hubs from the demo

Power on USB hubs
- first the small one (power supply for the FPGAs)
- then the large one (serial ports)

Switch on USB ports on the big hub
- slowly and in order
- you should get the following serial devices:
$ ls /dev/ttyACM* /dev/ttyUSB*
/dev/ttyACM0  /dev/ttyUSB2  /dev/ttyUSB5  /dev/ttyUSB8
/dev/ttyUSB0  /dev/ttyUSB3  /dev/ttyUSB6  /dev/ttyUSB9
/dev/ttyUSB1  /dev/ttyUSB4  /dev/ttyUSB7

Load the software images:
$ cd git/vulcan/demo/bin
$ make

If this fails, reset the ECU (little white button next to the USB power supply)
and repeat the upload. You can bring up individual ECUs by doing
$ make ecs        # central
$ make gateway    # gateway
$ make rpm_sec    # left wheels
$ make rpm_plain  # right wheels
$ make screen     # serial debug terminal on central

Bring up usbTin:
$ cd Desktop/VulCAN
$ java -jar USBtinViewer_v1.3.jar
Select /dev/ttyACM0 and 500kbps

All should be working now. There are demo videos and slides in Desktop/VulCAN/.


If you evel feel like building the firmware images from source:
$ cd git/vulcan/demo
$ make -C ecu-tcs/ load
$ make -C ecu-gateway/ load
$ make -C ecu-rpm/ load

