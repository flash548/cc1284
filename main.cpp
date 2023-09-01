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
#include <string.h>
#ifdef LCD_SUPPORT
#include "LCD.h"
#endif

void do_repl();

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
#ifdef LCD_SUPPORT
        InitLCD(false);
#endif
        do_repl();
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
    // if we get here, something bad happened in the interpreter, or we exited
    // the program - in which case - toggle LEDs
    // for notification
    _delay_ms(500);
    PORTC = ~0xAA;
    _delay_ms(500);
    PORTC = ~0x55;
  }

  return 0;
}
#endif

void do_repl() {

  char tempPrg[1000];
  tempPrg[0] = '\0';
  char cmdBuf[MAXREPLLINE];
  Interpreter i;
  while (1) { // enter into REPL loop forever
#ifdef AVR_TARGET
    send_string(">> ");
    get_string(cmdBuf, MAXREPLLINE + 2, true);
#else
    printf(">> ");
    fgets(cmdBuf, sizeof(cmdBuf), stdin);
    cmdBuf[strcspn(cmdBuf, "\n")] = 0;
#endif
    if (strcmp(cmdBuf, "NEW") == 0) {
      // erase temp buffer, and start new program buffer
      tempPrg[0] = '\0';
      while (1) {
        // eat new lines into the buffer - until "DONE"
#ifdef AVR_TARGET
        get_string(cmdBuf, MAXREPLLINE + 2, true);
        send_string("\r\n");
#else
        fgets(cmdBuf, sizeof(cmdBuf), stdin);
        cmdBuf[strcspn(cmdBuf, "\n")] = 0;
#endif
        if (strcmp(cmdBuf, "DONE") == 0) {
          break;
        } else {
          if (tempPrg[0] == '\0') {
            // its an empty buffer, start from beginning
            strcpy(tempPrg, cmdBuf);
            strcat(tempPrg, "\n");
          } else {
#ifdef AVR_TARGET
            strcat(tempPrg, cmdBuf);
            strcat(tempPrg, "\n");
#else
            strcat(tempPrg, cmdBuf);
#endif
          }
        }
      }
      // dump temp buffer program
    } else if (strcmp(cmdBuf, "LIST") == 0) {
#ifdef AVR_TARGET
      send_string(tempPrg);
      send_string("\r\n");
#else
      printf("%s\n", tempPrg);
#endif
    } else if (strcmp(cmdBuf, "RUN") == 0) {
      // run the temp buffer
      i.execute_code(tempPrg);
    } else if (strcmp(cmdBuf, "SAVE") == 0) {
#ifdef AVR_TARGET
      while (!eeprom_is_ready()) {
        PORTC = 0x40;
      };
      int i = 0;
      for (i=0; i < strlen(tempPrg); i++) {
        eeprom_write_byte((uint8_t *)&buffer[i],
                          tempPrg[i]); // write byte to EEPROM
      }

      eeprom_write_byte((uint8_t *)&buffer[i], '\0'); 
#else
      FILE *fp = fopen("test.base", "w");
      fwrite(tempPrg, sizeof(char), strlen(tempPrg), fp); // write byte to EEPROM
      fclose(fp);
#endif
    }
#ifdef AVR_TARGET
    else if (strcmp(cmdBuf, "DUMP") == 0) {
      int idx = 0;
      char c = (char)eeprom_read_byte((uint8_t *)&buffer[idx]);
      while (c != '\0' && idx < EEPROM_PGM_SIZE) {
        if (c == '\n') send_byte('\r');
        send_byte(c);
        idx++;
        c = (char)eeprom_read_byte((uint8_t *)&buffer[idx]);
      }
    } 
#endif 
#ifdef AVR_TARGET
    else {
      send_string("\r\n");
      i.execute_code(cmdBuf);
    }
    send_string("\r\n");
#else
    else {
      i.execute_code(cmdBuf);
      printf("\r\n");
    }
#endif
  }
}

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
    char contents[sz + 1];
    fread(contents, sz, 1, fp);
    fclose(fp);
    contents[sz] = '\0';
    Interpreter interpreter(contents);
    interpreter.run();
  } else {
    do_repl();
  }
}

#endif
