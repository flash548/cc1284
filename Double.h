#include "GenericValue.h"

#ifndef __DOUBLE_H__
#define __DOUBLE_H__

class Double : public GenericValue {

public:
  double dblNumber;
  Double(double i);
};

#endif