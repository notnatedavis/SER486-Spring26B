# Makefile - SER486 Homework 4
# atmega328p temperature sensor w/ ADC

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

.PHONY: all clean

all: $(BINDIR)/$(TARGET).hex

$(BINDIR) :
	mkdir -p $(BINDIR)

$(BINDIR)/$(TARGET).elf: $(SRC) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	$(SIZE) $@

$(BINDIR)/$(TARGET).hex: $(BINDIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "\nRelease HEX image: $@"

clean :
	rm -rf $(BINDIR)