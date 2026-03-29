# makefile - SER486 Homework 2
# atmega AVR blink program

# toolchain
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

# target device
MCU = atmega328p
F_CPU = 16000000UL

# source & output
TARGET = homework2
SRC = homework2.c

# directories
BINDIR = bin

# flags
RELEASE_OPTIMIZATION = Os
USER_SETTINGS = -I ./ -DYOURNAME="Nathaniel Davis-Perez"
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -$(RELEASE_OPTIMIZATION) -Wall -std=c99 -g $(USER_SETTINGS)


# build targets

.PHONY: all clean

# build release .hex in bin/
all: $(BINDIR)/$(TARGET).hex

# create bin/ if it doesn't exist
$(BINDIR) :
	mkdir -p $(BINDIR)

# compile .c -> .elf
$(BINDIR)/$(TARGET).elf: $(SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(TARGET).elf $(SRC)
	$(SIZE) $(BINDIR)/$(TARGET).elf

# convert .elf -> .hex
$(BINDIR)/$(TARGET).hex: $(BINDIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $(BINDIR)/$(TARGET).elf $(BINDIR)/$(TARGET).hex
	@echo ""
	@echo "Release HEX image: $(BINDIR)/$(TARGET).hex"

# remove all build artifacts
clean:
	rm -rf $(BINDIR)