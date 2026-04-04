CC=avr-gcc
MCU=atmega328p
CFLAGS=-mmcu=$(MCU) -DF_CPU=16000000UL -Os -Wall -std=c99 -g
USER_SETTINGS=-I./ -DYOURNAME=\"Nathaniel Davis-Perez\"
LIBS=lib_hw5.a

# target build output
all: bin/homework4.hex
bin/homework4.elf: main.c temp.c; mkdir -p bin; $(CC) $(CFLAGS) $(USER_SETTINGS) -o $@ $^ $(LIBS)
bin/homework4.hex: bin/homework4.elf; avr-objcopy -O ihex -R .eeprom $< $@
clean:; rm -rf bin
# ignores targets
.PHONY: all clean