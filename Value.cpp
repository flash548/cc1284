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

#include "Serial.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

Value::Value() {}
// Value::Value(const Value &a) { printf("Copy\n"); }
// constructor for a INT VARIABLE
Value::Value(int i) {
  ItemValue item;
  item.number = i;
  value = item;
  type = INTEGER;
}
// constructor for a CHAR/INT variable
Value::Value(char i) {
  ItemValue item;
  item.number = (int)i;
  value = item;
  type = INTEGER;
}
// constructor for a STRING variable
Value::Value(const char *s) {
  ItemValue item;
  int len = strlen(s);
  item.string = (char *)malloc((len * sizeof(char)) + 1);
  strcpy(item.string, s);
  item.string[len] = '\0';
  value = item;
  type = STRING;
}
// constructor for a BOOLEAN variable
Value::Value(bool b) {
  ItemValue item;
  item.bval = b;
  value = item;
  type = BOOLEAN;
}
// constructor for a FLOAT variable
Value::Value(double b) {
  ItemValue item;
  item.floatNumber = b;
  value = item;
  type = FLOAT;
}
// constructor for a ARRAY variable of TYPE
Value::Value(TYPE t, int size) {
  type = t;
  printf("ALLOC ARAY");
  ItemValue item;
  isArray = true;
  arraySize = size;
  if (t == INTEGER) {
    item.intArray = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
      item.intArray[i] = 0;
    }
  } else if (t == FLOAT) {
    item.dblArray = (double *)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
      item.dblArray[i] = 0.0f;
    }
  }

  value = item;
}

Value::~Value() {
  if (type == STR) {
    printf("FREEING STRING\n");
    free(value.string);
  } else if (isArray && type == INTEGER) {
    printf("FREEING ARRAY\n");
    free(value.intArray);
    value.intArray = NULL;
  } else if (isArray && type == FLOAT) {
    printf("FREEING DB ARRAY\n");
    free(value.dblArray);
  }
}

Value operator+(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      char tempIntStr[MAXSTRLENGTH];
      sprintf(tempIntStr, "%i", v1.value.number);
      char tmp[strlen(tempIntStr) + strlen(v2.value.string) + 1];
      strcpy(tmp, tempIntStr);
      strcat(tmp, v2.value.string);
      Value v(tmp);
      return v;
    } else if (v1.type == FLOAT) {
      char tempFloatStr[MAXSTRLENGTH];
      sprintf(tempFloatStr, "%lf", v1.value.floatNumber);
      char tmp[strlen(tempFloatStr) + strlen(v2.value.string) + 1];
      strcpy(tmp, tempFloatStr);
      strcat(tmp, v2.value.string);
      Value v(tmp);
      return v;
    } else if (v2.type == INTEGER) {
      char tempIntStr[MAXSTRLENGTH];
      sprintf(tempIntStr, "%i", v2.value.number);
      char tmp[strlen(tempIntStr) + strlen(v1.value.string) + 1];
      strcpy(tmp, v1.value.string);
      strcat(tmp, tempIntStr);
      Value v(tmp);
      return v;
    } else if (v2.type == FLOAT) {
      char tempFloatStr[MAXSTRLENGTH];
      sprintf(tempFloatStr, "%lf", v2.value.floatNumber);
      char tmp[strlen(tempFloatStr) + strlen(v1.value.string) + 1];
      strcat(tmp, v1.value.string);
      strcpy(tmp, tempFloatStr);
      Value v(tmp);
      return v;
    } else {
      // string to string concat
      char tmp[strlen(v1.value.string) + strlen(v2.value.string) + 1];
      strcpy(tmp, v1.value.string);
      strcat(tmp, v2.value.string);
      Value v(tmp);
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber + v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber + (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number + v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number + v2.value.number);
    return v;
  }
}

Value operator-(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber - v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber - (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number - v2.value.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.value.number - v2.ToInt());
  return v;
}

