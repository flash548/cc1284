/*
* Lexer.cpp
*
* Created: 12/14/2018 8:49:13 PM
* Author: z
*/


#include "Interpreter.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef TARGET_MICRO
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#else
#include <unistd.h>
#endif 


#ifdef LCD_SUPPORT
#include "LCD.h"
#endif

#include "Serial.h"

// default constructor
Interpreter::Interpreter()
{
	repl_mode = true;
	var_ptr = 0;	
}
Interpreter::Interpreter(char* txt)
{
	repl_mode = false;
	text = txt;
	pos = 0;
	pgm_length = strlen_ee(text);
	current_char = get_next_pgm_byte(pos);
	current_token = get_next_token();
	var_ptr = 0;
} 

// default destructor
Interpreter::~Interpreter()
{
}

// gets next pgm byte from either EEPROM or RAM
char Interpreter::get_next_pgm_byte(int idx)
{
	if (!repl_mode) {
    #ifdef TARGET_MICRO
      return (char)eeprom_read_byte((uint8_t*)&text[idx]);  
    #else
      return text[idx];
    #endif
  }
	else return text[idx];
}

void Interpreter::error(char* err)
{
#if defined DEBUG_ON_LCD && defined TARGET_MICRO
	SetLCD_XY(0,0);
	ClearLCD();
	lcd_printf("ERR: ");
	SetLCD_XY(1, 0);
	if (strlen(err) == 0) lcd_printf(current_token.value.ToString());
	else lcd_printf(err);
#endif
#if defined DEBUG_ON_SERIAL && defined TARGET_MICRO
	send_string("ERR: ");
	if (strlen(err) == 0) send_string(current_token.value.ToString());
	else send_string(err);
	send_string("\n");
#endif
#ifdef TARGET_MICRO
	if (!repl_mode) { wdt_enable(WDTO_15MS); while(1); }
#else
  printf("error: %s\n", current_token.value.ToString());
#endif
}

void Interpreter::advance()
{
	pos += 1;
	if (pos > pgm_length - 1)
	{
		current_char = '\0';
	}
	else
	{
		current_char = get_next_pgm_byte(pos);
	}
}

char Interpreter::peek()
{
	if (pos + 1 < pgm_length)
	{
		return get_next_pgm_byte(pos+1);
	}
	else
	{
		return '\0';
	}
}

void Interpreter::skip_whitespace()
{
	while (current_char != '\0' && isspace(current_char))
	{
		advance();
	}
}

Token Interpreter::parse_number()
{
	char result[20];
	int i = 0;
	while (current_char != '\0' && isdigit(current_char))
	{
		result[i++] = current_char;
		advance();
	}
	if (current_char == '.')
	{
		result[i++] = current_char;
		advance();
		while (current_char != '\0' && isdigit(current_char))
		{
			result[i++] = current_char;
			advance();
		}
		
		result[i++] = '\0';
		Token t;
		t.type = FLOATNUM;
		t.value = Value(atof(result));
		return t;
	}
	
	result[i++] = '\0';
	Token t;
	t.type = INT;
	t.value = Value(atoi(result));

	return t;
}

Token Interpreter::parse_hex()
{
	char result[20];
	int i = 0;
	while (current_char != '\0' && (isdigit(current_char) ||
	current_char == 'A' || current_char == 'B' ||
	current_char == 'C' || current_char == 'D' ||
	current_char == 'E' || current_char  == 'F'))
	{
		result[i++] = current_char;
		advance();
	}
	result[i++] = '\0';
	Token t;
	t.type = INT;
	t.value = Value((int)strtol(result, NULL, 16));
	return t;

}

Token Interpreter::parse_string()
{
	char result[MAXSTRLENGTH];
	int i = 0;
	while (current_char != '\0')
	{
		if (current_char == '"' && peek() != '"') break;  // end of string
		else if (current_char == '"' && peek() == '"') advance();  // deal with double-quote literals
		else if (current_char == '\\' && peek() == 'r') { advance(); current_char = '\r'; } // deal with carriage return char
		else if (current_char == '\\' && peek() == 'n') { advance(); current_char = '\n'; } // deal with newline char
		else if (current_char == '\\' && peek() == 't') { advance(); current_char = '\t'; } // deal with tab char
		result[i++] = current_char;
		advance();
	}

	advance();
	result[i++] = '\0';
	Token t;
	t.type = STR;
	t.value = Value(result);
	return t;
}

int nocase_cmp(char* str, const char* str2)
{
	int i = 0;
	while(str2[i] != '\0') {
		if (toupper(str[i]) != toupper(str2[i])) return 1;
		i++;
	}
	if (str[i] != '\0') return 1;
	
	return 0;
}

