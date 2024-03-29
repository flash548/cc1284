/*
 * Lexer.cpp
 *
 * Created: 12/14/2018 8:49:13 PM
 * Author: z
 */

#include "Interpreter.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PC_TARGET
#include <unistd.h>
#endif
#ifdef AVR_TARGET
#include "Serial.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#endif
#ifdef LCD_SUPPORT
#include "LCD.h"
#endif
// default constructor
Interpreter::Interpreter() {
  repl_mode = true;
  var_ptr = 0;
}

// constructor with given program buffer
Interpreter::Interpreter(char *txt) {
  repl_mode = false;
  text = txt;
  pos = 0;
#ifdef AVR_TARGET
  pgm_length = strlen_ee(text);
#endif
#ifdef PC_TARGET
  pgm_length = strlen(text);
#endif
  current_char = get_next_pgm_byte(pos);
  current_token = get_next_token();
  var_ptr = 0;
}

// default destructor
Interpreter::~Interpreter() {}

// gets next pgm byte from either EEPROM or RAM
char Interpreter::get_next_pgm_byte(int idx) {
#ifdef AVR_TARGET
  if (!repl_mode)
    return (char)eeprom_read_byte((uint8_t *)&text[idx]);
  else
    return text[idx];
#endif
#ifdef PC_TARGET
  return text[idx];
#endif
}

void Interpreter::writeln(Value r) {
#ifdef AVR_TARGET
  send_string(r.ToString());
  send_string("\r\n");
#endif
#ifdef PC_TARGET
  printf("%s\r\n", r.ToString());
#endif
}

void Interpreter::error(char *err) {
  char *tempStr = current_token.value.ToString();
#ifdef DEBUG_ON_LCD
  SetLCD_XY(0, 0);
  ClearLCD();
  lcd_printf("ERR: ");
  SetLCD_XY(1, 0);
  if (strlen(err) == 0)
    lcd_printf(tempStr);
  else
    lcd_printf(err);
#endif
#ifdef DEBUG_ON_SERIAL
  send_string("ERR: ");
  if (strlen(err) == 0)
    send_string(tempStr);
  else {
    send_string(tempStr);
    send_string("\r\n");
    send_string(err);
  }
  send_string("\r\n");
#endif
  if (!repl_mode) {
#ifdef AVR_TARGET
    char errMsg[100];
    send_string("Entering error loop...\r\n");
    sprintf(errMsg, "Error :(... %s\r\n", err);
    send_string(errMsg);
    sprintf(errMsg, "Current char: %c\r\n", current_char);
    send_string(errMsg);
    sprintf(errMsg, "Current line: %i\r\n", line_number);
    send_string(errMsg);
    sprintf(errMsg, "Current token val: %s\r\n", tempStr);
    send_string(errMsg);
    sprintf(errMsg, "Current pos: %i\r\n", pos);
    send_string(errMsg);
#else
    printf("Error :(... %s\r\n", err);
    printf("Current char: %c\r\n", current_char);
    printf("Current line: %i\r\n", line_number);
    printf("Current token val: %s\r\n", tempStr);
#ifdef PC_TARGET
    exit(1);
#endif
#endif
  } else {

  }
}

void Interpreter::advance() {
  pos += 1;
  if (pos > pgm_length - 1) {
    current_char = '\0';
  } else {
    current_char = get_next_pgm_byte(pos);
  }
}

char Interpreter::peek() {
  if (pos + 1 < pgm_length) {
    return get_next_pgm_byte(pos + 1);
  } else {
    return '\0';
  }
}

void Interpreter::skip_whitespace() {
  while (current_char != '\0' && isspace(current_char)) {
    advance();
  }
}

