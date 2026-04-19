CC = avr-gcc
MCU = atmega328p
CFLAGS = -mmcu=$(MCU) -DF_CPU=16000000UL -Os -Wall -std=c99 -g
USER = -I./ -DYOURNAME='"Nathaniel Davis-Perez"'

SOURCES = main.c config.c eeprom.c util.c vpd.c lib_proj2.c delay.c led.c
TARGET = proj2.elf

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(USER) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean