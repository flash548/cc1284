/*
 * main.h
 *
 * Created: 12/14/2018 9:30:35 PM
 *  Author: z
 */
#ifndef MAIN_H_
#define MAIN_H_

#define PC_TARGET
// #define AVR_TARGET

#ifdef AVR_TARGET
#include <avr/pgmspace.h>
// declare type of chip for internal use
#define ATMEGA1284
// #define ATMEGA32

// define the CPU freq in Hz
#define F_CPU 14745600UL
// #define F_CPU 16000000UL

// define EEPROM PRGM SIZE
#if defined ATMEGA32 || defined ATMEGA328
#define EEPROM_PGM_SIZE 1024
#elif defined ATMEGA1284
#define EEPROM_PGM_SIZE 4096
#else
#define EEPROM_PGM_SIZE 1024
#endif

// enable LCD support
// #define LCD_SUPPORT

// enable UART at 115.2kbaud
#define UART_SUPPORT

// send error messages to LCD
// #define DEBUG_ON_LCD

// send error messages over UART
#define DEBUG_ON_SERIAL

#endif

// MAX length a string variable can be
#define MAXSTRLENGTH 25

// number of variables we can declare/have
#define NUM_VARS 6

// max number of variable name chars
#define MAX_VARNAME_LEN 8

enum TokenType {
  MUL,        // 0
  MINUS,      // 1
  MOD,        // 2
  PLUS,       // 3
  DIV,        // 4
  LSHIFT,     // 5
  RSHIFT,     // 6
  AND,        // 7
  OR,         // 8
  XOR,        // 9
  IF,         // 10
  ELSE,       // 11
  THEN,       // 12
  ID,         // 13
  NOT,        // 14
  GT,         // 15
  LT,         // 16
  LTE,        // 17
  GTE,        // 18
  EQ,         // 19
  NEQ,        // 20
  TRUE,       // 21
  FALSE,      // 22
  NEWLINE,    // 23
  RETURN,     // 24
  STR_CONCAT, // 25
  COMMA,      // 26
  COLON,      // 27
  INCR,       // 28
  DECR,       // 29
  POW,        // 30
  INTDIV,     // 31
  LPAREN,     // 32
  RPAREN,     // 33
  END,        // 34
  INT,        // 35
  STR,        // 36
  BOOL,       // 37
  END_IF,     // 38
  WHILE,      // 39
  WEND,       // 40
  DIM,        // 41
  LET,        // 42
  FLOATNUM,   // 43
  AS,         // 44
  FUNC_CALL,  // 45
  FOR,        // 46
  TO,         // 47
  STEP,       // 48
  NEXT,       // 49
  GOSUB,      // 50
  LABEL,      // 51
  COMMENT,    // 52
};

