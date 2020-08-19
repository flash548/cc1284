avrdude -c usbtiny -p m1284p -V -U flash:w:cc1284.hex

REM -U lfuse:w:0xe0:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m
REM for EESAVE use:
REM avrdude -c usbtiny -p m1284p -U lfuse:w:0xe0:m -U hfuse:w:0xd1:m -U efuse:w:0xff:m