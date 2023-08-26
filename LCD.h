/*
 * LCD.h
 *
 * Created: 12/22/2018 7:47:28 PM
 *  Author: z
 */ 


#ifndef LCD_H_
#define LCD_H_

void InitLCD(bool portA);
void lcd_printf(const char* str);
void lcd_printf_ee(const char* str);
void ClearLCD();
void SetLCD_XY(int x, int y);
void WriteLCD(char c, bool data);
void SetNibblePins(char data);
void SetE(bool high);
void SetRW(bool high);
void SetRS(bool high);



#endif /* LCD_H_ */
