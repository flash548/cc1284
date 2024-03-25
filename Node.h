#include "Value.h"

#ifndef __NODE_H__
#define __NODE_H__

enum BinOperation {
  BINOP_ADD = 0,
  BINOP_SUBTRACT = 1,
  BINOP_MULTIPLY,
  BINOP_DIVIDE,
  BINOP_MODULO,
  BINOP_LSHIFT,
  BINOP_RSHIFT,
  BINOP_AND,
  BINOP_OR,
  BINOP_XOR,
  BINOP_ISEQUAL,
  BINOP_NOTEQUAL,
  BINOP_GT,
  BINOP_GTE,
  BINOP_LT,
  BINOP_LTE,
  BINOP_INTEGER_DIV
};

/// @brief
class Node {
public:
  Node();
  virtual Node visit();
};

/// @brief
class RootNode : public Node {
public:
  RootNode();
};

/// @brief
class BinOpNode : public Node {
private:
  BinOperation _op;
  Value _op1;
  Value _op2;

public:
  BinOpNode(Value op1, Value op2, BinOperation op);
  Node visit();
};

/// @brief
class ValueNode : public Node {
private:
  Value _val;

public:
  ValueNode(Value val);
  Node visit();
};

#endif