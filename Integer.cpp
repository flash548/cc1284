#include "Integer.h"

Integer::Integer(int num) : GenericValue() {
  intNumber = num;
  valuePtr = &intNumber;
}