/*
 * Serial.cpp
 *
 * Created: 12/30/2018 7:54:01 PM
 *  Author: z
 */ 

#include "main.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define MAXUARTBUFSIZE 100

// for UART0
char buf[MAXUARTBUFSIZE];  // buffer that the RXINT uses (if enabled)
int head = 0; // head for RX buffer
int tail = 0; // tail for RX buffer
char* msg; // points to most recent message in the buffer (\n terminated)

// for UART1
char buf1[MAXUARTBUFSIZE];  // buffer that the RXINT uses (if enabled)
int head1 = 0; // head for RX buffer
int tail1 = 0; // tail for RX buffer
char* msg1; // points to most recent message in the buffer (\n terminated)

ISR(USART0_RX_vect)
{
	buf[head++] = UDR0;
	if (head >= MAXUARTBUFSIZE) head = 0;
}

ISR(USART1_RX_vect)
{
	buf1[head1++] = UDR1;
	if (head1 >= MAXUARTBUFSIZE) head1 = 0;
}

// send null-terminated string out the UART
void send_string(char* buf)
{
	int counter = 0;
	do {
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = buf[counter++];
	} while (buf[counter] != '\0');
	
}

// receive a newline terminated string (or one that reaches max_size)
// blocks until it receives one
void get_string(char* buf, uint16_t max_size)
{
	uint16_t count = 0;
	do {
		while ( !(UCSR0A & (1<<RXC0)) );
		buf[count] = UDR0;
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
	while ( !(UCSR0A & (1<<RXC0)) && counter++ < MAXUARTBUFSIZE) { _delay_ms(1); }
	
	if (counter >= MAXUARTBUFSIZE) return '\0';
	else {
		char c = UDR0;
		return c;
	}
}

// send a single byte out the UART
void send_byte(char c)
{
	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = c;
}



char* readRxline() {	
	int start = tail;
	while (buf[tail] != '\n' && buf[tail] != '\r') {
		tail++;
		if (tail >= MAXUARTBUFSIZE) tail = 0;
		if (tail == start) {
			// we've wrapped all the way around with no luck
			tail = start;  // return the tail back to where it was
			return NULL;
		}
	}
	
	int size = 0;
	if (tail < start) {  // see if we wrapped
		size = (MAXUARTBUFSIZE - start) + tail;
	} 
	else { size = tail-start; }
		
	tail++;
	if (tail > MAXUARTBUFSIZE) tail = 0;
	if (buf[tail] == '\n') {
		tail++;
		if (tail > MAXUARTBUFSIZE) tail = 0;
	}
	if (tail > MAXUARTBUFSIZE) tail = 0;
	if (msg) { free(msg); }
	msg = (char* )malloc(sizeof(char) * size + 1);
	int count = 0;
	int bufIdx = start;
	int msgIdx = 0;
	while (count < size) {
		msg[msgIdx++] = buf[bufIdx];
		buf[bufIdx++] = '\0';
		if (bufIdx >= MAXUARTBUFSIZE) bufIdx = 0;
		count++;
	}
	msg[msgIdx] = '\0';
	return msg;
}

void grabbytes(int count) {
	if (tail > MAXUARTBUFSIZE) tail = 0;
	if (msg) { free(msg); }
	msg = (char* )malloc(sizeof(char) * count + 1);
	int i = 0;
	for (i = 0; i < count; i++) {
		msg[i] = buf[tail++];
		if (tail > MAXUARTBUFSIZE) tail = 0;
	}
}

char indexRxByte(int idx) {
	return msg[idx];
}

void reset() { head=0; tail= 0; }
	
// for UART1....
	
// send null-terminated string out the UART
void send_string1(char* buf)
{
	int counter = 0;
	do {
		while ( !( UCSR1A & (1<<UDRE1)) );
		UDR1 = buf[counter++];
	} while (buf[counter] != '\0');
	
}

// receive a newline terminated string (or one that reaches max_size)
// blocks until it receives one
void get_string1(char* buf, uint16_t max_size)
{
	uint16_t count = 0;
	do {
		while ( !(UCSR1A & (1<<RXC1)) );
		buf[count] = UDR1;
		if (buf[count] == '\n' || buf[count] == '\r') break;
		count++;
	} while ((count < max_size));
	
	if (count == max_size) { buf[count-1] = '\0'; } // truncate / terminate string
	else buf[count] = '\0';  // terminate the string
}

// read single byte from UART RX register
char get_byte1()
{
	int counter = 0;
	while ( !(UCSR1A & (1<<RXC1)) && counter++ < MAXUARTBUFSIZE) { _delay_ms(1); }
	
	if (counter >= MAXUARTBUFSIZE) return '\0';
	else {
		char c = UDR1;
		return c;
	}
}

// send a single byte out the UART
void send_byte1(char c)
{
	while ( !( UCSR1A & (1<<UDRE1)) );
	UDR1 = c;
}



char* readRxline1() {
	int start = tail1;
	while (buf1[tail1] != '\n' && buf1[tail1] != '\r') {
		tail1++;
		if (tail1 >= MAXUARTBUFSIZE) tail1 = 0;
		if (tail1 == start) {
			// we've wrapped all the way around with no luck
			tail1 = start;  // return the tail back to where it was
			return NULL;
		}
	}
	
	int size = 0;
	if (tail1 < start) {  // see if we wrapped
		size = (MAXUARTBUFSIZE - start) + tail1;
	}
	else { size = tail1-start; }
	
	tail1++;
	if (tail1 > MAXUARTBUFSIZE) tail1 = 0;
	if (buf1[tail1] == '\n') {
		tail1++;
		if (tail1 > MAXUARTBUFSIZE) tail1 = 0;
	}
	if (tail1 > MAXUARTBUFSIZE) tail1 = 0;
	if (msg1) { free(msg1); }
	msg1 = (char* )malloc(sizeof(char) * size + 1);
	int count = 0;
	int bufIdx = start;
	int msgIdx = 0;
	while (count < size) {
		msg1[msgIdx++] = buf1[bufIdx];
		buf1[bufIdx++] = '\0';
		if (bufIdx >= MAXUARTBUFSIZE) bufIdx = 0;
		count++;
	}
	msg1[msgIdx] = '\0';
	return msg1;
}

void grabbytes1(int count) {
	if (tail1 > MAXUARTBUFSIZE) tail1 = 0;
	if (msg1) { free(msg1); }
	msg1 = (char* )malloc(sizeof(char) * count + 1);
	int i = 0;
	for (i = 0; i < count; i++) {
		msg1[i] = buf1[tail1++];
		if (tail1 > MAXUARTBUFSIZE) tail1 = 0;
	}
}

char indexRxByte1(int idx) {
	return msg1[idx];
}

void reset1() { head1=0; tail1 = 0; }
		