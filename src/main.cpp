#include <iostream>

#include <cstdio>
#include <GLFW/glfw3.h>
#include "PrimeWatch.hpp"

int main() {
  PrimeWatch *main = new PrimeWatch();

  int res = main->initAndCreateWindow();
  if (res != 0) {
    return res;
  }

  main->mainLoop();

  main->shutdown();
  return 0;
}