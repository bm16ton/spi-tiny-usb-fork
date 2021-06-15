BUILDDIR = .

DEVICE = stm32f103
CORE = stm32/core
PERIPH = stm32/periph
DISCOVERY = stm32/discovery
USB = stm32/usb

SOURCES += main.c

SOURCES += $(BUILDDIR)/isr.c \
		   $(BUILDDIR)/descs.c \
		   $(BUILDDIR)/src/stm32f30x_rcc.c \
		   $(BUILDDIR)/stm32-utils/delay.c \
		   $(BUILDDIR)/stm32-utils/myprintf.c \
		   $(BUILDDIR)/stm32-utils/usb/kdusb/kdusb_lib.c






OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

INCLUDES += -I. \
			-Istm32-utils \
			-Istm32-utils/usb \
			-Istm32-utils/usb/kdusb \
			-I\


ELF = $(BUILDDIR)/program.elf
HEX = $(BUILDDIR)/program.hex
BIN = $(BUILDDIR)/program.bin

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy

CFLAGS  = -O0 -g -Wall -I. \
   -mcpu=cortex-m4 -mthumb \
   -mfpu=fpv4-sp-d16 -mfloat-abi=hard \


LDSCRIPT = main.ld
LDFLAGS += -T$(LDSCRIPT) -mthumb -mcpu=cortex-m4 -nostdlib

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

flash: $(BIN)
	st-flash write $(BIN) 0x8000000

clean:
	rm -rf build
