#pragma once

enum RunMode {
  OnlyFnGen,
  FnGenAndOpt,
  FullJIT
};

int jitApp(RunMode mode);
