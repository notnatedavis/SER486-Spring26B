#   !/bin/bash
#   install.sh
#   Sets up complete AVR microcontroller dev environment
#   w/ (AVR compiler toolchain, Custom simulavr, Patched GDB debugger)

cd ~
# update the software registry
sudo apt-get --assume-yes update		
sudo apt-get --assume-yes upgrade
# install avr build tools
sudo apt-get --assume-yes install gcc-avr binutils-avr gdb-avr avr-libc avrdude
# install tools required to build the simulator
sudo apt-get --assume-yes install build-essential g++ libtool-bin binutils-dev texinfo autoconf swig
sudo apt --assume-yes install gcc make git tk
git clone https://github.com/dsandy12/simulavr.git
# build the simulator
cd simulavr
./bootstrap
autoreconf -i
./configure --disable-doxygen-doc --enable-dependency-tracking
make
# patch and build gdb for avr
sudo apt-get --assume-yes --purge remove gdb-avr
wget ftp.gnu.org/gnu/gdb/gdb-8.1.1.tar.xz
tar xf gdb-8.1.1.tar.xz
cd gdb-8.1.1
perl -i -0pe 's/  ULONGEST addr = unpack_long \(type, buf\);\R\R  return avr_make_saddr \(addr\);\R/  ULONGEST addr = unpack_long (type, buf);\n\n  if (TYPE_DATA_SPACE (type))\n    return avr_make_saddr (addr);\n  else\n    return avr_make_iaddr (addr);\n/' gdb/avr-tdep.c
./configure --prefix=/usr --target=avr
make
sudo make install
cd ~

