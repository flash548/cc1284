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
// constructor for a INT VARIABLE
Value::Value(int i) {
  value = Integer(i);
  type = INTEGER;
}
// constructor for a CHAR/INT variable
Value::Value(char i) {
  value = Integer((int)i);
  type = INTEGER;
}
// constructor for a STRING variable
Value::Value(const char *s) {
  // int len = strlen(s);
  // char *sAry = (char *)malloc((len * sizeof(char)) + 1);
  // strcpy(sAry, s);
  // sAry[len] = '\0';

  // item = sAry;
  strcpy(str, s);
  type = STRING;
}
// constructor for a BOOLEAN variable
Value::Value(bool b) {
  value = Boolean(b);
  type = BOOLEAN;
}
// constructor for a FLOAT variable
Value::Value(double b) {
  value = Double(b);
  type = FLOAT;
}
// constructor for a ARRAY variable of TYPE
Value::Value(TYPE t, int size) {
  type = t;
  isArray = true;
  arraySize = size;
  if (t == INTEGER) {
    int *ptr = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
      ptr[i] = 0;
    }
    item = ptr;
  } else if (t == FLOAT) {
    double *ptr = (double *)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
      ptr[i] = 0.0f;
    }
    item = ptr;
  }
}

Value operator+(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      char tempIntStr[MAXSTRLENGTH];
      sprintf(tempIntStr, "%i", *((int *)v1.value.valuePtr));
      char tmp[strlen(tempIntStr) + strlen((char *)v2.value.valuePtr) + 1];
      strcpy(tmp, tempIntStr);
      strcat(tmp, (char *)v2.value.valuePtr);
      Value v(tmp);
      return v;
    } else if (v1.type == FLOAT) {
      char tempFloatStr[MAXSTRLENGTH];
      sprintf(tempFloatStr, "%lf", *((double *)v1.value.valuePtr));
      char tmp[strlen(tempFloatStr) + strlen((char *)v2.value.valuePtr) + 1];
      strcpy(tmp, tempFloatStr);
      strcat(tmp, (char *)v2.value.valuePtr);
      Value v(tmp);
      return v;
    } else if (v2.type == INTEGER) {
      char tempIntStr[MAXSTRLENGTH];
      sprintf(tempIntStr, "%i", *((int *)v2.value.valuePtr));
      char tmp[strlen(tempIntStr) + strlen((char *)v1.value.valuePtr) + 1];
      strcpy(tmp, (char *)v1.value.valuePtr);
      strcat(tmp, tempIntStr);
      Value v(tmp);
      return v;
    } else if (v2.type == FLOAT) {
      char tempFloatStr[MAXSTRLENGTH];
      sprintf(tempFloatStr, "%lf", *((double *)v2.value.valuePtr));
      char tmp[strlen(tempFloatStr) + strlen((char *)v1.value.valuePtr) + 1];
      strcat(tmp, (char *)v1.value.valuePtr);
      strcpy(tmp, tempFloatStr);
      Value v(tmp);
      return v;
    } else {
      // string to string concat
      char tmp[strlen((char *)v1.value.valuePtr) +
               strlen((char *)v2.value.valuePtr) + 1];
      strcpy(tmp, (char *)v1.value.valuePtr);
      strcat(tmp, (char *)v2.value.valuePtr);
      Value v(tmp);
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber + v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber + (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() + v2.floatNumber);
      return v;
    }
  } else {
    Value v(*((int *)v1.value.valuePtr) + *((int *)v2.value.valuePtr));
    return v;
  }
}

Value operator-(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber - v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber - (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() - v2.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.ToInt() - v2.ToInt());
  return v;
}

Value operator*(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber * v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber * (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() * v2.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.ToInt() * v2.ToInt());
  return v;
}

Value operator/(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber / v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber / (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() / v2.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.ToInt() / v2.ToInt());
  return v;
}

Value operator%(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.floatNumber % (int)v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.floatNumber % v2.ToInt());
      return v;
    } else {
      Value v(v1.ToInt() % (int)v2.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.ToInt() % v2.ToInt());
  return v;
}

Value operator<<(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.floatNumber << (int)v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.floatNumber << v2.ToInt());
      return v;
    } else {
      Value v(v1.ToInt() << (int)v2.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.ToInt() << v2.ToInt());
  return v;
}

Value operator>>(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.floatNumber >> (int)v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.floatNumber >> v2.ToInt());
      return v;
    } else {
      Value v(v1.ToInt() >> (int)v2.floatNumber);
      return v;
    }
  }

  // must be both INTs
  Value v(v1.ToInt() >> v2.ToInt());
  return v;
}

