/*
 * cc.cpp
 *
 * Created: 12/14/2018 3:03:59 PM
 * Author : z
 */
#include "Interpreter.h"
#include "Value.h"

#ifdef AVR_TARGET
#include "Serial.h"
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <util/delay.h>
#else
#include <stdio.h>
#endif
#ifdef LCD_SUPPORT
#include "LCD.h"
#endif

#ifdef AVR_TARGET
char buffer[EEPROM_PGM_SIZE] EEMEM; // pointer to the program in EEPROM
int waitPeriod = 0;  // wait period counter for bootloader wait time
int eePtr = 0;       // pointer to next write location in EEPROM
bool charRx = false; // if a char was received during bootloader wait time
char rxChar = '\0';  // the received char at the UART

/**
 * @brief Embedded Target entrpoint
 * 
 */
int main(void) {
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
#ifdef ATMEGA1284
  UCSR0B = 0x18; // enable TX and RX UART chans
  UBRR0 = 0x07;  // baud rate - 115200 (with CPU Freq at 14.7456 MHz)
#else
  UCSRB = 0x18; // enable TX and RX UART chans
  UBRRL = 0x07; // baud rate - 115200 (with CPU Freq at 14.7456 MHz)
#endif
  DDRC = 0xFF; // set PORTC to be output for debug feedback until we get to the
               // interpreter
  PORTC = 0xFF;
#ifdef ATMEGA1284
  send_string("1284 BASIC\r\n");
#else
  send_string("MEGA32 BASIC\r\n");
#endif

  while (waitPeriod++ < 20) // wait time for bootloader until launching
                            // interpreter (20 * 100mS) = 2sec
  {
    PORTC = 0x10;
    rxChar = get_byte();  // get byte blocks for up to 100mS or until byte rx'd
    if (rxChar != '\0') { // if received something either go into program load,
                          // or branch to REPL
      charRx = true;
      PORTC = 0x20;
      if (rxChar == ':') { // if a ':' was first char, then branch to REPL
        Interpreter i;
#ifdef LCD_SUPPORT
        InitLCD(false);
#endif
        char cmdBuf[MAXREPLLINE];
        while (1) { // enter into REPL loop forever
          send_string(">> ");
          get_string(cmdBuf, MAXREPLLINE+2, true);
		  if (i.nocase_cmp(cmdBuf, "DUMP") == 0) {
			int idx=0;
			char c = (char)eeprom_read_byte((uint8_t *)&buffer[idx]);
			while (c != '\0' && idx < EEPROM_PGM_SIZE) {
				send_byte(c);
				idx++;
				c = (char)eeprom_read_byte((uint8_t *)&buffer[idx]);
			}
		  } else {
            send_string("\r\n");  
          	i.execute_statement(cmdBuf);
		  }
          send_string("\r\n");
        }
      }
      break;
    }
  }

  // this IF construct checks if during bootloader window, if a byte was rx'd
  //   if it was, then kick off the process of receiving a program and storing
  //   it to EEPROM
  if (charRx) {              // if we got data...
    while (rxChar != '\0') { // do receive-and-store until we get a NULL byte
      while (!eeprom_is_ready()) {
        PORTC = 0x40;
      };
      eeprom_write_byte((uint8_t *)&buffer[eePtr],
                        rxChar); // write byte to EEPROM
      eePtr++;
      send_byte(0x0A);     // send a newline for ACK
      rxChar = get_byte(); // receive the new byte
    }
    eeprom_write_byte((uint8_t *)&buffer[eePtr],
                      '\0'); // write the end-of-program (NUL) byte to EEPROM
  }

  PORTC = 0x00;

  Interpreter i(buffer);
  i.run();
  while (1) {
    // if we get here, something bad happened in the interpreter, or we exited the
	// program - in which case - toggle LEDs
    // for notification
    _delay_ms(500);
    PORTC = ~0xAA;
    _delay_ms(500);
    PORTC = ~0x55;
  }

  return 0;
}
#endif

#ifdef PC_TARGET
/**
 * @brief Entrypoint for running on PC
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv) {
  if (argc > 1) {
    FILE *fp = fopen(argv[1], "r");
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    rewind(fp);
    char contents[sz+1];
    fread(contents, sz, 1, fp);
    fclose(fp);
    contents[sz] = '\0';
    Interpreter interpreter(contents);
    interpreter.run();
  } else {

	// some default code to run if no file
    Interpreter interpreter("print(2+3)\ndelay(1000)");
    interpreter.run();
  }
}

#endif
