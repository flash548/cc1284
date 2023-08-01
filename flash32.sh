#! sh
avr-gcc -w -g -Os -DAVR_TARGET -UPC_TARGET -mmcu=atmega32 -c *.cpp 
    && avr-gcc -g -DAVR_TARGET -UPC_TARGET -mmcu=atmega32 -o basic.elf *.o 
    && avr-objcopy -j .text -j .data -O ihex basic.elf basic.hex 
    && avrdude -c usbtiny -p m32 -V -U flash:w:basic.hex
