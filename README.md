Forked from https://github.com/KrystianD/spi-tiny-usb.git . created makefile replaced sum deprecated kernel calls, Fixed usb endpoints, Both firmware and module load and work. Enumerates as spidev device, gpio for led and interupt pin, and as UIO device. Will hopefully get around to removing IOU, adding gpios and replacing spidev with a mechanism to specify spi device platform info


spi-tiny-usb
============

SPI interface adapter over USB with Linux kernel module.

NOTE: you need to set your custom VID/PID pair for USB device descriptor as I do not provide any.
