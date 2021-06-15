BUILDDIR = ./

DEVICE = stm32f103
CORE = stm32-utils
PERIPH = stm32-utils
DISCOVERY = stm32-utils
USB = stm32-utils/usb

SOURCES += main.c

SOURCES += isr.c \
		   descs.c \
		   stm32-utils/delay.c \
		   stm32-utils/myprintf.c \
		   stm32-utils/usb/kdusb/kdusb_lib.c \


OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I. \
			-Istm32-utils \
			-Istm32-utils/usb \
			-Istm32-utils/usb/kdusb \
			-I/usr/lib/gcc-cross/arm-linux-gnueabi/8 \


ELF = $(BUILDDIR)/spi-tiny-usb.elf
HEX = $(BUILDDIR)/spi-tiny-usb.hex
BIN = $(BUILDDIR)/spi-tiny-usb.bin

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy

CFLAGS  = -O0 -g -Wall -I. -Ifkdusb -Istm32-utils -Istm32-utils/usb -Istm32-utils/usb/kdusb/ \
   -D__NO_SYSTEM_INIT  -nostartfiles \
   -lnosys -mcpu=cortex-m3 -L/usr/lib/gcc/arm-none-eabi/9.2.1/thumb/v7e-m+fp/softfp/ -lgcc \




LDSCRIPT = main.ld
LDFLAGS += -T$(LDSCRIPT) -nostartfiles
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -nostartfiles -static -o $@ $(OBJECTS) $(LDLIBS)  -L/usr/lib/gcc/arm-none-eabi/9.2.1/thumb/v7e-m+fp/softfp/ -lgcc

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@ -lm -lgcc

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@ -lgcc

flash: $(BIN)
	st-flash write $(BIN) 0x8000000

clean:
	rm $(OBJECTS)
