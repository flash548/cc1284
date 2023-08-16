#include "GenericValue.h"

#ifndef __INTEGER_H__
#define __INTEGER_H__

class Integer : public GenericValue {

public:
  int intNumber;
  Integer(int i);
};

#endif