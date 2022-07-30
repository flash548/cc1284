## BASIC for AVR mega1284

A Work-In-Progress... probably does not work at the moment.

`avr-gcc -g -Os -mmcu=atmega1284 -c *.cpp`
`avr-gcc -g -mmcu=atmega1284 -o basic.elf *.o`
`avr-objcopy -j .text -j .data -O ihex basic.elf basic.hex`

`avrdude -c usbtiny -p m1284 -U flash:w:basic.hex`

