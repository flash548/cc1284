/*
* Value.cpp
*
* Created: 12/14/2018 8:36:21 PM
* Author: z
*/


#include "Value.h"
#include <stdlib.h>

#ifdef AVR_TARGET
	#include <avr/io.h>
#endif

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "Serial.h"

Value::Value() {}
// constructor for a INT VARIABLE
Value::Value(int i) 
{ 
	number = i; 
	sprintf(str, "%i", number); 
	type = INTEGER; 
	isArray = false;
}
// constructor for a CHAR/INT variable
Value::Value(char i) 
{ 
	number = (int)i; 
	sprintf(str, "%i", number); 
	type = INTEGER; 
	isArray = false;
}
// constructor for a STRING variable
Value::Value(const char* s) 
{
  	number = 0;
	strcpy(str, s); 
	type = STRING; 
	isArray = false;
}
// constructor for a BOOLEAN variable
Value::Value(bool b) 
{ 
  if (b) {
    number = 1;
  } else {
    number = 0;
  }
	bval = b; 
	type = BOOLEAN; 
	isArray = false;
}
// constructor for a FLOAT variable
Value::Value(double b)
{
  number = (int)b;
	floatNumber = b;
	type = FLOAT;
	isArray = false;
}
// constructor for a ARRAY variable of TYPE
Value::Value(TYPE t, int size)
{
    type = t;
    isArray = true;
    arraySize = size;
    if (t == INTEGER) {
	    intArray = (int *) malloc(size*sizeof(int));
	    for (int i = 0; i < size; i++) { intArray[i] = 0; }
    }
    else if (t == FLOAT) {
	    dblArray = (double *) malloc(size*sizeof(double));
	    for (int i = 0; i < size; i++) { dblArray[i] = 0.0f; }
    }
}

Value operator+(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		char tmp[MAXSTRLENGTH];
		if (v1.type == INTEGER) {
			sprintf(tmp, "%i", v1.number);
			strcpy(tmp, v1.str);
			strcat(tmp, v2.str);
			Value v(tmp);
			return v;
		}
		else if (v1.type == FLOAT) {
			sprintf(tmp, "%f", v1.floatNumber);
			strcpy(tmp, v1.str);
			strcat(tmp, v2.str);
			Value v(tmp);
			return v;
		}
		else if (v2.type == INTEGER) {
			char tmp2[MAXSTRLENGTH];
			sprintf(tmp2, "%i", v2.number);
			strcpy(tmp, v1.str);
			strcat(tmp, tmp2);
			Value v(tmp);
			return v;
		}
		else if (v2.type == FLOAT) {
			char tmp2[MAXSTRLENGTH];
			sprintf(tmp2, "%f", v2.floatNumber);
			strcpy(tmp, v1.str);
			strcat(tmp, tmp2);
			Value v(tmp);
			return v;
		}
		else {
			strcpy(tmp, v1.str);
			strcat(tmp, v2.str);
			Value v(tmp);
			return v;
		}
		
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber + v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber + (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number + v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number + v2.number);
		return v;
	}
}

Value operator-(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber - v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber - (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number - v2.floatNumber);
			return v;
		}
	}
	
	// must be both INTs
	Value v(v1.number - v2.number);
	return v;
	
}

Value operator*(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber * v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber * (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number * v2.floatNumber);
			return v;
		}
	}
	
	// must be both INTs
	Value v(v1.number * v2.number);
	return v;

}

Value operator/(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber / v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber / (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number / v2.floatNumber);
			return v;
		}
	}
	
	// must be both INTs
	Value v((float)v1.number / (float)v2.number);
	return v;
}

Value operator%(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v((int)v1.floatNumber % (int)v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v((int)v1.floatNumber % v2.number);
			return v;
		}
		else {
			Value v(v1.number % (int)v2.floatNumber);
			return v;
		}
	}
	
	// must be both INTs
	Value v(v1.number % v2.number);
	return v;
}

Value operator<<(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v((int)v1.floatNumber << (int)v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v((int)v1.floatNumber << v2.number);
			return v;
		}
		else {
			Value v(v1.number << (int)v2.floatNumber);
			return v;
		}
	}
	
	// must be both INTs
	Value v(v1.number << v2.number);
	return v;
}

Value operator>>(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v((int)v1.floatNumber >> (int)v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v((int)v1.floatNumber >> v2.number);
			return v;
		}
		else {
			Value v(v1.number >> (int)v2.floatNumber);
			return v;
		}
	}
	
	// must be both INTs
	Value v(v1.number >> v2.number);
	return v;
	
}