Value operator*(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber * v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber * (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number * v2.value.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.value.number * v2.ToInt());
  return v;
}

Value operator/(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber / v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber / (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number / v2.value.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.value.number / v2.ToInt());
  return v;
}

Value operator%(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.value.floatNumber % (int)v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.value.floatNumber % v2.ToInt());
      return v;
    } else {
      Value v(v1.value.number % (int)v2.value.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.value.number % v2.ToInt());
  return v;
}

Value operator<<(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.value.floatNumber << (int)v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.value.floatNumber << v2.ToInt());
      return v;
    } else {
      Value v(v1.value.number << (int)v2.value.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.value.number << v2.ToInt());
  return v;
}

Value operator>>(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.value.floatNumber >> (int)v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.value.floatNumber >> v2.ToInt());
      return v;
    } else {
      Value v(v1.value.number >> (int)v2.value.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.value.number >> v2.ToInt());
  return v;
}

Value operator&(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.value.bval && v2.value.bval);
    return v;
  } else if (v1.type == STRING || v2.type == STRING) {
    char tmp[MAXSTRLENGTH];
    if (v1.type == INTEGER) {
      sprintf(tmp, "%i", v1.value.number);
      strcpy(tmp, v1.value.string);
      strcat(tmp, v2.value.string);
      Value v(tmp);
      return v;
    } else if (v1.type == FLOAT) {
      sprintf(tmp, "%lf", v1.value.floatNumber);
      strcpy(tmp, v1.value.string);
      strcat(tmp, v2.value.string);
      Value v(tmp);
      return v;
    } else if (v2.type == INTEGER) {
      char tmp2[MAXSTRLENGTH];
      sprintf(tmp2, "%i", v2.ToInt());
      strcpy(tmp, v1.value.string);
      strcat(tmp, tmp2);
      Value v(tmp);
      return v;
    } else if (v2.type == FLOAT) {
      char tmp2[MAXSTRLENGTH];
      sprintf(tmp2, "%lf", v2.value.floatNumber);
      strcpy(tmp, v1.value.string);
      strcat(tmp, tmp2);
      Value v(tmp);
      return v;
    } else {
      strcpy(tmp, v1.value.string);
      strcat(tmp, v2.value.string);
      Value v(tmp);
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.value.floatNumber & (int)v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.value.floatNumber & v2.ToInt());
      return v;
    } else {
      Value v(v1.value.number & (int)v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number & v2.ToInt());
    return v;
  }
}
Value operator|(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.value.bval || v2.value.bval);
    return v;
  } else if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.value.floatNumber | (int)v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.value.floatNumber | v2.ToInt());
      return v;
    } else {
      Value v(v1.value.number | (int)v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number | v2.ToInt());
    return v;
  }
}
Value operator!(const Value &v1) {
  if (v1.type == BOOLEAN) {
    Value v(!v1.value.bval);
    return v;
  } else if (v1.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT) {
    Value v(~(int)v1.value.floatNumber);
    return v;
  } else {
    Value v(~v1.value.number);
    return v;
  }
}
Value operator^(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.value.bval ^ v2.value.bval);
    return v;
  } else if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.value.floatNumber ^ (int)v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.value.floatNumber ^ v2.ToInt());
      return v;
    } else {
      Value v(v1.value.number ^ (int)v2.value.floatNumber);
      return v;
    }
  } else {
    Value v((int)v1.value.number ^ (int)v2.ToInt());
    return v;
  }
}
Value operator==(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.value.bval == v2.value.bval);
    return v;
  } else if (v1.type == STRING && v2.type == STRING) {
    Value v(strcmp(v1.value.string, v2.value.string) == 0);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber == v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber == (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number == v2.value.floatNumber);
      return v;
    }
  } else {
    // integer compare
    Value v(v1.value.number == v2.value.number);
    return v;
  }
}
Value operator!=(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.value.bval != v2.value.bval);
    return v;
  } else if (v1.type == STRING && v2.type == STRING) {
    Value v(strcmp(v1.value.string, v2.value.string) != 0);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber != v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber != (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number != v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number != v2.ToInt());
    return v;
  }
}
Value operator<(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.value.number < (int16_t)strlen(v2.value.string));
      return v;
    } else {
      Value v((int16_t)strlen(v1.value.string) < v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber < v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber < (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number < v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number < v2.ToInt());
    return v;
  }
}
Value operator<=(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.value.number <= (int16_t)strlen(v2.value.string));
      return v;
    } else {
      Value v((int16_t)strlen(v1.value.string) <= v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber <= v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber <= (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number <= v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number <= v2.ToInt());
    return v;
  }
}
Value operator>(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.value.number > (int16_t)strlen(v2.value.string));
      return v;
    } else {
      Value v((int16_t)strlen(v1.value.string) > v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber > v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber > (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number > v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number > v2.ToInt());
    return v;
  }
}
Value operator>=(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.value.number >= (int16_t)strlen(v2.value.string));
      return v;
    } else {
      Value v((int16_t)strlen(v1.value.string) >= v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.value.floatNumber >= v2.value.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.value.floatNumber >= (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.value.number >= v2.value.floatNumber);
      return v;
    }
  } else {
    Value v(v1.value.number >= v2.ToInt());
    return v;
  }
}