Token Interpreter::parse_number() {
  char result[20];
  int i = 0;
  while (current_char != '\0' && isdigit(current_char)) {
    result[i++] = current_char;
    advance();
  }
  if (current_char == '.') {
    result[i++] = current_char;
    advance();
    while (current_char != '\0' && isdigit(current_char)) {
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

Token Interpreter::parse_hex() {
  char result[20];
  int i = 0;
  while (current_char != '\0' &&
         (isdigit(current_char) || current_char == 'A' || current_char == 'B' ||
          current_char == 'C' || current_char == 'D' || current_char == 'E' ||
          current_char == 'F')) {
    result[i++] = current_char;
    advance();
  }
  result[i++] = '\0';
  Token t;
  t.type = INT;
  t.value = Value((int)strtol(result, NULL, 16));
  return t;
}

Token Interpreter::parse_string() {
  char result[MAXSTRLENGTH];
  int i = 0;
  while (current_char != '\0') {
    if (current_char == '"' && peek() != '"')
      break; // end of string
    else if (current_char == '"' && peek() == '"')
      advance(); // deal with double-quote literals
    else if (current_char == '\\' && peek() == 'n') {
      advance();
      current_char = '\n';
    } // deal with newline char
    else if (current_char == '\\' && peek() == 't') {
      advance();
      current_char = '\t';
    } // deal with tab char
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

/**
 Case insensitive comparison for strings
 */
int Interpreter::nocase_cmp(char *str, char *str2) {
  int i = 0;
  while (str2[i] != '\0' || str[i] != '\0') {
    if (toupper(str[i]) != toupper(str2[i]))
      return 1;
    i++;
  }

  return 0;
}

Token Interpreter::_id() {
  char name[MAXSTRLENGTH];
  int i = 0;
  Token t;
  while (current_char != '\0' && isalnum(current_char)) {
    if (isalpha(current_char))
      current_char = toupper(current_char);
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

  if (nocase_cmp(name, "PRINT") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PRINT);
  } else if (nocase_cmp(name, "ASSERT") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_ASSERT);
  } else if (nocase_cmp(name, "LEN") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_LEN);
  } else if (nocase_cmp(name, "UBOUND") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_UBOUND);
  } else if (nocase_cmp(name, "FREERAM") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_FREERAM);
  } else if (nocase_cmp(name, "REBOOT") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_REBOOT);
  } else if (nocase_cmp(name, "DELAY") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_DELAY);
  
  } else if (nocase_cmp(name, "SUBSTR") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_SUBSTR);
  }
#if defined ATMEGA32 || defined ATMEGA1284
  else if (nocase_cmp(name, "DDRA") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_DDRA);
  } else if (nocase_cmp(name, "PORTA") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PORTA);
  } else if (nocase_cmp(name, "PINA") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PINA);
  }
#endif
  else if (nocase_cmp(name, "DDRB") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_DDRB);
  } else if (nocase_cmp(name, "DDRC") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_DDRC);
  } else if (nocase_cmp(name, "DDRD") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_DDRD);
  } else if (nocase_cmp(name, "PORTB") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PORTB);
  } else if (nocase_cmp(name, "PORTC") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PORTC);
  } else if (nocase_cmp(name, "PORTD") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PORTD);
  } else if (nocase_cmp(name, "PINB") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PINB);
  } else if (nocase_cmp(name, "PINC") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PINC);
  } else if (nocase_cmp(name, "PIND") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_PIND);
  } else if (nocase_cmp(name, "ADCSETUP") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_ADCSETUP);
  } else if (nocase_cmp(name, "AREAD") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_AREAD);
  } else if (nocase_cmp(name, "SETPWM0") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_SETPWM0);
  } else if (nocase_cmp(name, "SETPWM1") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_SETPWM1);
  }
#ifdef LCD_SUPPORT
  else if (nocase_cmp(name, "INITLCD") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_INITLCD);
  } else if (nocase_cmp(name, "SETXY") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_SETXY);
  } else if (nocase_cmp(name, "CLEARLCD") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_CLEARLCD);
  } else if (nocase_cmp(name, "LCDPRINT") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_LCDPRINT);
  }
