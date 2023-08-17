/*
 * LCD.cpp
 *
 * Created: 12/22/2018 7:46:53 PM
 *  Author: z
 */

#ifdef LCD_SUPPORT
#ifdef AVR_TARGET

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD.h"

int RS_pin = 0;
int RW_pin = 1;
int E_pin = 2;
int data_start_pin = 4;
bool onPortA = true;

void InitLCD(bool portA)
{
	onPortA = portA;
	onPortA ? DDRA = 0xFF : DDRB = 0xFF;
	onPortA ? PORTA = 0x00 : PORTB = 0x00;
	WriteLCD(0x02, false);
	WriteLCD(0x0C, false);
	WriteLCD(0x01, false);
	WriteLCD(0x06, false);
	_delay_ms(100);
}

void lcd_printf(const char *str)
{
	int i = 0;
	while (str[i] != '\0')
	{
		WriteLCD(str[i++], true);
	}
}

// prints a string from an address in EEPROM pointed to by *str
void lcd_printf_ee(const char *str)
{
	int i = 0;
	char c = (char)eeprom_read_byte((uint8_t *)&str[i++]);
	while (c != '\0')
	{
		WriteLCD(c, true);
		c = (char)eeprom_read_byte((uint8_t *)&str[i++]);
	}
}

// clears the LCD
void ClearLCD()
{
	WriteLCD(0x1, false);
}

void SetLCD_XY(int x, int y)
{
	int i = 0;
	if (x != 0)
	{
		i = y | 0x40;
	}
	else
	{
		i = y;
	}

	i |= 0x80;
	WriteLCD(i, false);
}

void WriteLCD(char c, bool data)
{
	SetRS(data);
	SetRW(false);
	_delay_ms(2);
	SetE(true);
	_delay_ms(2);
	SetNibblePins((c >> 4) & 0xF);
	_delay_ms(2);
	SetE(false);
	_delay_ms(2);
	SetE(true);
	_delay_ms(2);
	SetNibblePins((c & 0xF));
	_delay_ms(1);
	SetE(false);
	_delay_ms(2);
}

void SetNibblePins(char data)
{
	if (onPortA)
	{
		((data & 0xF) & 0x1) ? PORTA |= (1 << 4) : PORTA &= ~(1 << 4);
		(((data & 0xF) >> 1) & 0x1) ? PORTA |= (1 << 5) : PORTA &= ~(1 << 5);
		(((data & 0xF) >> 2) & 0x1) ? PORTA |= (1 << 6) : PORTA &= ~(1 << 6);
		(((data & 0xF) >> 3) & 0x1) ? PORTA |= (1 << 7) : PORTA &= ~(1 << 7);
	}
	else
	{
		((data & 0xF) & 0x1) ? PORTB |= (1 << 4) : PORTB &= ~(1 << 4);
		(((data & 0xF) >> 1) & 0x1) ? PORTB |= (1 << 5) : PORTB &= ~(1 << 5);
		(((data & 0xF) >> 2) & 0x1) ? PORTB |= (1 << 6) : PORTB &= ~(1 << 6);
		(((data & 0xF) >> 3) & 0x1) ? PORTB |= (1 << 7) : PORTB &= ~(1 << 7);
	}
}

void SetE(bool high)
{
	if (high)
		onPortA ? PORTA |= (1 << 2) : PORTB |= (1 << 2);
	else
		onPortA ? PORTA &= ~(1 << 2) : PORTB &= ~(1 << 2);
}

void SetRW(bool high)
{
	if (high)
		onPortA ? PORTA |= (1 << 1) : PORTB |= (1 << 1);
	else
		onPortA ? PORTA &= ~(1 << 1) : PORTB &= ~(1 << 1);
}

void SetRS(bool high)
{
	if (high)
		onPortA ? PORTA |= (1 << 0) : PORTB |= (1 << 0);
	else
		onPortA ? PORTA &= ~(1 << 0) : PORTB &= ~(1 << 0);
}

#endif
#endif
