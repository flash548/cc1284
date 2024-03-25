#include "Node.h"
#include <stdio.h>

/// @brief
Node::Node() {}
Node Node::visit() {
  printf("%s\n", "base impl");
  return Node();
}

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
  BinOpNode binOpNode(Value("hello"), Value("world"), BINOP_ADD);
  binOpNode.visit();
  Value v(INTEGER, 3);
  v.update_array(0, 1);
  v.update_array(1, 2);
  v.update_array(2, 3);
  BinOpNode binOpNode2(v.index_array(2), Value(2), BINOP_ADD);
  binOpNode2.visit();
  return 0;
}