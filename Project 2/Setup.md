# Necessary Setup

### Fresh Install

- `sudo apt update`
- `sudo apt install -y gcc-avr avr-libc binutils-avr gdb-avr simavr`
    - `gcc-avr` – AVR C cross-compiler (builds code for atmega328p)
    - `avr-libc` – standard C library for AVR microcontrollers
    - `binutils-avr` – assembler, linker, + other binary utilities for AVR targets (full toolchain support)
    - `gdb-avr` – debug support inside sim
    - `simavr` – SimAVR actual simulation 

- validate installs
- `avr-gcc --version`
- `avrdude -v`
- `simavr -h`
- ``

### Configuration

- create a `.gdbinit` file in working directory
    - initialize with `avr-gdb -x .gdbinit file.elf`

### Simulate 

- assuming in directory of actions, `simavr -m atmega328p file.elf` 