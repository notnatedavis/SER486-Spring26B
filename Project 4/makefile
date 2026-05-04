SRCS = main.c lib_projd.c alarm.c config.c delay.c eeprom.c led.c temp.c tempfsm.c timer1.c uart.c vpd.c
OBJS = $(SRCS:.c=.o)

proj4.elf: $(OBJS)
	avr-gcc -mmcu=atmega328p -o proj4.elf $(OBJS)

%.o: %.c
	avr-gcc -mmcu=atmega328p -c -Os $< -o $@

clean:
	rm -f *.o *.elf

CFLAGS = -mmcu=atmega328p -Os -DF_CPU=16000000UL

%.o: %.c
	avr-gcc $(CFLAGS) -c $< -o $@