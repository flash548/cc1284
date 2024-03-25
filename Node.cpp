#include "Node.h"
#include <stdio.h>

/// @brief
Node::Node() {}
Node Node::visit() {
  printf("%s\n", "base impl");
  return Node();
}

/// @brief
/// @param op1
/// @param op2
/// @param op
BinOpNode::BinOpNode(Value op1, Value op2, BinOperation op) : Node() {
  _op1 = op1;
  _op2 = op2;
  _op = op;
}
Node BinOpNode::visit() {
  switch (_op) {
  case BINOP_ADD:
    return ValueNode(Value(_op1 + _op2)).visit();
  case BINOP_SUBTRACT:
    return ValueNode(Value(_op1 - _op2)).visit();
  case BINOP_MULTIPLY:
    return ValueNode(Value(_op1 * _op2)).visit();
  case BINOP_DIVIDE:
    return ValueNode(Value(_op1 / _op2)).visit();
  case BINOP_MODULO:
    return ValueNode(Value(_op1 % _op2)).visit();
  case BINOP_LSHIFT:
    return ValueNode(Value(_op1 << _op2)).visit();
  case BINOP_RSHIFT:
    return ValueNode(Value(_op1 >> _op2)).visit();
  case BINOP_AND:
    return ValueNode(Value(_op1 & _op2)).visit();
  case BINOP_OR:
    return ValueNode(Value(_op1 | _op2)).visit();
  case BINOP_XOR:
    return ValueNode(Value(_op1 ^ _op2)).visit();
  case BINOP_ISEQUAL:
    return ValueNode(Value(_op1 == _op2)).visit();
  case BINOP_NOTEQUAL:
    return ValueNode(Value(_op1 != _op2)).visit();
  case BINOP_GT:
    return ValueNode(Value(_op1 > _op2)).visit();
  case BINOP_GTE:
    return ValueNode(Value(_op1 >= _op2)).visit();
  case BINOP_LT:
    return ValueNode(Value(_op1 < _op2)).visit();
  case BINOP_LTE:
    return ValueNode(Value(_op1 <= _op2)).visit();
  case BINOP_INTEGER_DIV:
    return ValueNode(_op1.intDiv(_op2)).visit();
  default:
    break;
  }
  return Node();
}

UnOpNode::UnOpNode(Value op1, UnaryOperation op) {
  _op1 = op1;
  _op = op;
}
Node UnOpNode::visit() {
  switch (_op) {
  case UNOP_NEGATE:
    return ValueNode(Value(-_op1)).visit();
  case UNOP_NOT:
    return ValueNode(Value(!_op1)).visit();
  case UNOP_POSITIVE:
    return ValueNode(Value(+_op1)).visit();
  default:
    break;
  }
  return Node();
}

ValueNode::ValueNode(Value val) { _val = val; }
Node ValueNode::visit() {
  printf("%s\n", _val.ToString());
  return Node();
}

/// @brief
/// @return
int main() {
  BinOpNode binOpNode(Value(20), Value(3), BINOP_INTEGER_DIV);
  binOpNode.visit();
  return 0;
}