#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include <array>

class PrimeWatch {
public:
  PrimeWatch();


  int initAndCreateWindow();

  void mainLoop();
  void shutdown();
private:
  bool initialized{false};
  GLFWwindow *window;

  std::vector<int> pids{};
  int selectedPidIndex{0};

  std::array<float, 120> frameTimes;

  void initBgfxAndImgui(int width, int height);
  static void framebuffer_size_cb(GLFWwindow *window, int width, int height);
  void doFrame();
  void processInput();
  void doImGui();
  void doMemoryParse();
};