Token Interpreter::_id()
{
	char name[MAXSTRLENGTH];
	int i = 0;
	Token t;
	while (current_char != '\0' && isalnum(current_char))
	{
		if (isalpha(current_char)) current_char = toupper(current_char);
		name[i++] = current_char;
		advance();
	}
	
	// detect labels and return
	if (current_char == ':') {
		advance();
		name[i++] = '\0';
		t.type = LABEL;
		t.value = Value(name);
		return t;
	}

	name[i++] = '\0';

	if (nocase_cmp(name, "PRINT") == 0) { t.type = FUNC_CALL; t.value = Value(FUNC_CALL_PRINT); }
	else if (nocase_cmp(name, "LEN") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_LEN); }
	else if (nocase_cmp(name, "UBOUND") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_UBOUND); }
	else if (nocase_cmp(name, "SUBSTR") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SUBSTR); }
	else if (nocase_cmp(name, "STR") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_STR); }
	else if (nocase_cmp(name, "INT") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_INT); }
	else if (nocase_cmp(name, "BOOL") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_BOOL); }
	else if (nocase_cmp(name, "DBL") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_DOUBLE); }
	else if (nocase_cmp(name, "DELAY") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_DELAY); }
	else if (nocase_cmp(name, "DDRA") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_DDRA); }
	else if (nocase_cmp(name, "PORTA") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PORTA); }
	else if (nocase_cmp(name, "PINA") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PINA); }
	else if (nocase_cmp(name, "DDRB") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_DDRB); }
	else if (nocase_cmp(name, "DDRC") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_DDRC); }
	else if (nocase_cmp(name, "DDRD") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_DDRD); }
	else if (nocase_cmp(name, "PORTB") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PORTB); }
	else if (nocase_cmp(name, "PORTC") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PORTC); }
	else if (nocase_cmp(name, "PORTD") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PORTD); }
	else if (nocase_cmp(name, "PINB") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PINB); }
	else if (nocase_cmp(name, "PINC") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PINC); }
	else if (nocase_cmp(name, "PIND") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_PIND); }
	else if (nocase_cmp(name, "ADCSETUP") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_ADCSETUP); }
	else if (nocase_cmp(name, "AREAD") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_AREAD); }
	else if (nocase_cmp(name, "AREADRAW") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_AREADRAW); }
	else if (nocase_cmp(name, "SETPWM0") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SETPWM0); }
	else if (nocase_cmp(name, "SETPWM1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SETPWM1); }
#ifdef LCD_SUPPORT
	else if (nocase_cmp(name, "INITLCD") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_INITLCD); }
	else if (nocase_cmp(name, "SETXY") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SETXY); }
	else if (nocase_cmp(name, "CLEARLCD") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_CLEARLCD); }
