#include "GenericValue.h"

#ifndef __Boolean_H__
#define __Boolean_H__

class Boolean : public GenericValue {

public:
  bool boolVal;
  Boolean(bool i);
};

#endif