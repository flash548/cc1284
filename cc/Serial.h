/*
 * Serial.h
 *
 * Created: 12/30/2018 7:54:15 PM
 *  Author: z
 */ 

#ifndef SERIAL_H_
#define SERIAL_H_

// for UART0
char* readRxline();
void grabbytes(int count);
void send_string(char* buf);
char indexRxByte(int idx);
void reset();
void get_string(char* buf, uint16_t max_size);
char get_byte();
void send_byte(char c);

// for UART1
char* readRxline1();
void grabbytes1(int count);
void send_string1(char* buf);
char indexRxByte1(int idx);
void reset1();
void get_string1(char* buf, uint16_t max_size);
char get_byte1();
void send_byte1(char c);


#endif /* SERIAL_H_ */