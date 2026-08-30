#include <stdio.h>
#include <string.h>
#include "JIT/jit.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: jitapp <mode>\n");
    printf("Modes:\n");
    printf("  OnlyFnGen\n");
    printf("  FnGenAndOpt\n");
    printf("  FullJIT\n");
    return -1;
  }
  RunMode mode;
  if (strcmp(argv[1], "OnlyFnGen") == 0) {
    mode = OnlyFnGen;
  } else if (strcmp(argv[1], "FnGenAndOpt") == 0) {
    mode = FnGenAndOpt;
  } else if (strcmp(argv[1], "FullJIT") == 0) {
    mode = FullJIT;
  } else {
    printf("Invalid mode: %s\n", argv[1]);
    return -1;
  }
  return jitApp(mode);
}
