#include "../Interpreter.h"
#include "../Value.h"
#include "../main.h"
#include <stdio.h>
#include <stdlib.h>

void testCmpNoCase();

void assertZero(int result) {  if (result != 0) exit(1); }
void assertNotZero(int result) { if (result == 0) exit(1); }
void assertFalse(bool result) { if (result) exit(1); }
void assertTrue(bool result) { if(!result) exit(1); }


void (*testFunc[])() = {testCmpNoCase};

int main(int argc, const char *argv[]) {
  for (int i = 0; i < sizeof(testFunc) / sizeof(*testFunc); i++) {
    testFunc[i]();
  }
}

void testCmpNoCase() {
  Interpreter i("");
  assertZero(i.nocase_cmp("print", "PRINT"));
  assertNotZero(i.nocase_cmp("prints", "PRINT"));
  assertNotZero(i.nocase_cmp("PRINT", "✅"));
  printf("✅\n");
}