#endif
  else if (nocase_cmp(name, "SENDSERIAL") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_SENDSERIAL);
  } else if (nocase_cmp(name, "RXSERIAL") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_RXSERIAL);
  } else if (nocase_cmp(name, "RXBYTE") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_RXBYTE);
  } else if (nocase_cmp(name, "TXBYTE") == 0) {
    t.type = FUNC_CALL;
    t.value = Value(FUNC_CALL_TXBYTE);
  }
  // end builtin-funcs...
  else if (nocase_cmp(name, "TRUE") == 0) {
    t.type = BOOL;
    t.value = Value(true);
  } else if (nocase_cmp(name, "FALSE") == 0) {
    t.type = BOOL;
    t.value = Value(false);
  } else if (nocase_cmp(name, "DIM") == 0) {
    t.type = DIM;
    t.value = Value("DIM");
  } else if (nocase_cmp(name, "LET") == 0) {
    t.type = LET;
    t.value = Value("LET");
  } else if (nocase_cmp(name, "NOT") == 0) {
    t.type = NOT;
    t.value = Value("NOT");
  } else if (nocase_cmp(name, "AND") == 0) {
    t.type = AND;
    t.value = Value("AND");
  } else if (nocase_cmp(name, "OR") == 0) {
    t.type = OR;
    t.value = Value("OR");
  } else if (nocase_cmp(name, "XOR") == 0) {
    t.type = XOR;
    t.value = Value("XOR");
  } else if (nocase_cmp(name, "IF") == 0) {
    t.type = IF;
    t.value = Value("IF");
  } else if (nocase_cmp(name, "THEN") == 0) {
    t.type = THEN;
    t.value = Value("THEN");
  } else if (nocase_cmp(name, "WHILE") == 0) {
    t.type = WHILE;
    t.value = Value("WHILE");
  } else if (nocase_cmp(name, "WEND") == 0) {
    t.type = WEND;
    t.value = Value("WEND");
  } else if (nocase_cmp(name, "RETURN") == 0) {
    t.type = RETURN;
    t.value = Value("RETURN");
  } else if (nocase_cmp(name, "GOSUB") == 0) {
    t.type = GOSUB;
    t.value = Value("GOSUB");
  } else if (nocase_cmp(name, "ELSE") == 0) {
    t.type = ELSE;
    t.value = Value("ELSE");
  } else if (nocase_cmp(name, "FOR") == 0) {
    t.type = FOR;
    t.value = Value("FOR");
  } else if (nocase_cmp(name, "TO") == 0) {
    t.type = TO;
    t.value = Value("TO");
  } else if (nocase_cmp(name, "AS") == 0) {
    t.type = AS;
    t.value = Value("AS");
  } else if (nocase_cmp(name, "STEP") == 0) {
    t.type = STEP;
    t.value = Value("STEP");
  } else if (nocase_cmp(name, "NEXT") == 0) {
    t.type = NEXT;
    t.value = Value("NEXT");
  } else if (nocase_cmp(name, "END") == 0) {
    t.type = END;
    t.value = Value("END");
  } else {
    // return as a variable name token (value is the name of the var)
    t.type = ID;
    t.value = Value(name);
  }

  return t;
}

