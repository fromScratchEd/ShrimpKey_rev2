Drivers
======

Under Debian, only root has full access to USB devices.
To be able to upload sketches as non-root:
Copy 99-USBasp.rules to /etc/udev/rules.d and restart udev ((/etc/init.d/udev restart) (with sudo or as root)).

Under Ubuntu, look here (in German): http://lazyzero.de/elektronik/avr/usbasplinux

Under Windows, you need to install the usbasp-windriver.
For Windows 8 instructions, see http://letsmakerobots.com/node/36841