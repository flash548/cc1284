/*
 * Value.h
 *
 * Created: 12/14/2018 8:36:21 PM
 * Author: z
 */

#include "main.h"

#ifndef __VALUE_H__
#define __VALUE_H__
#define TRUE_INT_VAL -1
#define FALSE_INT_VAL 0

enum TYPE { INTEGER, STRING, BOOLEAN, FLOAT };

union ItemValue {
  int number;
  int *intArray;
  double *dblArray;
  double floatNumber;
  bool bval;
  char *string;
};

class Value {

public:
  ItemValue value;
  bool isArray = false;
  int arraySize;
  TYPE type;
  bool ToBoolean();
  int ToInt() const;
  char *ToString();
  Value();
  Value(int i);
  Value(char i);
  Value(const char *s);
  Value(bool b);
  Value(TYPE t, int size);
  Value(double i);
  // Value(const Value& v);
  ~Value();
  Value index_array(int index);
  void update_array(int index, int val);
  void update_array(int index, double val);
  friend Value operator+(const Value &v1);
  friend Value operator-(const Value &v1);
  friend Value operator+(const Value &v1, const Value &v2);
  friend Value operator-(const Value &v1, const Value &v2);
  friend Value operator*(const Value &v1, const Value &v2);
  friend Value operator/(const Value &v1, const Value &v2);
  friend Value operator%(const Value &v1, const Value &v2);
  friend Value operator<<(const Value &v1, const Value &v2);
  friend Value operator>>(const Value &v1, const Value &v2);
  friend Value operator&(const Value &v1, const Value &v2);
  friend Value operator|(const Value &v1, const Value &v2);
  friend Value operator!(const Value &v1);
  friend Value operator^(const Value &v1, const Value &v2);
  friend Value operator==(const Value &v1, const Value &v2);
  friend Value operator!=(const Value &v1, const Value &v2);
  friend Value operator>(const Value &v1, const Value &v2);
  friend Value operator>=(const Value &v1, const Value &v2);
  friend Value operator<(const Value &v1, const Value &v2);
  friend Value operator<=(const Value &v1, const Value &v2);

}; // Value

struct Token {
  TokenType type;
  Value value;
};

#endif //__VALUE_H__
