#include <stdio.h>
#include <stdlib.h>
#include "Value.h"

class AST {

  public:
    AST() { printf("AST\n"); };
    void visit() { printf ("No implemented\n"); }
    ~AST() {};
};

class RootNode: public AST {

  public:
    RootNode() { printf("RootNode\n"); } 
    ~RootNode() {}

};

class ValueNode: public AST {

  public:
    Value m_val;
    ValueNode(Value& val) {
      m_val = val;
    }
    void visit() {
      printf("Value visit\n");
    }
};





int main() {
  Value v(4);
  ValueNode node(v);
  node.visit();
  exit(0);
}
