#include "Double.h"

Double::Double(double num) : GenericValue() {
  dblNumber = num;
  valuePtr = &dblNumber;
}