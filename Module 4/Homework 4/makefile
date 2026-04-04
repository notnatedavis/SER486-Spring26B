# makefile - SER486 Homework 4
# atmega328p temperature sensor w/ ADC

# constants
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
MCU = atmega328p
F_CPU = 16000000UL
TARGET = homework4
SRC = main.c temp.c
LIBS = lib_hw5.a
BINDIR = bin
USER_SETTINGS = -I ./ -DYOURNAME="Nathaniel Davis-Perez"
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -std=c99 -g $(USER_SETTINGS)

# ignores targets
.PHONY: all clean

# target build output
all: $(BINDIR)/$(TARGET).hex

clean :
	rm -rf $(BINDIR)

# if bin/ doesn't exist, creates it to store output
$(BINDIR) :
	mkdir -p $(BINDIR)

# check if all exist
# compiles the two .c files & links with lib_hw5.a to create .elf file
# $(SIZE) $@ runs avr-size on the .elf file to report stats usage
$(BINDIR)/$(TARGET).elf: $(SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	$(SIZE) $@

# checks if .elf exists
# use OBJCOPY to convert .elf to .hex w/ removal of .eeprom section
# echo if successful
$(BINDIR)/$(TARGET).hex: $(BINDIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "\nRelease HEX image: $@"