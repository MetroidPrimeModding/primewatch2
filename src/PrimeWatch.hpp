#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class PrimeWatch {
public:
  PrimeWatch();


  int initAndCreateWindow();

  void mainLoop();
  void shutdown();
private:
  bool initialized{false};
  GLFWwindow *window;

  void initGlAndImgui(int width, int height);
  static void framebuffer_size_cb(GLFWwindow *window, int width, int height);
  void doFrame();
  void processInput();
  void doImGui() const;
};


