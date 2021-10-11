#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "world/WorldRenderer.hpp"
#include "PrimeWatchInput.hpp"

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
  MemoryEditor mem_edit;
  WorldRenderer worldRenderer;
  PrimeWatchInput input;

  std::array<float, 120> frameTimes;

  void initGlAndImgui(int width, int height);
  static void framebuffer_size_cb(GLFWwindow *window, int width, int height);
  static void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);

  void doFrame();
  void processInput();
  void doImGui();
  void doMemoryParse();
  void doMainMenu();
};


