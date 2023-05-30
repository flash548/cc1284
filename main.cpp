/*
 * cc.cpp
 *
 * Created: 12/14/2018 3:03:59 PM
 * Author : z
 */ 
#include "Value.h"
#include "Interpreter.h"

#ifdef AVR_TARGET
	#include <avr/io.h>
	#include <util/delay.h>
	#include <avr/eeprom.h>
	#include <avr/pgmspace.h>
	#include <avr/wdt.h>
	#include "Serial.h"
#endif
#ifdef PC_TARGET
  #include <stdio.h>
  #include <stdlib.h>
#endif
#ifdef LCD_SUPPORT
	#include "LCD.h"
#endif

#ifdef AVR_TARGET
char buffer[EEPROM_PGM_SIZE] EEMEM;  // pointer to the program in EEPROM
int waitPeriod = 0; // wait period counter for bootloader wait time
int eePtr = 0;  // pointer to next write location in EEPROM
bool charRx = false; // if a char was received during bootloader wait time
char rxChar = '\0';  // the received char at the UART


int main(void)
{	
	wdt_disable();
	UCSR0B = 0x18;  // enable TX and RX UART chans
	UBRR0 = 0x07;  // baud rate - 115200 (with CPU Freq at 14.7456 MHz)
	DDRC = 0xFF;   // set PORTC to be output for debug feedback until we get to the interpreter
	PORTC= 0x00;
	send_string("1284 BASIC\n");
	while (waitPeriod++ < 20)   // wait time for bootloader until launching interpreter (20 * 100mS) = 2sec
	{
		PORTC = 0x10;
		rxChar = get_byte();  // get byte blocks for up to 100mS or until byte rx'd
		if (rxChar != '\0') {   // if received something either go into program load, or branch to REPL
			charRx = true; 
			PORTC = 0x20;
			send_string("received:");
			send_byte(rxChar);
			send_byte('\n');
			if (rxChar == ':') {  // if a ':' was first char, then branch to REPL
				Interpreter i;
				#ifdef LCD_SUPPORT
					InitLCD(false);
				#endif
				char cmdBuf[MAXSTRLENGTH];
				while (1) {  // enter into REPL loop forever
					send_string(">> ");
					get_string(cmdBuf, MAXSTRLENGTH);
					i.execute_statement(cmdBuf);					
				}
				
			} 
			break; 
		}
	}
	
	// this IF construct checks if during bootloader window, if a byte was rx'd
	//   if it was, then kick off the process of receiving a program and storing it to EEPROM
	if (charRx) {  // if we got data...
		while (rxChar != '\0') {  // do receive-and-store until we get a NULL byte
			while (!eeprom_is_ready()) { PORTC = 0x40; };
			eeprom_write_byte((uint8_t*)&buffer[eePtr], rxChar);  // write byte to EEPROM
			eePtr++;
			send_byte(0x0A);  // send a newline for ACK
			rxChar = get_byte(); // receive the new byte
		}
		eeprom_write_byte((uint8_t*)&buffer[eePtr], '\0');  // write the end-of-program (NUL) byte to EEPROM
	}	

	PORTC=0x00;
	
	Interpreter i(buffer);
	i.run();
	while (1) { 
		// if we get here, something bad happened in the interpreter, toggle LEDs for notification
		_delay_ms(500);
		PORTC=~0xAA;
		_delay_ms(500);
		PORTC=~0x55;
	 }
	return 0;
}
#endif

#ifdef PC_TARGET

void errorFunc(char* error) {
    printf("%s\n", error);
    exit(1);
}

int main(int argc, const char * argv[]) {
    
    Interpreter i("n=&H00\nwhile(true)\nprins(&H00 + n)\nn=n+1\ndelay(100)\nwend", &errorFunc);
    i.run();
    return 0;
}


#endif
