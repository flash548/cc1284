/*
 * cc.cpp
 *
 * Created: 12/14/2018 3:03:59 PM
 * Author : z
 */ 
using namespace std;
#include "Value.h"
#include "Interpreter.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#ifdef LCD_SUPPORT
	#include "LCD.h"
#endif
#include "Serial.h"

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
char buffer[EEPROM_PGM_SIZE] EEMEM;  // pointer to the program in EEPROM
int waitPeriod = 0; // wait period counter for bootloader wait time
int eePtr = 0;  // pointer to next write location in EEPROM
bool charRx = false; // if a char was received during bootloader wait time
char rxChar = '\0';  // the received char at the UART

void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();

	return;
}

int main(void)
{	
	wdt_disable();
	
	TCCR1A=(1<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
	TCCR1B=(0<<ICNC1) | (0<<ICES1) | (1<<WGM13) | (1<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x02;
	ICR1L=0xCF;
	OCR1AH=0x00;
	OCR1AL=0x16;
	OCR1BH=0x00;
	OCR1BL=0x16;
	
	UCSR0B = 0x18;  // enable TX and RX UART chans
	UBRR0H = 0x01;
	UBRR0L = 0x7F;  // baud rate - 2400 (with CPU Freq at 14.7456 MHz)	
	
	UCSR1B = 0x18;  // enable TX and RX UART chans
	UBRR1H = 0x01;
	UBRR1L = 0x7F;  // baud rate - 2400 (with CPU Freq at 14.7456 MHz)
	
	DDRD = 0xFF;
	PORTD = 0xFF;	
	while(get_byte() != '\0');  // consume anything on the serial port UART 
	
	DDRC = 0xFF;   // set PORTC to be output for debug feedback until we get to the interpreter
	PORTC= 0x00;
	
	while (waitPeriod++ < 20)   // wait time for bootloader until launching interpreter (20 * 100mS) = 2sec
	{
		rxChar = get_byte();  // get a byte, this blocks for up to 100mS or until byte rx'd
		if (rxChar != '\0') {   // if received something either go into program load, or branch to REPL
			charRx = true; 
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
		int nullTimer = 0;
		while (rxChar != '\0') {  // do receive-and-store until we get a NULL byte
			while (!eeprom_is_ready()) { PORTC = 0x40; };
			eeprom_write_byte((uint8_t*)&buffer[eePtr], rxChar);  // write byte to EEPROM
			eePtr++;
			send_byte(0x0A);  // send a newline for ACK			
			nullTimer = 0;
			do {
				rxChar = get_byte(); // receive the new byte
				nullTimer++;
			} while (rxChar == '\0' && nullTimer < 50);
			
		}
		eeprom_write_byte((uint8_t*)&buffer[eePtr], '\0');  // write the end-of-program (NUL) byte to EEPROM
	}	
	
	Interpreter i(buffer);
	i.run();
	while (1) { 
		// if we get here, something bad happened in the interpreter, toggle LEDs for notification
		_delay_ms(500);
		PORTC=0x80;
		_delay_ms(500);
		PORTC=0x60;
	 }
	return 0;
}