// unary operators
Value operator+(const Value &v1) {
  if (v1.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT) {
    Value v(+v1.value.floatNumber);
    return v;
  } else {
    Value v(+v1.value.number);
    return v;
  }
}
Value operator-(const Value &v1) {
  if (v1.type == STRING) {
    Value v(v1.value.string);
    return v;
  } else if (v1.type == FLOAT) {
    Value v(-v1.value.floatNumber);
    return v;
  } else {
    Value v(-v1.value.number);
    return v;
  }
}

bool Value::ToBoolean() {
  if (type == STRING) {
    return true;
  } else if (type == BOOLEAN) {
    return value.bval;
  } else if (type == FLOAT) {
    return value.floatNumber != 0;
  } else {
    return value.number != 0;
  }
}

int Value::ToInt() const { return value.number; }

char *Value::ToString() {
  if (type == STRING) {
    // pointless, but we're gonna be calling free for the others, so here too
    char *tempStr = (char *)malloc((sizeof(char) * (strlen(value.string) + 1)));
    strcpy(tempStr, value.string);
    return tempStr;
  } else if (type == BOOLEAN) {
    if (value.bval) {
      char *tempStr = (char *)malloc((sizeof(char) * 5));
      strcpy(tempStr, "TRUE");
      return tempStr;
    } else {
      char *tempStr = (char *)malloc((sizeof(char) * 6));
      strcpy(tempStr, "FALSE");
      return tempStr;
    }
  } else if (type == FLOAT) {
    int len = snprintf(NULL, 0, "%lf", value.floatNumber);
    char *tempStr = (char *)malloc(sizeof(char) * (len + 1));
    snprintf(tempStr, len + 1, "%lf", value.floatNumber);
    return tempStr;
  } else {
    int len = snprintf(NULL, 0, "%i", value.number);
    char *tempStr = (char *)malloc(sizeof(char) * (len + 1));
    snprintf(tempStr, len + 1, "%i", value.number);
    return tempStr;
  }
}

Value Value::index_array(int index) {
  if (isArray) {
    if (type == INTEGER) {
      return Value(((int *)value.intArray)[index]);
    } else if (type == FLOAT) {
      return Value(((double *)value.dblArray)[index]);
    }
  }

  return Value(-1);
}

void Value::update_array(int index, int val) {
  if (isArray) {
    ((int *)value.intArray)[index] = val;
  }
}

void Value::update_array(int index, double val) {
  if (isArray) {
    ((double *)value.dblArray)[index] = val;
  }
}