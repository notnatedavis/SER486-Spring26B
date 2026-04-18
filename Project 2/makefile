CC = avr-gcc
MCU = atmega328p
CFLAGS = -mmcu=$(MCU) -DF_CPU=16000000UL -Os -Wall -std=c99 -g
USER = -I./ -DYOURNAME='"Nathaniel Davis-Perez"'
LIBS = lib_proj2.a
SOURCES = main.c config.c eeprom.c util.c vpd.c
TARGET = proj2.elf

all: $(TARGET)

# link all sources into an ELF file
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(USER) -o $@ $^ $(LIBS)

# clean build artifacts
clean:
	rm -f $(TARGET)

# Phony targets (not real files)
.PHONY: all clean