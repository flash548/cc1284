#include "Boolean.h"

Boolean::Boolean(bool v) : GenericValue() {
  boolVal = v;
  valuePtr = &boolVal;
}