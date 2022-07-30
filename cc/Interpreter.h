/*
* Lexer.h
*
* Created: 12/14/2018 8:49:13 PM
* Author: z
*/

#include "main.h"
#include "Value.h"

#ifndef __LEXER_H__
#define __LEXER_H__

#define FUNC_CALL_PRINT 1
#define FUNC_CALL_LEN 2
#define FUNC_CALL_UBOUND 3
#define FUNC_CALL_DELAY 6
#define FUNC_CALL_DDRA 7
#define FUNC_CALL_DDRB 8
#define FUNC_CALL_DDRC 9
#define FUNC_CALL_DDRD 10
#define FUNC_CALL_PORTA 11
#define FUNC_CALL_PORTB 12
#define FUNC_CALL_PORTC 13
#define FUNC_CALL_PORTD 14
#define FUNC_CALL_PINA 15
#define FUNC_CALL_PINB 16
#define FUNC_CALL_PINC 17
#define FUNC_CALL_PIND 18
#define FUNC_CALL_INITLCD 19
#define FUNC_CALL_SETXY 20
#define FUNC_CALL_CLEARLCD 21
#define FUNC_CALL_SENDSERIAL 22
#define FUNC_CALL_RXSERIAL 23
#define FUNC_CALL_TXBYTE 24
#define FUNC_CALL_RXBYTE 25
#define FUNC_CALL_ADCSETUP 26
#define FUNC_CALL_AREAD 27
#define FUNC_CALL_SETPWM0 28
#define FUNC_CALL_SETPWM1 29
#define FUNC_CALL_SUBSTR 30
#define FUNC_CALL_INITUART 31
#define FUNC_CALL_READLINE 32
#define FUNC_CALL_SETBAUD 33
#define FUNC_CALL_AREADRAW 34
#define FUNC_CALL_STR 35
#define FUNC_CALL_INT 36
#define FUNC_CALL_DOUBLE 37
#define FUNC_CALL_BOOL 38
#define FUNC_CALL_SENDBYTES 39
#define FUNC_CALL_GETBYTES 40
#define FUNC_CALL_INDEXRXBYTE 41
#define FUNC_CALL_RESETRXBUF 42

// UART1 functions
#define FUNC_CALL_RXSERIAL1 43
#define FUNC_CALL_TXBYTE1 44
#define FUNC_CALL_RXBYTE1 45
#define FUNC_CALL_INITUART1 46
#define FUNC_CALL_READLINE1 47
#define FUNC_CALL_SETBAUD1 48
#define FUNC_CALL_SENDBYTES1 49
#define FUNC_CALL_GETBYTES1 50
#define FUNC_CALL_INDEXRXBYTE1 51
#define FUNC_CALL_RESETRXBUF1 52
#define FUNC_CALL_SENDSERIAL1 53


class Interpreter
{
	//variables
	public:
	bool repl_mode;  // are we in REPL mode?  if so, read pgm data from RAM not EEPROM
	char vars[NUM_VARS][MAX_VARNAME_LEN];  // array to hold variable names
	Value vals[NUM_VARS]; // array to hold variable values
	char serialRxBuf[MAXSTRLENGTH];
	char serialRxBuf1[MAXSTRLENGTH];
	int var_ptr;  // pointer to next available variable location
	char* text;
	int pos;
	int pgm_length;
	char current_char;
	void error(char* err);
	void advance();
	char peek();
	void skip_whitespace();
	Token parse_number();
	Token parse_hex();
	Token parse_string();
	Token _id();
	char get_next_pgm_byte(int idx);
	Token get_next_token();
	void eat(TokenType tokType);
	void execute_statement(char* line);
	void run();
	void statement_list();
	void statement();
	void assignment_statement();
	Value function_call();
	void for_statement();
	void while_statement();
	void if_statement();
	void gosub_statement();
	Value expr();
	Value term();
	Value factor();
	Token current_token;
	Value lookup_var(const char* name);
	bool store_var(const char* name, Value v);
	void delayMs(int number);
	int strlen_ee(char* str);
	protected:
	private:

	//functions
	public:
	Interpreter();
	Interpreter(char* txt);
	~Interpreter();
	protected:
	private:
	Interpreter( const Interpreter &c );
	Interpreter& operator=( const Interpreter &c );

}; //Lexer

int nocase_cmp(char* str, const char* str2);

#endif //__LEXER_H__
