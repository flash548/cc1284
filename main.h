/*
 * main.h
 *
 * Created: 12/14/2018 9:30:35 PM
 *  Author: z
 */
#ifndef MAIN_H_
#define MAIN_H_

// #define PC_TARGET
#define AVR_TARGET

#ifdef AVR_TARGET

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

enum TokenType
{
    MUL,
    MINUS,
    MOD,
    PLUS,
    DIV,
    LSHIFT,
    RSHIFT,
    AND,
    OR,
    XOR,
    IF,
    ELSE,
    THEN,
    ID,
    NOT,
    GT,
    LT,
    LTE,
    GTE,
    EQ,
    NEQ,
    TRUE,
    FALSE,
    NEWLINE,
    RETURN,
    STR_CONCAT,
    COMMA,
    COLON,
    INCR,
    DECR,
    POW,
    INTDIV,
    LPAREN,
    RPAREN,
    END,
    INT,
    STR,
    BOOL,
    END_IF,
    WHILE,
    WEND,
    DIM,
    LET,
    FLOATNUM,
    AS,
    FUNC_CALL,
    FOR,
    TO,
    STEP,
    NEXT,
    GOSUB,
    LABEL
};

#endif /* MAIN_H_ */