#ifdef AVR_TARGET
// weird AVR PROGMEM string table requirements...
// https://www.nongnu.org/avr-libc/user-manual/pgmspace.html
const char _MUL[] PROGMEM = "MUL";               // 0
const char _MINUS[] PROGMEM = "MINUS";           // 1
const char _MOD[] PROGMEM = "MOD";               // 2
const char _PLUS[] PROGMEM = "PLUS";             // 3
const char _DIV[] PROGMEM = "DIV";               // 4
const char _LSHIFT[] PROGMEM = "LSHIFT";         // 5
const char _RSHIFT[] PROGMEM = "RSHIFT";         // 6
const char _AND[] PROGMEM = "AND";               // 7
const char _OR[] PROGMEM = "OR";                 // 8
const char _XOR[] PROGMEM = "XOR";               // 9
const char _IF[] PROGMEM = "IF";                 // 10
const char _ELSE[] PROGMEM = "ELSE";             // 11
const char _THEN[] PROGMEM = "THEN";             // 12
const char _ID[] PROGMEM = "ID";                 // 13
const char _NOT[] PROGMEM = "NOT";               // 14
const char _GT[] PROGMEM = "GT";                 // 15
const char _LT[] PROGMEM = "LT";                 // 16
const char _LTE[] PROGMEM = "LTE";               // 17
const char _GTE[] PROGMEM = "GTE";               // 18
const char _EQ[] PROGMEM = "EQ";                 // 19
const char _NEQ[] PROGMEM = "NEQ";               // 20
const char _TRUE[] PROGMEM = "TRUE";             // 21
const char _FALSE[] PROGMEM = "FALSE";           // 22
const char _NEWLINE[] PROGMEM = "NEWLINE";       // 23
const char _RETURN[] PROGMEM = "RETURN";         // 24
const char _STR_CONCAT[] PROGMEM = "STR_CONCAT"; // 25
const char _COMMA[] PROGMEM = "COMMA";           // 26
const char _COLON[] PROGMEM = "COLON";           // 27
const char _INCR[] PROGMEM = "INCR";             // 28
const char _DECR[] PROGMEM = "DECR";             // 29
const char _POW[] PROGMEM = "POW";               // 30
const char _INTDIV[] PROGMEM = "INTDIV";         // 31
const char _LPAREN[] PROGMEM = "LPAREN";         // 32
const char _RPAREN[] PROGMEM = "RPAREN";         // 33
const char _END[] PROGMEM = "END";               // 34
const char _INT[] PROGMEM = "INT";               // 35
const char _STR[] PROGMEM = "STR";               // 36
const char _BOOL[] PROGMEM = "BOOL";             // 37
const char _END_IF[] PROGMEM = "END_IF";         // 38
const char _WHILE[] PROGMEM = "WHILE";           // 39
const char _WEND[] PROGMEM = "WEND";             // 40
const char _DIM[] PROGMEM = "DIM";               // 41
const char _LET[] PROGMEM = "LET";               // 42
const char _FLOATNUM[] PROGMEM = "FLOATNUM";     // 43
const char _AS[] PROGMEM = "AS";                 // 44
const char _FUNC_CALL[] PROGMEM = "FUNC_CALL";   // 45
const char _FOR[] PROGMEM = "FOR";               // 46
const char _TO[] PROGMEM = "TO";                 // 47
const char _STEP[] PROGMEM = "STEP";             // 48
const char _NEXT[] PROGMEM = "NEXT";             // 49
const char _GOSUB[] PROGMEM = "GOSUB";           // 50
const char _LABEL[] PROGMEM = "LABEL";           // 51
const char _COMMENT[] PROGMEM = "COMMENT";       // 52
const char const* token_strings[] PROGMEM =
    {
        _MUL,        // 0
        _MINUS,      // 1
        _MOD,        // 2
        _PLUS,       // 3
        _DIV,        // 4
        _LSHIFT,     // 5
        _RSHIFT,     // 6
        _AND,        // 7
        _OR,         // 8
        _XOR,        // 9
        _IF,         // 10
        _ELSE,       // 11
        _THEN,       // 12
        _ID,         // 13
        _NOT,        // 14
        _GT,         // 15
        _LT,         // 16
        _LTE,        // 17
        _GTE,        // 18
        _EQ,         // 19
        _NEQ,        // 20
        _TRUE,       // 21
        _FALSE,      // 22
        _NEWLINE,    // 23
        _RETURN,     // 24
        _STR_CONCAT, // 25
        _COMMA,      // 26
        _COLON,      // 27
        _INCR,       // 28
        _DECR,       // 29
        _POW,        // 30
        _INTDIV,     // 31
        _LPAREN,     // 32
        _RPAREN,     // 33
        _END,        // 34
        _INT,        // 35
        _STR,        // 36
        _BOOL,       // 37
        _END_IF,     // 38
        _WHILE,      // 39
        _WEND,       // 40
        _DIM,        // 41
        _LET,        // 42
        _FLOATNUM,   // 43
        _AS,         // 44
        _FUNC_CALL,  // 45
        _FOR,        // 46
        _TO,         // 47
        _STEP,       // 48
        _NEXT,       // 49
        _GOSUB,      // 50
        _LABEL,      // 51
        _COMMENT,    // 52
}
#endif
#ifdef PC_TARGET
    static const char *
    token_strings[] = {
        "MUL",        // 0
        "MINUS",      // 1
        "MOD",        // 2
        "PLUS",       // 3
        "DIV",        // 4
        "LSHIFT",     // 5
        "RSHIFT",     // 6
        "AND",        // 7
        "OR",         // 8
        "XOR",        // 9
        "IF",         // 10
        "ELSE",       // 11
        "THEN",       // 12
        "ID",         // 13
        "NOT",        // 14
        "GT",         // 15
        "LT",         // 16
        "LTE",        // 17
        "GTE",        // 18
        "EQ",         // 19
        "NEQ",        // 20
        "TRUE",       // 21
        "FALSE",      // 22
        "NEWLINE",    // 23
        "RETURN",     // 24
        "STR_CONCAT", // 25
        "COMMA",      // 26
        "COLON",      // 27
        "INCR",       // 28
        "DECR",       // 29
        "POW",        // 30
        "INTDIV",     // 31
        "LPAREN",     // 32
        "RPAREN",     // 33
        "END",        // 34
        "INT",        // 35
        "STR",        // 36
        "BOOL",       // 37
        "END_IF",     // 38
        "WHILE",      // 39
        "WEND",       // 40
        "DIM",        // 41
        "LET",        // 42
        "FLOATNUM",   // 43
        "AS",         // 44
        "FUNC_CALL",  // 45
        "FOR",        // 46
        "TO",         // 47
        "STEP",       // 48
        "NEXT",       // 49
        "GOSUB",      // 50
        "LABEL",      // 51
        "COMMENT",    // 52
};
#endif

#endif /* MAIN_H_ */
