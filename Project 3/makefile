TARGET = proj3
MCU = atmega328p
F_CPU = 16000000UL

SOURCES = main.c delay.c led.c config.c vpd.c eeprom.c util.c \
          tempfsm.c wdt.c lib_proj2.c alarm.c uartsocket.c temp.c
OBJECTS = $(SOURCES:.c=.o)

CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Werror

$(TARGET).elf: $(OBJECTS)
	$(CC) -mmcu=$(MCU) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET).elf

.PHONY: clean