/*
 * Serial.h
 *
 * Created: 12/30/2018 7:54:15 PM
 *  Author: z
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

void send_string(char* buf);
void get_string(char* buf, uint16_t max_size);
char get_byte();
void send_byte(char c);


#endif /* SERIAL_H_ */