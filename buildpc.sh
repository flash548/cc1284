#! sh
clang++ -w -UAVR_TARGET -c main.cpp Value.cpp Interpreter.cpp && clang++ *.o -o a.out \
  && mv a.out basic && chmod +x basic && rm *.o