Token Interpreter::get_next_token() {
  Token t;

  while (current_char != '\0') {
    if (isspace(current_char)) {
      // check for Unix and Windows line endings
      if (current_char == '\r' || current_char == '\n') {
        if (current_char == '\r' && peek() == '\n') {
          advance();
        }

        advance();
        line_number++;
        t.type = NEWLINE;
        t.value = Value('\n');
        return t;
      }

      skip_whitespace();
      continue;
    }

    if (isdigit(current_char)) {
      return parse_number();
    }

    // start of comment
    // just eat the comment here til end of line or end of file
    if (current_char == '\'') {
      advance();
      while (current_char != '\n') {
        advance();
      }
      continue;
    }

    if (current_char == '"') {
      advance();
      return parse_string();
    }

    if (isalnum(current_char)) {
      return _id();
    }

    if (current_char == '&' && peek() == 'H') {
      advance();
      advance();
      return parse_hex();
    }

    if (current_char == '&') {
      advance();
      // Token t;
      t.type = STR_CONCAT;
      t.value = Value('&');
      return t;
    }

    if (current_char == '=') {
      advance();
      // Token t;
      t.type = EQ;
      t.value = Value('=');
      return t;
    }

    if (current_char == ',') {
      advance();
      // Token t;
      t.type = COMMA;
      t.value = Value(',');
      return t;
    }

    if (current_char == '%') {
      advance();
      // Token t;
      t.type = MOD;
      t.value = Value('%');
      return t;
    }

    if (current_char == ':') {
      advance();
      // Token t;
      t.type = NEWLINE;
      t.value = Value(':');
      return t;
    }

    if (current_char == '<') {
      if (peek() == '>') {
        advance();
        advance();
        // Token t;
        t.type = NEQ;
        t.value = Value("<>");
        return t;
      } else if (peek() == '=') {
        advance();
        advance();
        // Token t;
        t.type = LTE;
        t.value = Value("<=");
        return t;
      } else if (peek() == '<') {
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

    if (current_char == '>') {
      if (peek() == '=') {
        advance();
        advance();
        // Token t;
        t.type = GTE;
        t.value = Value(">=");
        return t;
      } else if (peek() == '>') {
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

    if (current_char == '+') {
      advance();
      if (current_char == '=') {
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

    if (current_char == '-') {
      advance();
      if (current_char == '=') {
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

    if (current_char == '^') {
      advance();
      // Token t;
      t.type = POW;
      t.value = Value('^');
      return t;
    }

    if (current_char == '*') {
      advance();
      // Token t;
      t.type = MUL;
      t.value = Value("*");
      return t;
    }

    if (current_char == '/') {
      advance();
      // Token t;
      t.type = DIV;
      t.value = Value('/');
      return t;
    }
    if (current_char == '\\') {
      advance();
      t.type = INTDIV;
      t.value = Value('\\');
      return t;
    }

    if (current_char == '(') {
      advance();
      // Token t;
      t.type = LPAREN;
      t.value = Value('(');
      return t;
    }

    if (current_char == ')') {
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

void Interpreter::eat(TokenType tokType) {
  if (current_token.type == tokType) {
    current_token = get_next_token();

  } else {
#ifdef AVR_TARGET
    char str_current[10];
    char str_expected[10];
    char err_msg[100];
    sprintf(err_msg, "%i, %i\n", current_token.type, tokType);
    writeln(err_msg);
    for (unsigned char i = 0; i < 5; i++) {
      strcpy_P(str_current,
               (PGM_P)pgm_read_word(&(token_strings[current_token.type + i])));
      strcpy_P(str_expected,
               (PGM_P)pgm_read_word(&(token_strings[tokType + i])));
    }
    sprintf(err_msg, "Current: %s, Expected: %s\n", str_current, str_expected);
    writeln(err_msg);
#endif
#ifdef PC_TARGET
    const char *str_current = token_strings[current_token.type];
    const char *str_expected = token_strings[tokType];
    printf("Current Token: %s, Expected: %s\n", str_current, str_expected);
#endif
    error("Token Mismatch");
  }
}

// just runs a single statement from REPL mode
void Interpreter::execute_code(char *line) {
  // set everything up here just if we were running a whole program..
  //  source will obviously be from RAM - not EEPROM
  text = line;
  pos = 0;
  pgm_length = strlen(text);
  current_char = get_next_pgm_byte(pos);
  current_token = get_next_token();
  statement_list(); // execute the statement
}

// program: statement_list
void Interpreter::run() { statement_list(); }

// statement: function_call | if_statement | assignment_statement | empty
void Interpreter::statement_list() {
  statement();
  if (current_token.type == NEWLINE) {
    eat(NEWLINE);
  }

tryagain:
  while (current_token.type != END) {
    if (current_token.type == NEWLINE) {
      eat(NEWLINE);
      goto tryagain;
    }

    statement();
  }
}

// statement: function_call | assignment_statement | empty
void Interpreter::statement() {
  if (current_token.type == NEWLINE) {
    eat(NEWLINE);
    return; 
  }

  if (current_token.type == ID || current_token.type == DIM)
    assignment_statement();
  else if (current_token.type == FUNC_CALL)
    function_call();
  else if (current_token.type == WHILE)
    while_statement();
  else if (current_token.type == FOR)
    for_statement();
  else if (current_token.type == IF)
    if_statement();
  else if (current_token.type == GOSUB)
    gosub_statement();
  else {
    evaluate_and_print();
  }
}

void Interpreter::evaluate_and_print() {
  Value r(expr());
  if (repl_mode) {
    writeln(r.ToString());
  }
}

// statement: function_call | assignment_statement | empty
void Interpreter::assignment_statement() {
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
      if (current_token.type == EQ) {
        eat(EQ);
        Value parentValue = lookup_var(varname);
        Value result = expr();
        if (result.type == INTEGER)
          parentValue.update_array(index, result.number);
        else if (result.type == FLOAT)
          parentValue.update_array(index, result.floatNumber);
      } else {
        // evaulate the array element
        pos = 0;
        current_char = get_next_pgm_byte(pos);
        current_token = get_next_token();
        evaluate_and_print();
      }
    } else {
      // regular old variable assignment...maybe
      if (current_token.type == EQ) {
        eat(EQ);
        Value right(expr());
        store_var(varname, right);
      } else {
        // evaulate some void context variable or expression that's not 
        // an array element expression
        pos = 0;
        current_char = get_next_pgm_byte(pos);
        current_token = get_next_token();
        evaluate_and_print();
      }
    }
  } else {
    // DIM a new array of given size
    eat(LPAREN);
    int size = expr().number;
    eat(RPAREN);
    eat(AS);
    char type[10];
    strcpy(type, current_token.value.ToString());
    if (nocase_cmp(type, "INTEGER") == 0) {
      Value right(INTEGER, size);
      store_var(varname, right);
    } else {
      Value right(FLOAT, size);
      store_var(varname, right);
    }
    eat(ID);
  }
}

// gosub_statement: GOSUB label
void Interpreter::gosub_statement() {
  eat(GOSUB);
  char subname[20];
  strcpy(subname, current_token.value.ToString());
  int tempPos = pos;
  int tempLine = line_number;
  eat(ID);
  bool foundLabel = false;
  pos = 0; // rewind to beginning of program
  while (!foundLabel) {
    current_token = get_next_token();
    if (current_token.type == LABEL &&
        strcmp(subname, current_token.value.ToString()) == 0) {
      // found the sub
      eat(LABEL);
      foundLabel = true;
      while (current_token.type != RETURN) {
        statement();
        eat(NEWLINE);
      }
      eat(RETURN);

      // return to the caller..
      pos = tempPos;
      line_number = tempLine;
      current_char = get_next_pgm_byte(pos);
      current_token = get_next_token();
    }
  }
  // eat(NEWLINE);
}

// if_statement: IF expr THEN statement
void Interpreter::if_statement() {
  eat(IF);
  if (expr().ToBoolean()) {
    eat(THEN);
    statement();
    // optional newline
    if (current_token.type == NEWLINE) eat(NEWLINE);
    if (current_token.type == ELSE) {
      // if there was an else clause, eat it up...
      eat(ELSE);
      while (current_token.type != NEWLINE)
        current_token = get_next_token();
      eat(NEWLINE);
    }
  } else {
    eat(THEN);
    while (current_token.type != NEWLINE 
            && current_token.type != ELSE)
      current_token = get_next_token();
    if (current_token.type == NEWLINE) eat(NEWLINE);
    if (current_token.type == ELSE) {
      eat(ELSE);
      statement();
      eat(NEWLINE);
    } else {
      // no else was given...
      while (current_token.type != NEWLINE)
        current_token = get_next_token();
      eat(NEWLINE);
    }
  }
}

// statement: whiile_statement LPAREN expr RPAREN NEWLINE statement_list()
// NEWLINE WEND
void Interpreter::while_statement() {
  eat(WHILE);
  int tempPos = pos - 1;
  int tempLine = line_number;
  eat(LPAREN);
  while (expr().ToBoolean()) {
    eat(RPAREN);
    eat(NEWLINE);
    while (current_token.type != WEND) {
      statement();
      eat(NEWLINE);
    }
    // return back to the expression
    pos = tempPos;
    line_number = tempLine;
    current_char = get_next_pgm_byte(pos);
    current_token = get_next_token();
    eat(LPAREN);
  }
  if (current_token.type == RPAREN) {
    eat(RPAREN);
    int while_count = 1;
    while (while_count > 0) {
      // go thru block until we hit our WEND..
      //  skip other WHILE constructs if needed
      current_token = get_next_token();
      if (current_token.type == WHILE)
        while_count++;
      if (current_token.type == WEND)
        while_count--;
    }
    eat(WEND);
  }
}

// statement: for_statement assignment_statement TO expr STEP expr NEWLINE
// statement_list() NEWLINE NEXT ID
void Interpreter::for_statement() {
  int tempPos = pos + 3;
  int tempLine = line_number;
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
    while (current_token.type == NEWLINE) eat(NEWLINE);
    while (current_token.type != NEXT) {
      statement();
      if (current_token.type == NEWLINE) eat(NEWLINE);
    }

    store_var(varname, lookup_var(varname).number + incrVal);
    // return back to the expression
    pos = tempPos;
    line_number = tempLine;
    current_char = get_next_pgm_byte(pos);
    do {
      current_token = get_next_token();
    } while (current_token.type != TO);
    eat(TO);
  }
  int for_count = 1;
  while (for_count > 0) {
    // go thru block until we hit our NEXT..
    //  skip other FOR constructs if needed
    current_token = get_next_token();
    if (current_token.type == FOR)
      for_count++;
    if (current_token.type == NEXT)
      for_count--;
  }
  eat(NEXT);
  if (current_token.type == ID)
    eat(ID);
}

// statement: function_call | assignment_statement | empty
Value Interpreter::function_call() {
  int funcType = current_token.value.number;
  eat(FUNC_CALL);
  eat(LPAREN);

  if (funcType == FUNC_CALL_PRINT) {
    Value right(expr());
    eat(RPAREN);
    char *strVal = right.ToString();
#ifdef AVR_TARGET
    writeln(strVal);
#endif
#ifdef PC_TARGET
    writeln(strVal);
#endif
    return right;
  }
#ifdef AVR_TARGET
  else if (funcType == FUNC_CALL_FREERAM) {
    eat(RPAREN);
    Value right(freeRAM());
    return right;
  } else if (funcType == FUNC_CALL_REBOOT) {
    eat(RPAREN);
    wdt_enable(WDTO_120MS);
    while (1)
      ;
    Value right(0);
    return right;
  }
#endif
  else if (funcType == FUNC_CALL_ASSERT) {
#ifdef PC_TARGET
    Value item(expr());
    eat(RPAREN);
    if (!item.ToBoolean()) {
      printf("Assertion failed at line: %i\n", line_number);
      exit(1);
    }
#endif
  } else if (funcType == FUNC_CALL_UBOUND) {
    Value right(expr().arraySize - 1);
    eat(RPAREN);
    return right;
  } else if (funcType == FUNC_CALL_DELAY) {
    Value right(expr());
    eat(RPAREN);
    delayMs(right.number);
    return right;
  } else if (funcType == FUNC_CALL_SUBSTR) {
    Value refVar(expr());
    eat(COMMA);
    Value start(expr());
    eat(COMMA);
    Value len(expr());
    eat(RPAREN);
    char newStr[len.number + 1];
    memcpy(newStr, refVar.ToString(), len.number);
    newStr[len.number] = '\0';
    return Value(newStr);
  } else if (funcType == FUNC_CALL_LEN) {
    Value right(expr());
    Value leng((int)strlen(right.str));
    eat(RPAREN);
    return leng;
  }
#ifdef AVR_TARGET
  else if (funcType == FUNC_CALL_DDRA) {
    Value right(expr());
    eat(RPAREN);
    DDRA = right.number;
    return right;
  } else if (funcType == FUNC_CALL_PORTA) {
    Value right(expr());
    eat(RPAREN);
    PORTA = right.number;
    return right;
  } else if (funcType == FUNC_CALL_PINA) {
    eat(RPAREN);
    Value v(PINA);
    return v;
  } else if (funcType == FUNC_CALL_DDRB) {
    Value right(expr());
    eat(RPAREN);
    DDRB = right.number;
    return right;
  } else if (funcType == FUNC_CALL_DDRC) {
    Value right(expr());
    eat(RPAREN);
    DDRC = right.number;
    return right;
  } else if (funcType == FUNC_CALL_DDRD) {
    Value right(expr());
    eat(RPAREN);
    DDRD = right.number;
    return right;
  } else if (funcType == FUNC_CALL_PORTB) {
    Value right(expr());
    eat(RPAREN);
    PORTB = right.number;
    return right;
  } else if (funcType == FUNC_CALL_PORTC) {
    Value right(expr());
    eat(RPAREN);
    PORTC = right.number;
    return right;
  } else if (funcType == FUNC_CALL_PORTD) {
    Value right(expr());
    eat(RPAREN);
    PORTD = right.number;
    return right;
  } else if (funcType == FUNC_CALL_PINB) {
    eat(RPAREN);
    Value v(PINB);
    return v;
  } else if (funcType == FUNC_CALL_PINC) {
    eat(RPAREN);
    Value v(PINC);
    return v;
  } else if (funcType == FUNC_CALL_PIND) {
    eat(RPAREN);
    Value v(PIND);
    return v;
  } else if (funcType == FUNC_CALL_ADCSETUP) {
    eat(RPAREN);
    ADMUX = 0x40;  // use AREF PIN
    ADCSRA = 0x87; // ADC on, and divide by 128 for prescalar
  } else if (funcType == FUNC_CALL_SETPWM0) {
    Value right(expr());
    if (right.number <= 0) {
#if defined ATMEGA32 || defined ATMEGA1284
      PORTD &= ~(1 << 5);
#else
      PORTB &= ~(1 << 1);
#endif
    } else if (right.number >= 255) {
#if defined ATMEGA32 || defined ATMEGA1284
      PORTD |= (1 << 5);
#else
      PORTB |= (1 << 1);
#endif
    } else {
      TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (0 << WGM11) | (1 << WGM10);
      TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) |
               (0 << CS12) | (1 << CS11) | (1 << CS10);
      OCR1AH = 0x00;
      OCR1AL = right.number;
    }
    eat(RPAREN);
  } else if (funcType == FUNC_CALL_SETPWM1) {
    Value right(expr());
    if (right.number <= 0) {
#if defined ATMEGA32 || defined ATMEGA1284
      PORTD &= ~(1 << 4);
#else
      PORTB &= ~(1 << 2);
#endif
    } else if (right.number >= 255) {
#if defined ATMEGA32 || defined ATMEGA1284
      PORTD |= (1 << 4);
#else
      PORTB |= (1 << 2);
#endif
    } else {
      TCCR1A = (1 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (1 << WGM10);
      TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) |
               (0 << CS12) | (1 << CS11) | (1 << CS10);
      OCR1BH = 0x00;
      OCR1BL = right.number;
    }
    eat(RPAREN);
  } else if (funcType == FUNC_CALL_AREAD) {
    Value pin(expr());
    eat(RPAREN);
    ADMUX = 0x40 | pin.number;
    _delay_ms(1);
    ADCSRA |= (1 << ADSC); // Start the A/D conversion
    while ((ADCSRA & (1 << ADSC)))
      ; // wait for completion
    int ad_data = ADCW;
    Value res((double)(ad_data / 1023.0) * 5.0); // get result
    return res;
  }
#endif
#ifdef LCD_SUPPORT
  else if (funcType == FUNC_CALL_INITLCD) {
    Value right(expr());
    eat(RPAREN);
    InitLCD(right.ToBoolean());
  } else if (funcType == FUNC_CALL_SETXY) {
    Value row(expr());
    eat(COMMA);
    Value col(expr());
    eat(RPAREN);
    SetLCD_XY(row.number, col.number);
  } else if (funcType == FUNC_CALL_CLEARLCD) {
    eat(RPAREN);
    ClearLCD();
  } else if (funcType == FUNC_CALL_LCDPRINT) {
    Value right(expr());
    char *strVal = right.ToString();
    lcd_printf(strVal);
    eat(RPAREN);
    return right;
  }
#endif
#ifdef AVR_TARGET
  else if (funcType == FUNC_CALL_SENDSERIAL) {
    Value row(expr());
    eat(RPAREN);
    send_string(row.ToString());
    send_string("\r\n");
  } else if (funcType == FUNC_CALL_RXSERIAL) {
    eat(RPAREN);
    get_string(serialRxBuf, MAXSTRLENGTH, false);
    Value ret(serialRxBuf);
    return ret;
  } else if (funcType == FUNC_CALL_TXBYTE) {
    Value right(expr());
    eat(RPAREN);
    send_byte((char)right.number);
    Value v(0);
    return v;
  } else if (funcType == FUNC_CALL_RXBYTE) {
    eat(RPAREN);
    Value rx(get_byte());
    return rx;
  }
#endif

  // default return for functions that don't return anything
  Value v(0);
  return v;
}

/*
expr: term ((PLUS | MINUS | STR_CONCAT | LSHIFT | RSHIFT) term)*
*/
Value Interpreter::expr() {
  Value result = term();

  while (current_token.type == PLUS || current_token.type == MINUS ||
         current_token.type == STR_CONCAT || current_token.type == LSHIFT ||
         current_token.type == RSHIFT) {
    Token token = current_token;
    if (token.type == PLUS) {
      eat(PLUS);
      result = (result + term());
    } else if (token.type == MINUS) {
      eat(MINUS);
      result = (result - term());
    } else if (token.type == STR_CONCAT) {
      eat(STR_CONCAT);
      result = (result & term());
    } else if (token.type == LSHIFT) {
      eat(LSHIFT);
      result = (result << term());
    } else if (token.type == RSHIFT) {
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
Value Interpreter::term() {
  Value result = factor();

  while (current_token.type == MUL || current_token.type == DIV ||
         current_token.type == INTDIV || current_token.type == AND || current_token.type == OR ||
         current_token.type == XOR || current_token.type == GT ||
         current_token.type == GTE || current_token.type == LT ||
         current_token.type == LTE || current_token.type == MOD ||
         current_token.type == NEQ || current_token.type == EQ) {
    Token token = current_token;
    // if (token.type == POW)
    // {
    // eat(POW);
    // result = result.pow(factor());
    // }
    if (token.type == MUL) {
      eat(MUL);
      result = result * factor();
    } else if (token.type == DIV) {
      eat(DIV);
      result = result / factor();
    }
    else if (token.type == INTDIV)
    {
      eat(INTDIV);
      result = result.intDiv(factor());
    }
    else if (token.type == MOD) {
      eat(MOD);
      result = result % factor();
    } else if (token.type == EQ) {
      eat(EQ);
      result = (result == factor());
    } else if (token.type == NEQ) {
      eat(NEQ);
      result = (result != factor());
    } else if (token.type == LT) {
      eat(LT);
      result = (result < factor());
    } else if (token.type == LTE) {
      eat(LTE);
      result = (result <= factor());
    } else if (token.type == GT) {
      eat(GT);
      result = (result > factor());
    } else if (token.type == GTE) {
      eat(GTE);
      result = (result >= factor());
    } else if (token.type == AND) {
      eat(AND);
      result = (result & factor());
    } else if (token.type == OR) {
      eat(OR);
      result = (result | factor());
    } else if (token.type == XOR) {
      eat(XOR);
      result = (result ^ factor());
    }
  }

  return result;
}

// factor : (PLUS | MINUS) factor | INTEGER | LPAREN expr RPAREN | (NOT) BOOLEAN
// | STRING | ID
Value Interpreter::factor() {
  Token token = current_token;

  if (token.type == PLUS) {
    eat(PLUS);
    return +factor();
  } else if (token.type == MINUS) {
    eat(MINUS);
    return -factor();
  } else if (token.type == NOT) {
    eat(NOT);
    return !factor();
  } else if (token.type == BOOL) {
    eat(BOOL);
    return token.value;
  } else if (token.type == STR) {
    eat(STR);
    return token.value;
  } else if (token.type == ID) {
    eat(ID);
    if (current_token.type == LPAREN) {
      // we have an array element here, look it up
      eat(LPAREN);
      int index = expr().number;
      eat(RPAREN);
      Value parentVal = lookup_var(token.value.ToString());
      Value v = parentVal.index_array(index);
      return v;
    } else {
      Value v = lookup_var(token.value.ToString());
      return v;
    }
  } else if (token.type == FUNC_CALL) {
    return function_call();
  } else if (token.type == INT) {
    eat(INT);
    return token.value;
  } else if (token.type == FLOATNUM) {
    eat(FLOATNUM);
    return token.value;
  } else if (token.type == LPAREN) {
    eat(LPAREN);
    Value result = expr();
    eat(RPAREN);
    return result;
  }

  error("Invalid Factor");

  Value v(0);
  return v;
}

Value Interpreter::lookup_var(const char *name) {
  int i = 0;
  for (i = 0; i < NUM_VARS; i++) {
    if (strcmp(vars[i], name) == 0)
      break;
  }

  if (i >= NUM_VARS) {
    char b[30];
    sprintf(b, "Var %s not found", name);
    error(b);
  }

  return vals[i];
}

bool Interpreter::store_var(const char *name, Value v) {
  int i = 0;
  for (i = 0; i < NUM_VARS; i++) {
    // var name found, update the existing value
    if (strcmp(vars[i], name) == 0) {
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

void Interpreter::delayMs(int number) {
#ifdef AVR_TARGET
  int i = 0;
  for (i = 0; i < number; i++)
    _delay_ms(1);
#endif
#ifdef PC_TARGET
  usleep(number * 1000);
#endif
}

#ifdef AVR_TARGET
int Interpreter::strlen_ee(char *str) {
  int i = 0;
  int len = 0;
  char c = (char)eeprom_read_byte((uint8_t *)&str[i++]);
  while (c != '\0') {
    len++;
    c = (char)eeprom_read_byte((uint8_t *)&str[i++]);
  }
  return len;
}

int Interpreter::freeRAM() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0) ? (int)&__heap_start : (int)__brkval;
}
#endif
