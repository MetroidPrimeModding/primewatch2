#include <iostream>

#include "PrimeWatch.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


PrimeWatch::PrimeWatch() {

}

int PrimeWatch::initAndCreateWindow() {
  if (initialized) {
    return 0;
  }
  initialized = true;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  constexpr int width = 1200;
  constexpr int height = 800;

  window = glfwCreateWindow(width, height, "Prime Watch 2", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to init GLAD" << std::endl;
    return 2;
  }

  initGlAndImgui(width, height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);

  return 0;
}

void PrimeWatch::initGlAndImgui(const int width, const int height) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
//  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 150");
  io.Fonts->AddFontDefault();

  glViewport(0, 0, width, height);
}

void PrimeWatch::framebuffer_size_cb(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void PrimeWatch::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    processInput();
    doFrame();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void PrimeWatch::shutdown() {
  std::cout << "goodbye" << std::endl;
  glfwTerminate();
}

void PrimeWatch::processInput() {

}

void PrimeWatch::doFrame() {
  // imgui
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  doImGui();

  ImGui::Render();

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PrimeWatch::doImGui() const { ImGui::ShowDemoWindow(nullptr); }
