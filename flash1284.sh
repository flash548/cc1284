#! sh
rm *.o
avr-gcc -w -g -Os -DAVR_TARGET -UPC_TARGET -mmcu=atmega1284 -c *.cpp \
    && avr-gcc -g -DAVR_TARGET -UPC_TARGET -mmcu=atmega1284 -o basic.elf *.o \
    && avr-objcopy -j .text -j .data -O ihex basic.elf basic.hex \
    && avrdude -c usbtiny -p m1284 -V -U flash:w:basic.hex
