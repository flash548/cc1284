/*
 * Serial.cpp
 *
 * Created: 12/30/2018 7:54:01 PM
 *  Author: z
 */ 

#include "main.h"
#include <avr/io.h>
#include <util/delay.h>

// send null-terminated string out the UART
void send_string(char* buf)
{
	int counter = 0;
	do {
		while ( !( UCSRA & (1<<UDRE)) );
		UDR = buf[counter++];
	} while (buf[counter] != '\0');
	
}

// receive a newline terminated string (or one that reaches max_size)
// blocks until it receives one
void get_string(char* buf, uint16_t max_size)
{
	uint16_t count = 0;
	do {
		while ( !(UCSRA & (1<<RXC)) );
		buf[count] = UDR;
		if (buf[count] == '\n' || buf[count] == '\r') break;
		count++;
	} while ((count < max_size));
	
	if (count == max_size) { buf[count-1] = '\0'; } // truncate / terminate string
	else buf[count] = '\0';  // terminate the string	
}

// read single byte from UART RX register
char get_byte()
{
	int counter = 0;
	while ( !(UCSRA & (1<<RXC)) && counter++ < 100) { _delay_ms(1); }
	
	if (counter >= 100) return '\0';
	else {
		char c = UDR;
		return c;
	}
}

// send a single byte out the UART
void send_byte(char c)
{
	while ( !( UCSRA & (1<<UDRE)) );
	UDR = c;
}