Value operator&(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.bval && v2.bval);
    return v;
  } else if (v1.type == STRING || v2.type == STRING) {
    char tmp[MAXSTRLENGTH];
    if (v1.type == INTEGER) {
      sprintf(tmp, "%i", v1.ToInt());
      strcpy(tmp, v1.str);
      strcat(tmp, v2.str);
      Value v(tmp);
      return v;
    } else if (v1.type == FLOAT) {
      sprintf(tmp, "%lf", v1.floatNumber);
      strcpy(tmp, v1.str);
      strcat(tmp, v2.str);
      Value v(tmp);
      return v;
    } else if (v2.type == INTEGER) {
      char tmp2[MAXSTRLENGTH];
      sprintf(tmp2, "%i", v2.ToInt());
      strcpy(tmp, v1.str);
      strcat(tmp, tmp2);
      Value v(tmp);
      return v;
    } else if (v2.type == FLOAT) {
      char tmp2[MAXSTRLENGTH];
      sprintf(tmp2, "%lf", v2.floatNumber);
      strcpy(tmp, v1.str);
      strcat(tmp, tmp2);
      Value v(tmp);
      return v;
    } else {
      strcpy(tmp, v1.str);
      strcat(tmp, v2.str);
      Value v(tmp);
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.floatNumber & (int)v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.floatNumber & v2.ToInt());
      return v;
    } else {
      Value v(v1.ToInt() & (int)v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() & v2.ToInt());
    return v;
  }
}
Value operator|(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.bval || v2.bval);
    return v;
  } else if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.floatNumber | (int)v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.floatNumber | v2.ToInt());
      return v;
    } else {
      Value v(v1.ToInt() | (int)v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() | v2.ToInt());
    return v;
  }
}
Value operator!(const Value &v1) {
  if (v1.type == BOOLEAN) {
    Value v(!v1.bval);
    return v;
  } else if (v1.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT) {
    Value v(~(int)v1.floatNumber);
    return v;
  } else {
    Value v(~v1.ToInt());
    return v;
  }
}
Value operator^(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.bval ^ v2.bval);
    return v;
  } else if (v1.type == STRING || v2.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v((int)v1.floatNumber ^ (int)v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v((int)v1.floatNumber ^ v2.ToInt());
      return v;
    } else {
      Value v(v1.ToInt() ^ (int)v2.floatNumber);
      return v;
    }
  } else {
    Value v((int)v1.ToInt() ^ (int)v2.ToInt());
    return v;
  }
}
Value operator==(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.bval == v2.bval);
    return v;
  } else if (v1.type == STRING && v2.type == STRING) {
    Value v(strcmp((char *)v1.value.valuePtr, (char *)v2.value.valuePtr) == 0);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber == v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber == (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() == v2.floatNumber);
      return v;
    }
  } else {
    // integer compare?
    printf("%i, %i\n", v1.ToInt(), v2.ToInt());
    Value v(*((int *)v1.value.valuePtr) == *((int *)v2.value.valuePtr));
    return v;
  }
}
Value operator!=(const Value &v1, const Value &v2) {
  if (v1.type == BOOLEAN || v2.type == BOOLEAN) {
    Value v(v1.bval != v2.bval);
    return v;
  } else if (v1.type == STRING && v2.type == STRING) {
    Value v(strcmp(v1.str, v2.str) != 0);
    return v;
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber != v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber != (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() != v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() != v2.ToInt());
    return v;
  }
}
Value operator<(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.ToInt() < (int16_t)strlen(v2.str));
      return v;
    } else {
      Value v((int16_t)strlen(v1.str) < v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber < v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber < (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() < v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() < v2.ToInt());
    return v;
  }
}
Value operator<=(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.ToInt() <= (int16_t)strlen(v2.str));
      return v;
    } else {
      Value v((int16_t)strlen(v1.str) <= v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber <= v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber <= (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() <= v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() <= v2.ToInt());
    return v;
  }
}
Value operator>(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.ToInt() > (int16_t)strlen(v2.str));
      return v;
    } else {
      Value v((int16_t)strlen(v1.str) > v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber > v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber > (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() > v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() > v2.ToInt());
    return v;
  }
}
Value operator>=(const Value &v1, const Value &v2) {
  if (v1.type == STRING || v2.type == STRING) {
    if (v1.type == INTEGER) {
      Value v(v1.ToInt() >= (int16_t)strlen(v2.str));
      return v;
    } else {
      Value v((int16_t)strlen(v1.str) >= v2.ToInt());
      return v;
    }
  } else if (v1.type == FLOAT && v2.type == FLOAT) {
    Value v(v1.floatNumber >= v2.floatNumber);
    return v;
  } else if (v1.type == FLOAT || v2.type == FLOAT) {
    if (v1.type == FLOAT) {
      Value v(v1.floatNumber >= (float)v2.ToInt());
      return v;
    } else {
      Value v((float)v1.ToInt() >= v2.floatNumber);
      return v;
    }
  } else {
    Value v(v1.ToInt() >= v2.ToInt());
    return v;
  }
}

// unary operators
Value operator+(const Value &v1) {
  if (v1.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT) {
    Value v(+v1.floatNumber);
    return v;
  } else {
    Value v(+v1.ToInt());
    return v;
  }
}
Value operator-(const Value &v1) {
  if (v1.type == STRING) {
    Value v(v1.str);
    return v;
  } else if (v1.type == FLOAT) {
    Value v(-v1.floatNumber);
    return v;
  } else {
    Value v(-v1.ToInt());
    return v;
  }
}

bool Value::ToBoolean() {
  if (type == STRING) {
    return true;
  } else if (type == BOOLEAN) {
    return bval;
  } else if (type == FLOAT) {
    return floatNumber != 0;
  } else {
    return *((int *)value.valuePtr) != 0;
  }
}

int Value::ToInt() const { return *((int *)value.valuePtr); }

char *Value::ToString() {
  if (type == STRING)
    return str;
  else if (type == BOOLEAN) {
    if (bval)
      return "TRUE";
    else
      return "FALSE";
  } else if (type == FLOAT) {
    sprintf(str, "%lf", floatNumber);
    return str;
  } else {
    sprintf(str, "%i", *((int *)value.valuePtr));
    return str;
  }
}

Value Value::index_array(int index) {
  if (isArray) {
    if (type == INTEGER) {
      return Value(((int *)item)[index]);
    } else if (type == FLOAT) {
      return Value(((double *)item)[index]);
    }
  }

  return Value(-1);
}

void Value::update_array(int index, int val) {
  if (isArray) {
    ((int *)item)[index] = val;
  }
}

void Value::update_array(int index, double val) {
  if (isArray) {
    ((double *)item)[index] = val;
  }
}