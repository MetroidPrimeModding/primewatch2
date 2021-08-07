#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

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

  void initGlAndImgui(int width, int height);
  static void framebuffer_size_cb(GLFWwindow *window, int width, int height);
  void doFrame();
  void processInput();
  void doImGui();
};


