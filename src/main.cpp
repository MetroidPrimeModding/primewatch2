#include <iostream>

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