Value operator&(const Value& v1, const Value& v2)
{
	if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
		Value v(v1.bval && v2.bval);
		return v;
	}
	else if (v1.type == STRING || v2.type == STRING) {
		char tmp[MAXSTRLENGTH];
		if (v1.type == INTEGER) {
			sprintf(tmp, "%i", v1.number);
			strcpy(tmp, v1.str);
			strcat(tmp, v2.str);
			Value v(tmp);
			return v;
		}
		else if (v1.type == FLOAT) {
			sprintf(tmp, "%lf", v1.floatNumber);
			strcpy(tmp, v1.str);
			strcat(tmp, v2.str);
			Value v(tmp);
			return v;
		}
		else if (v2.type == INTEGER) {
			char tmp2[MAXSTRLENGTH];
			sprintf(tmp2, "%i", v2.number);
			strcpy(tmp, v1.str);
			strcat(tmp, tmp2);
			Value v(tmp);
			return v;
		}
		else if (v2.type == FLOAT) {
			char tmp2[MAXSTRLENGTH];
			sprintf(tmp2, "%f", v2.floatNumber);
			strcpy(tmp, v1.str);
			strcat(tmp, tmp2);
			Value v(tmp);
			return v;
		}
		else {
			strcpy(tmp, v1.str);
			strcat(tmp, v2.str);
			Value v(tmp);
			return v;
		}
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v((int)v1.floatNumber & (int)v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v((int)v1.floatNumber & v2.number);
			return v;
		}
		else {
			Value v(v1.number & (int)v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number & v2.number);
		return v;
	}
}
Value operator|(const Value& v1, const Value& v2)
{
	if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
		Value v(v1.bval || v2.bval);
		return v;
	}
	else if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v((int)v1.floatNumber | (int)v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v((int)v1.floatNumber | v2.number);
			return v;
		}
		else {
			Value v(v1.number | (int)v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number | v2.number);
		return v;
	}
}
Value operator !(const Value& v1)
{
	if (v1.type == BOOLEAN) {
		Value v(!v1.bval);
		return v;
	}
	else if (v1.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT) {
		Value v(~(int)v1.floatNumber);
		return v;
	}
	else {
		Value v(~v1.number);
		return v;
	}
}
Value operator ^(const Value& v1, const Value& v2)
{
	if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
		Value v(v1.bval ^ v2.bval);
		return v;
	}
	else if (v1.type == STRING || v2.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v((int)v1.floatNumber ^ (int)v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v((int)v1.floatNumber ^ v2.number);
			return v;
		}
		else {
			Value v(v1.number ^ (int)v2.floatNumber);
			return v;
		}
	}
	else {
		Value v((int)v1.number ^ (int)v2.number);
		return v;
	}
}
Value operator ==(const Value& v1, const Value& v2)
{
	if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
		Value v(v1.bval == v2.bval);
		return v;
	}
	else if (v1.type == STRING && v2.type == STRING) {
		Value v(strcmp(v1.str, v2.str) == 0);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber == v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber == (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number == v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number == v2.number);
		return v;
	}
}
Value operator !=(const Value& v1, const Value& v2)
{
	if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
		Value v(v1.bval != v2.bval);
		return v;
	}
	else if (v1.type == STRING && v2.type == STRING) {
		Value v(strcmp(v1.str, v2.str) != 0);
		return v;
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber != v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber != (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number != v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number != v2.number);
		return v;
	}
}
Value operator <(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		if (v1.type == INTEGER) {
			Value v(v1.number < (int16_t)strlen(v2.str));
			return v;
		}
		else {
			Value v((int16_t)strlen(v1.str) < v2.number);
			return v;
		}
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber < v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber < (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number < v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number < v2.number);
		return v;
	}
}
Value operator <=(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		if (v1.type == INTEGER) {
			Value v(v1.number <= (int16_t)strlen(v2.str));
			return v;
		}
		else {
			Value v((int16_t)strlen(v1.str) <= v2.number);
			return v;
		}
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber <= v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber <= (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number <= v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number <= v2.number);
		return v;
	}
}
Value operator >(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		if (v1.type == INTEGER) {
			Value v(v1.number > (int16_t)strlen(v2.str));
			return v;
		}
		else {
			Value v((int16_t)strlen(v1.str) > v2.number);
			return v;
		}
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber > v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber > (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number > v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number > v2.number);
		return v;
	}
}
Value operator >=(const Value& v1, const Value& v2)
{
	if (v1.type == STRING || v2.type == STRING) {
		if (v1.type == INTEGER) {
			Value v(v1.number >= (int16_t)strlen(v2.str));
			return v;
		}
		else {
			Value v((int16_t)strlen(v1.str) >= v2.number);
			return v;
		}
	}
	else if (v1.type == FLOAT && v2.type == FLOAT) {
		Value v(v1.floatNumber >= v2.floatNumber);
		return v;
	}
	else if (v1.type == FLOAT || v2.type == FLOAT) {
		if (v1.type == FLOAT) {
			Value v(v1.floatNumber >= (float)v2.number);
			return v;
		}
		else {
			Value v((float)v1.number >= v2.floatNumber);
			return v;
		}
	}
	else {
		Value v(v1.number >= v2.number);
		return v;
	}
}

// unary operators
Value operator +(const Value& v1)
{
	if (v1.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT) {
		Value v(+v1.floatNumber);
		return v;
	}
	else {
		Value v(+v1.number);
		return v;
	}
}
Value operator -(const Value& v1)
{
	if (v1.type == STRING) {
		Value v(v1.str);
		return v;
	}
	else if (v1.type == FLOAT) {
		Value v(-v1.floatNumber);
		return v;
	}
	else {
		Value v(-v1.number);
		return v;
	}
}

bool Value::ToBoolean()
{
	if (type == STRING) {
		return true;
	}
	else if (type == BOOLEAN) {
		return bval;
	}
	else if (type == FLOAT) {
		return floatNumber != 0;
	}
	else {
		return number != 0;
	}

}

char* Value::ToString()
{
	if (type == STRING) return str;
	else if (type == BOOLEAN) {
		if (bval) return "TRUE";
		else return "FALSE";
	}
	else if (type == FLOAT) {
		sprintf(str, "%f", floatNumber);
		return str;
	}
	else {
		sprintf(str, "%i", number);
		return str;
	}
}

Value Value::index_array(int index)
{
	if (isArray) {
		if (type == INTEGER) return Value(intArray[index]);
		else if (type == FLOAT) return Value(dblArray[index]);
	} else if (type == STRING) {
		char buf[2];
		sprintf(buf, "%c", str[index]);
		return Value(buf);
	}
	return Value(-1);
}

void Value::update_array(int index, int val)
{
	if (isArray) {
		intArray[index] = val;
	}
}

void Value::update_array(int index, double val)
{
	if (isArray) {
		dblArray[index] = val;
	}
}

Value Value::intDiv(const Value& v2) {
  return Value(number / v2.number);
}