#endif
	// UART0 builtins
	else if (nocase_cmp(name, "INITUART") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_INITUART); }
	else if (nocase_cmp(name, "SETBAUD") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SETBAUD); }
	else if (nocase_cmp(name, "READLINE") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_READLINE); }
	else if (nocase_cmp(name, "SENDSERIAL") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SENDSERIAL); }
	else if (nocase_cmp(name, "RXSERIAL") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_RXSERIAL); }
	else if (nocase_cmp(name, "RXBYTE") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_RXBYTE); }
	else if (nocase_cmp(name, "TXBYTE") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_TXBYTE); }
	else if (nocase_cmp(name, "SENDBYTES") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SENDBYTES); }
	else if (nocase_cmp(name, "GETBYTES") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_GETBYTES); }
	else if (nocase_cmp(name, "INDEXRXBYTE") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_INDEXRXBYTE); }
	else if (nocase_cmp(name, "RESETRXBUF") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_RESETRXBUF); }
	// UART1 builtins
	else if (nocase_cmp(name, "INITUART1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_INITUART1); }
	else if (nocase_cmp(name, "SETBAUD1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SETBAUD1); }
	else if (nocase_cmp(name, "READLINE1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_READLINE1); }
	else if (nocase_cmp(name, "SENDSERIAL1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SENDSERIAL1); }
	else if (nocase_cmp(name, "RXSERIAL1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_RXSERIAL1); }
	else if (nocase_cmp(name, "RXBYTE1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_RXBYTE1); }
	else if (nocase_cmp(name, "TXBYTE1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_TXBYTE1); }
	else if (nocase_cmp(name, "SENDBYTES1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_SENDBYTES1); }
	else if (nocase_cmp(name, "GETBYTES1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_GETBYTES1); }
	else if (nocase_cmp(name, "INDEXRXBYTE1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_INDEXRXBYTE1); }
	else if (nocase_cmp(name, "RESETRXBUF1") == 0) { t.type = FUNC_CALL;	t.value = Value(FUNC_CALL_RESETRXBUF1); }
	// end builtin-funcs...
	else if (nocase_cmp(name, "TRUE") == 0) { t.type = BOOL; t.value = Value(true); }
	else if (nocase_cmp(name, "FALSE") == 0) { t.type = BOOL; t.value = Value(false); }
	else if (nocase_cmp(name, "DIM") == 0) { t.type = DIM; t.value = Value("DIM"); }
	else if (nocase_cmp(name, "LET") == 0) { t.type = LET; t.value = Value("LET"); }
	else if (nocase_cmp(name, "NOT") == 0) { t.type = NOT; t.value = Value("NOT"); }
	else if (nocase_cmp(name, "AND") == 0) { t.type = AND; t.value = Value("AND"); }
	else if (nocase_cmp(name, "OR") == 0) { t.type = OR; t.value = Value("OR"); }
	else if (nocase_cmp(name, "XOR") == 0) { t.type = NOT; t.value = Value("XOR"); }
	else if (nocase_cmp(name, "IF") == 0) { t.type = IF; t.value = Value("IF"); }
	else if (nocase_cmp(name, "THEN") == 0) { t.type = THEN; t.value = Value("THEN"); }
	else if (nocase_cmp(name, "WHILE") == 0) { t.type = WHILE; t.value = Value("WHILE"); }
	else if (nocase_cmp(name, "WEND") == 0) { t.type = WEND; t.value = Value("WEND"); }
	else if (nocase_cmp(name, "RETURN") == 0) { t.type = RETURN; t.value = Value("RETURN"); }
	else if (nocase_cmp(name, "GOSUB") == 0) { t.type = GOSUB; t.value = Value("GOSUB"); }
	else if (nocase_cmp(name, "ELSE") == 0) { t.type = ELSE; t.value = Value("ELSE"); }
	else if (nocase_cmp(name, "FOR") == 0) { t.type = FOR; t.value = Value("FOR"); }
	else if (nocase_cmp(name, "TO") == 0) { t.type = TO; t.value = Value("TO"); }
	else if (nocase_cmp(name, "AS") == 0) { t.type = AS; t.value = Value("AS"); }
	else if (nocase_cmp(name, "STEP") == 0) { t.type = STEP; t.value = Value("STEP"); }
	else if (nocase_cmp(name, "NEXT") == 0) { t.type = NEXT; t.value = Value("NEXT"); }
	else if (nocase_cmp(name, "END") == 0) {t.type = END; t.value = Value("END"); }
	else
	{
		// return as a variable name token (value is the name of the var)
		t.type = ID;
		t.value = Value(name);
	}

	return t;

}

Token Interpreter::get_next_token()
{
	Token t;

	while (current_char != '\0')
	{
		if (isspace(current_char))
		{
			// check for Unix and Windows line endings
			if (current_char == '\r' || current_char == '\n')
			{
				if (current_char == '\r' && peek() == '\n')
				{
					advance();
				}

				advance();
				// Token t;
				t.type = NEWLINE;
				t.value = Value('\n');
				return t;
			}

			skip_whitespace();
			continue;
		}

		if (isdigit(current_char))
		{
			return parse_number();
		}

		if (current_char == '"')
		{
			advance();
			return parse_string();
		}

		if (isalnum(current_char))
		{
			return _id();
		}

		if (current_char == '&' && peek() == 'H')
		{
			advance();
			advance();
			return parse_hex();
		}

		if (current_char == '&')
		{
			advance();
			// Token t;
			t.type = STR_CONCAT;
			t.value = Value('&');
			return t;
		}

		if (current_char == '=')
		{
			advance();
			// Token t;
			t.type = EQ;
			t.value = Value('=');
			return t;
		}

		if (current_char == ',')
		{
			advance();
			// Token t;
			t.type = COMMA;
			t.value = Value(',');
			return t;
		}

		if (current_char == '%')
		{
			advance();
			// Token t;
			t.type = MOD;
			t.value = Value('%');
			return t;
		}

		if (current_char == ':')
		{
			advance();
			// Token t;
			t.type = COLON;
			t.value = Value(':');
			return t;
		}

		if (current_char == '<')
		{
			if (peek() == '>')
			{
				advance();
				advance();
				// Token t;
				t.type = NEQ;
				t.value = Value("<>");
				return t;
			}
			else if (peek() == '=')
			{
				advance();
				advance();
				// Token t;
				t.type = LTE;
				t.value = Value("<=");
				return t;
			}
			else if (peek() == '<')
			{
				advance();
				advance();
				// Token t;
				t.type = LSHIFT;
				t.value = Value("<<");
				return t;
			}

			advance();
			// Token t;
			t.type = LT;
			t.value = Value('<');
			return t;
		}

		if (current_char == '>')
		{
			if (peek() == '=')
			{
				advance();
				advance();
				// Token t;
				t.type = GTE;
				t.value = Value(">=");
				return t;
			}
			else if (peek() == '>')
			{
				advance();
				advance();
				// Token t;
				t.type = RSHIFT;
				t.value = Value(">>");
				return t;
			}

			advance();
			// Token t;
			t.type = GT;
			t.value = Value('<');
			return t;
		}

		if (current_char == '+')
		{
			advance();
			if (current_char == '=')
			{
				advance();
				// Token t;
				t.type = INCR;
				t.value = Value("+=");
				return t;
			}
			// Token t;
			t.type = PLUS;
			t.value = Value("+");
			return t;
		}

		if (current_char == '-')
		{
			advance();
			if (current_char == '=')
			{
				advance();
				// Token t;
				t.type = DECR;
				t.value = Value("-=");
				return t;
			}
			// Token t;
			t.type = MINUS;
			t.value = Value('-');
			return t;
		}

		if (current_char == '^')
		{
			advance();
			// Token t;
			t.type = POW;
			t.value = Value('^');
			return t;
		}

		if (current_char == '*')
		{
			advance();
			// Token t;
			t.type = MUL;
			t.value = Value("*");
			return t;
		}

		if (current_char == '/')
		{
			advance();
			// Token t;
			t.type = DIV;
			t.value = Value('/');
			return t;
			
		}

		if (current_char == '\\')
		{
			advance();
			// Token t;
			t.type = INTDIV;
			t.value = Value('\\');
			return t;
		}

		if (current_char == '(')
		{
			advance();
			// Token t;
			t.type = LPAREN;
			t.value = Value('(');
			return t;
		}

		if (current_char == ')')
		{
			advance();
			// Token t;
			t.type = RPAREN;
			t.value = Value(')');
			return t;
		}

		error("Invalid token detected");
	}

	t.type = END;
	t.value = Value('\0');
	return t;

}

void Interpreter::eat(TokenType tokType)
{
	if (current_token.type == tokType)
	{
		current_token = get_next_token();
	}
	else { error("Token Mismatch"); }
}

// just runs a single statement from REPL mode
void Interpreter::execute_statement(char* line)
{
	// set everything up here just if we were running a whole program..
	//  source will obviously be from RAM - not EEPROM
	text = line;
	pos = 0;
	pgm_length = strlen(text);
	current_char = get_next_pgm_byte(pos);
	current_token = get_next_token();
	statement();  // execute the statement
}

// program: statement_list
void Interpreter::run()
{
	statement_list();
}

// statement: function_call | if_statement | assignment_statement | empty
void Interpreter::statement_list()
{
	statement();
	while (current_token.type != END)
	{
		if (current_token.type == NEWLINE)
		{
			eat(NEWLINE);
		}
		statement();
	}
}

// statement: function_call | assignment_statement | empty
void Interpreter::statement()
{
	if (current_token.type == ID || current_token.type == DIM) assignment_statement();
	else if (current_token.type == FUNC_CALL) function_call();
	else if (current_token.type == WHILE) while_statement();
	else if (current_token.type == FOR) for_statement();
	else if (current_token.type == IF) if_statement();
	else if (current_token.type == GOSUB) gosub_statement();
	else if (current_token.type == INT) {
		Value r(expr());
		if (repl_mode) { 
      #ifdef TARGET_MICRO
      send_string(r.ToString()); 
      send_string("\r\n"); 
      #else
      printf("%s\r\n", r.ToString());
      #endif
    }
	}
}

// statement: function_call | assignment_statement | empty
void Interpreter::assignment_statement()
{
	char varname[20];
	bool newArrayOp = false;
	if (current_token.type == DIM) {
		newArrayOp = true;
		eat(DIM);
	}
	strcpy(varname, current_token.value.ToString());
	eat(ID);
	if (!newArrayOp) {
		if (current_token.type == LPAREN) {
			// we're assigning to an array element here..
			eat(LPAREN);
			int index = expr().number;
			eat(RPAREN);
			eat(EQ);
			Value parentValue = lookup_var(varname);
			Value result = expr();
			if (result.type == INTEGER) parentValue.update_array(index, result.number);
			else if (result.type == FLOAT) parentValue.update_array(index, result.floatNumber);
		}
		else {
			// regular old variable assignment...maybe. (might be a REPL statement)
			if (current_token.type == EQ) {
				eat(EQ);
				Value right(expr());
				store_var(varname, right);
			}
			else {
				// must just be a REPL statement or just some useless line, just eval the ID
				//  so rewind the program pointer to beginning of line and run expr() on it...
				pos = 0;
				current_char = get_next_pgm_byte(pos);
				current_token = get_next_token();
				Value right(expr());
				if (repl_mode) { 
          #ifdef TARGET_MICRO
          send_string(right.ToString()); 
          send_string("\r\n");
          #else
          printf("%s\r\n", right.ToString());
          #endif
        }
			}
		}
	}
	else {
		// DIM a new array of given size
		eat(LPAREN);
		int size = expr().number;
		eat(RPAREN);
		eat(AS);
		char type[10];
		strcpy(type, current_token.value.ToString());
		if (nocase_cmp(type, "INTEGER") == 0) { Value right(INTEGER, size); store_var(varname, right); }
		else {
			Value right(FLOAT, size);
			store_var(varname, right);
		}
		eat(ID);
	}
}

// gosub_statement: GOSUB label
void Interpreter::gosub_statement()
{
	eat(GOSUB);
	char subname[20];
	strcpy(subname, current_token.value.ToString());
	int tempPos = pos;
	eat(ID);
	bool foundLabel = false;
	pos = 0;  // rewind to beginning of program
	while (!foundLabel) {
		current_token = get_next_token();
		if (current_token.type == LABEL && strcmp(subname, current_token.value.ToString()) == 0) {
			// found the sub
			eat(LABEL);
			foundLabel = true;
			while (current_token.type != RETURN) {
				statement();
				if (current_token.type == NEWLINE) eat(NEWLINE);
			}
			eat(RETURN);
			
			// return to the caller..
			pos = tempPos;
			current_char = get_next_pgm_byte(pos);
			current_token = get_next_token();
		}
	}
	//eat(NEWLINE);
}

// if_statement: IF expr THEN statement
void Interpreter::if_statement()
{
	eat(IF);
	if (expr().ToBoolean()) {
		eat(THEN);
		statement();
		eat(NEWLINE);
		if (current_token.type == ELSE) {
			// if there was an else clause, eat it up...
			eat(ELSE);
			while (current_token.type != NEWLINE) current_token = get_next_token();
			eat(NEWLINE);
		}
	}
	else {
		eat(THEN);
		while (current_token.type != NEWLINE) current_token = get_next_token();
		eat(NEWLINE);
		if (current_token.type == ELSE) {
			eat(ELSE);
			statement();
			eat(NEWLINE);
		}
		//else {
			// no else was given...
		//	while (current_token.type != NEWLINE) current_token = get_next_token();
		//	eat(NEWLINE);
		//}
	}
}

// statement: whiile_statement LPAREN expr RPAREN NEWLINE statement_list() NEWLINE WEND
void Interpreter::while_statement()
{
	eat(WHILE);
	int tempPos = pos-1;
	eat(LPAREN);
	while (expr().ToBoolean()) {
		eat(RPAREN); eat(NEWLINE);
		while (current_token.type != WEND) {
			statement();
			if (current_token.type == NEWLINE) eat(NEWLINE);
		}
		// return back to the expression
		pos = tempPos;
		current_char = get_next_pgm_byte(pos);
		current_token = get_next_token();
		eat(LPAREN);
	}
	if (current_token.type == RPAREN) {
		eat(RPAREN);
		int while_count = 1;
		while (while_count > 0)
		{
			// go thru block until we hit our WEND..
			//  skip other WHILE constructs if needed
			current_token = get_next_token();
			if (current_token.type == WHILE) while_count++;
			if (current_token.type == WEND) while_count--;
		}
		eat(WEND);
	}

}

// statement: for_statement assignment_statement TO expr STEP expr NEWLINE statement_list() NEWLINE NEXT ID
void Interpreter::for_statement()
{
	int tempPos = pos+3;
	eat(FOR);
	char varname[20];
	strcpy(varname, current_token.value.ToString());
	assignment_statement();
	eat(TO);
	while (lookup_var(varname).number <= expr().number) {
		int incrVal = 1;
		if (current_token.type == STEP) {
			eat(STEP);
			Value v(expr());
			incrVal = v.number;
		}
		while (current_token.type != NEXT) {
			statement();
			if (current_token.type == NEWLINE) eat(NEWLINE);
		}
		
		store_var(varname, lookup_var(varname).number + incrVal);
		// return back to the expression
		pos = tempPos;
		current_char = get_next_pgm_byte(pos);
		do {
			current_token = get_next_token();
		}
		while (current_token.type != TO);
		eat(TO);
	}
	int for_count = 1;
	while (for_count > 0)
	{
		// go thru block until we hit our NEXT..
		//  skip other FOR constructs if needed
		current_token = get_next_token();
		if (current_token.type == FOR) for_count++;
		if (current_token.type == NEXT) for_count--;
	}
	eat(NEXT);
	if (current_token.type == ID) eat(ID);
}

// statement: function_call | assignment_statement | empty
Value Interpreter::function_call()
{
	int funcType = current_token.value.number;
	eat(FUNC_CALL);
	eat(LPAREN);

	if (funcType == FUNC_CALL_PRINT) {
		Value right(expr());
		eat(RPAREN);
#if defined LCD_SUPPORT && defined TARGET_MICRO
		lcd_printf(right.ToString());
#else
    printf("%s\r\n", right.ToString());
#endif

		if (repl_mode) { 
      #ifdef TARGET_MICRO
      send_string(right.ToString()); 
      send_string("\r\n"); 
      #else
      printf("%s\r\n", right.ToString());
      #endif
    }
		return right;
	}
	else if (funcType == FUNC_CALL_UBOUND) { Value right(expr().arraySize-1); eat(RPAREN); return right; }
	else if (funcType == FUNC_CALL_DELAY) { Value right(expr()); eat(RPAREN); delayMs(right.number);	return right; }
	else if (funcType == FUNC_CALL_SUBSTR) { 
		Value refVar(expr()); eat(COMMA); Value start(expr()); eat(COMMA); Value len(expr()); eat(RPAREN);
		char newStr[len.number+1];
		memcpy(newStr, refVar.ToString()+start.number, len.number);
		newStr[len.number] = '\0';
		return Value(newStr);
	}
	else if (funcType == FUNC_CALL_STR) { Value right((expr().ToString())); eat(RPAREN); return right; }
	else if (funcType == FUNC_CALL_INT) { Value right((expr().ToInt())); eat(RPAREN); return right; }
	else if (funcType == FUNC_CALL_BOOL) { Value right((expr().ToBoolean())); eat(RPAREN); return right; }
	else if (funcType == FUNC_CALL_DOUBLE) { Value right((expr().ToDouble())); eat(RPAREN); return right; }
	else if (funcType == FUNC_CALL_LEN) { Value right((int)strlen(expr().ToString())); eat(RPAREN); return right; }
  #ifdef TARGET_MICRO
	else if (funcType == FUNC_CALL_DDRA) { Value right(expr()); eat(RPAREN);  DDRA = right.number; return right; }
	else if (funcType == FUNC_CALL_PORTA) { Value right(expr()); eat(RPAREN); PORTA = right.number; return right; }
	else if (funcType == FUNC_CALL_PINA) { eat(RPAREN); Value v(PINA); return v; }
	else if (funcType == FUNC_CALL_DDRB) { Value right(expr()); eat(RPAREN);  DDRB = right.number; return right; }
	else if (funcType == FUNC_CALL_DDRC) { Value right(expr()); eat(RPAREN);  DDRC = right.number; return right; }
	else if (funcType == FUNC_CALL_DDRD) { Value right(expr()); eat(RPAREN);  DDRD = right.number; return right; }
	else if (funcType == FUNC_CALL_PORTB) { Value right(expr()); eat(RPAREN); PORTB = right.number; return right; }
	else if (funcType == FUNC_CALL_PORTC) { Value right(expr()); eat(RPAREN); PORTC = right.number; return right; }
	else if (funcType == FUNC_CALL_PORTD) { Value right(expr()); eat(RPAREN); PORTD = right.number; return right; }
	else if (funcType == FUNC_CALL_PINB) { eat(RPAREN); Value v(PINB); return v; }
	else if (funcType == FUNC_CALL_PINC) { eat(RPAREN); Value v(PINC); return v; }
	else if (funcType == FUNC_CALL_PIND) { eat(RPAREN); Value v(PIND); return v; }
	else if (funcType == FUNC_CALL_ADCSETUP) { 
		eat(RPAREN); 
		ADMUX = 0x40; // use AREF PIN
		ADCSRA = 0x87; // ADC on, and divide by 128 for prescalar
	}
	else if (funcType == FUNC_CALL_SETPWM0) {
		Value right(expr());
		if (right.number <= 0) { 
			right.number = 0xF;
		}
		else if (right.number >=255) {
			right.number = 0x1C;	
		}
		
		OCR1AH=0x00;
		OCR1AL=right.number;	
		eat(RPAREN);
	}
	else if (funcType == FUNC_CALL_SETPWM1) { 
		Value right(expr());
		if (right.number <= 0) {
			right.number=0xF;
		}
		else if (right.number >= 255) {
			right.number = 0x1C;
		}
			
		OCR1BH=0x00;
		OCR1BL=right.number;
		eat(RPAREN);
	}
	else if (funcType == FUNC_CALL_AREAD) { 
		Value pin(expr()); 
		eat(RPAREN);  
		ADMUX = 0x40 | pin.number;
		_delay_ms(1);
		ADCSRA|=(1<<ADSC); // Start the A/D conversion
		while ((ADCSRA & (1<<ADSC)));  // wait for completion
		int ad_data = ADCW;
		Value res((double)(ad_data/1023.0)*5.0);  // get result
		return res;
	}
	else if (funcType == FUNC_CALL_AREADRAW) {
		Value pin(expr());
		eat(RPAREN);
		ADMUX = 0x40 | pin.number;
		_delay_ms(1);
		ADCSRA|=(1<<ADSC); // Start the A/D conversion
		while ((ADCSRA & (1<<ADSC)));  // wait for completion
		Value res((int)ADCW);  // get result
		return res;
	}
  	// UART0 FUNCTIONS=====
	
	// sends a string out the UART0 and appends a \r\n sequence
	else if (funcType == FUNC_CALL_SENDSERIAL) { Value row(expr()); eat(RPAREN); send_string(row.ToString()); send_string("\r\n"); }		
	// reads a string and blocks until a \r or \n is received
	else if (funcType == FUNC_CALL_RXSERIAL) { eat(RPAREN);get_string(serialRxBuf, MAXSTRLENGTH); Value ret(serialRxBuf); return ret; }		
    // sends a single byte out UART0
	else if (funcType == FUNC_CALL_TXBYTE) { Value right(expr()); eat(RPAREN); send_byte((char)right.number); Value v(0); return v; }		
	// read one byte from the UART0 receive registor with a 100mS timeout (returns \0 on timeout)
	else if (funcType == FUNC_CALL_RXBYTE) { eat(RPAREN); Value rx(get_byte()); return rx; }		
	// send bytes out the UART0, each arg representing a byte
	else if (funcType == FUNC_CALL_SENDBYTES) {
		do {
			send_byte(expr().ToInt());
			if (current_token.type == COMMA) eat(COMMA);
		} while (current_token.type != RPAREN);
		eat(RPAREN);
		Value v(0); return v; 
	}
	// grab a specific number of bytes from the serial rx buffer
	else if (funcType == FUNC_CALL_GETBYTES) {
		int numBytes = expr().ToInt();
		eat(RPAREN); 
		grabbytes(numBytes); 
		Value v(INTEGER, numBytes);
		for (int i = 0; i < numBytes; i++) v.update_array(numBytes, indexRxByte(i));
		return v;
	}	
	// index into the rx buffer
	else if (funcType == FUNC_CALL_INDEXRXBYTE) { Value idx(expr()); eat(RPAREN); Value rx(indexRxByte(idx.ToInt())); return rx; }		
	// reset the tail of the rx buffer
	else if (funcType == FUNC_CALL_RESETRXBUF) { eat(RPAREN); reset(); Value v(0); return v; }		
	// read a string line (\n terminated) from the rx buffer
	else if (funcType == FUNC_CALL_READLINE) { 
		eat(RPAREN); 
		char* msg = readRxline(); 
		if (msg) { return Value(msg); } 
		else return Value("");
	}	
	// set the baud of UART0
	else if (funcType == FUNC_CALL_SETBAUD) { 
		int baud = expr().number;
		if (baud == 2400) { UBRR0H=0x01; UBRR0L=0x7F;}
		else if (baud == 9600) { UBRR0L=0x5F; }	 
		else { UBRR0L=0x07; }
		eat(RPAREN);
	}	
	// enable the UART rx buffer on interrupt
	else if (funcType == FUNC_CALL_INITUART) { eat(RPAREN); UCSR0B |= (1 << RXCIE0); sei(); }
		
	// UART1 FUNCTIONS====
	// sends a string out the UART1 and appends a \r\n sequence
	else if (funcType == FUNC_CALL_SENDSERIAL1) { Value row(expr()); eat(RPAREN); send_string1(row.ToString()); send_string1("\r\n"); }
	// reads a string and blocks until a \r or \n is received
	else if (funcType == FUNC_CALL_RXSERIAL1) { eat(RPAREN);get_string1(serialRxBuf1, MAXSTRLENGTH); Value ret(serialRxBuf1); return ret; }
	// sends a single byte out UART1
	else if (funcType == FUNC_CALL_TXBYTE1) { Value right(expr()); eat(RPAREN); send_byte1((char)right.number); Value v(0); return v; }
	// read one byte from the UART1 receive registor with a 100mS timeout (returns \0 on timeout)
	else if (funcType == FUNC_CALL_RXBYTE1) { eat(RPAREN); Value rx(get_byte1()); return rx; }
	// send bytes out the UART1, each arg representing a byte
	else if (funcType == FUNC_CALL_SENDBYTES1) {
		do {
			send_byte1(expr().ToInt());
			if (current_token.type == COMMA) eat(COMMA);
		} while (current_token.type != RPAREN);
		eat(RPAREN);
		Value v(0); return v;
	}
	// grab a specific number of bytes from the serial rx1 buffer
	else if (funcType == FUNC_CALL_GETBYTES1) {
		int numBytes = expr().ToInt();
		eat(RPAREN);
		grabbytes1(numBytes);
		Value v(INTEGER, numBytes);
		for (int i = 0; i < numBytes; i++) v.update_array(numBytes, indexRxByte1(i));
		return v;
	}
	// index into the rx1 buffer
	else if (funcType == FUNC_CALL_INDEXRXBYTE1) { Value idx(expr()); eat(RPAREN); Value rx(indexRxByte1(idx.ToInt())); return rx; }
	// reset the tail of the rx1 buffer
	else if (funcType == FUNC_CALL_RESETRXBUF1) { eat(RPAREN); reset1(); Value v(0); return v; }
	// read a string line (\n terminated) from the rx1 buffer
	else if (funcType == FUNC_CALL_READLINE1) {
		eat(RPAREN);
		char* msg = readRxline1();
		if (msg) { return Value(msg); }
		else return Value("");
	}
	// set the baud of UART1
	else if (funcType == FUNC_CALL_SETBAUD1) {
		int baud = expr().number;
		if (baud == 2400) { UBRR1H=0x01; UBRR1L=0x7F;}
		else if (baud == 9600) { UBRR1L=0x5F; }
		else { UBRR1L=0x07; }
		eat(RPAREN);
	}
	// enable the UART1 rx buffer on interrupt
	else if (funcType == FUNC_CALL_INITUART1) { eat(RPAREN); UCSR1B |= (1 << RXCIE1); sei(); }
		
#endif
#if defined LCD_SUPPORT && defined TARGET_MICRO
	else if (funcType == FUNC_CALL_INITLCD) { Value right(expr()); eat(RPAREN); InitLCD(right.ToBoolean()); }
	else if (funcType == FUNC_CALL_SETXY) { Value row(expr()); eat(COMMA); Value col(expr()); eat(RPAREN); SetLCD_XY(row.number, col.number); }
	else if (funcType == FUNC_CALL_CLEARLCD) { eat(RPAREN); ClearLCD(); }
#endif	


	// a default return value if we don't return before this
	Value v(0);
	return v;
}

/*
expr: term ((PLUS | MINUS | STR_CONCAT | LSHIFT | RSHIFT) term)*
*/
Value Interpreter::expr()
{
	Value result = term();

	while (current_token.type == PLUS ||
	current_token.type == MINUS ||
	current_token.type == STR_CONCAT ||
	current_token.type == LSHIFT ||
	current_token.type == RSHIFT)
	{
		Token token = current_token;
		if (token.type == PLUS)
		{
			eat(PLUS);
			result = (result + term());
		}
		else if (token.type == MINUS)
		{
			eat(MINUS);
			result = (result - term());
		}
		else if (token.type == STR_CONCAT)
		{
			eat(STR_CONCAT);
			result = (result & term());
		}
		else if (token.type == LSHIFT)
		{
			eat(LSHIFT);
			result = (result << term());
		}
		else if (token.type == RSHIFT)
		{
			eat(RSHIFT);
			result = (result >> term());
		}
	}

	return result;
}

// term : factor ((POW | MUL | DIV | INTDIV |
//              MOD | EQ | NEQ |
//              LT | LTE | GT | GTE |
//              AND | OR | XOR) factor)*
Value Interpreter::term()
{
	Value result = factor();

	while (
	current_token.type == MUL ||
	current_token.type == DIV ||
	current_token.type == AND ||
	current_token.type == OR ||
	current_token.type == XOR ||
	current_token.type == GT ||
	current_token.type == GTE ||
	current_token.type == LT ||
	current_token.type == LTE ||
	current_token.type == MOD ||
	current_token.type == NEQ ||
	current_token.type == EQ
	)
	{
		Token token = current_token;
		// if (token.type == POW)
		// {
		// eat(POW);
		// result = result.pow(factor());
		// }
		if (token.type == MUL)
		{
			eat(MUL);
			result = result * factor();
		}
		else if (token.type == DIV)
		{
			eat(DIV);
			result = result / factor();
		}
		// else if (token.type == INTDIV)
		// {
		// eat(INTDIV);
		// result = result.intDiv(factor());
		// }
		else if (token.type == MOD)
		{
			eat(MOD);
			result = result % factor();
		}
		else if (token.type == EQ)
		{
			eat(EQ);
			result = (result == factor());
		}
		else if (token.type == NEQ)
		{
			eat(NEQ);
			result = (result != factor());
		}
		else if (token.type == LT)
		{
			eat(LT);
			result = (result < factor());
		}
		else if (token.type == LTE)
		{
			eat(LTE);
			result = (result <= factor());
		}
		else if (token.type == GT)
		{
			eat(GT);
			result = (result > factor());
		}
		else if (token.type == GTE)
		{
			eat(GTE);
			result = (result >= factor());
		}
		else if (token.type == AND)
		{
			eat(AND);
			result = (result & factor());
		}
		else if (token.type == OR)
		{
			eat(OR);
			result = (result | factor());
		}
		else if (token.type == XOR)
		{
			eat(XOR);
			result = (result ^ factor());
		}
	}

	return result;
}

// factor : (PLUS | MINUS) factor | INTEGER | LPAREN expr RPAREN | (NOT) BOOLEAN | STRING | ID
Value Interpreter::factor()
{
	Token token = current_token;

	if (token.type == PLUS)
	{
		eat(PLUS);
		return +factor();
	}
	else if (token.type == MINUS)
	{
		eat(MINUS);
		return -factor();
	}
	else if (token.type == NOT)
	{
		eat(NOT);
		return !factor();

	}
	else if (token.type == BOOL)
	{
		eat(BOOL);
		return token.value;
	}
	else if (token.type == STR)
	{
		eat(STR);
		return token.value;
	}
	else if (token.type == ID)
	{
		eat(ID);
		if (current_token.type == LPAREN) {
			// we have an array element here, look it up
			eat(LPAREN);
			int index = factor().number;
			eat(RPAREN);
			Value parentVal = lookup_var(token.value.ToString());
			Value v = parentVal.index_array(index);
			return v;
		}
		else {
			Value v = lookup_var(token.value.ToString());
			return v;
		}
	}
	else if (token.type == FUNC_CALL)
	{
		return function_call();
	}
	else if (token.type == INT)
	{
		eat(INT);
		return token.value;
	}
	else if (token.type == FLOATNUM)
	{
		eat(FLOATNUM);
		return token.value;
	}
	else if (token.type == LPAREN)
	{
		eat(LPAREN);
		Value result = expr();
		eat(RPAREN);
		return result;
	}

	error("Invalid Factor");

	Value v(0);
	return v;
}

Value Interpreter::lookup_var(const char* name)
{
	int i = 0;
	for (i=0;i<NUM_VARS;i++)
	{
		if (strcmp(vars[i], name) == 0) break;
	}
	
	if (i >= NUM_VARS) {
		char b[30];
		sprintf(b, "Var %s not found", name);
		error(b);
	}
	
	return vals[i];
}

bool Interpreter::store_var(const char* name, Value v)
{
	int i = 0;
	for (i=0;i<NUM_VARS;i++)
	{
		// var name found, update the existing value
		if (strcmp(vars[i], name)== 0) {
			vals[i] = v;
			return true;
		}
	}
	
	// didn't find var name, so add it to end of array
	if (i >= NUM_VARS) {
		strcpy(vars[var_ptr], name);
		vals[var_ptr++] = v;
		return true;
	}
	
	if (var_ptr >= NUM_VARS) {
		error("Number of vars exceeded");
	}
	
	return false;
}

void Interpreter::delayMs(int number)
{
  #if defined TARGET_MICRO
	int i = 0;
	for (i=0;i<number;i++) _delay_ms(1);
  #else
  usleep(1000*number);
  #endif  

}

int Interpreter::strlen_ee(char* str)
{
  #ifdef TARGET_MICRO
	int i = 0;
	int len = 0;
	char c = (char)eeprom_read_byte((uint8_t*)&str[i++]);
	while (c != '\0') {
		len++;
		c = (char)eeprom_read_byte((uint8_t*)&str[i++]);
	}
	return len;
  #else
  return sizeof(str);
  #endif
}