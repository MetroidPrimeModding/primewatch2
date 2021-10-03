#include <iostream>
#include <chrono>
#include <implot.h>
#include <defs/GameDefinitions.hpp>
#include <defs/GameObject.hpp>

#include "PrimeWatch.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "MemoryAccess.hpp"
#include "GameMemory.h"
#include "prime1/CStateManager.hpp"

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

  GameDefinitions::loadDefinitionsFromPath("prime1.json");

  // This won't copy anything, but will init the memory buffer
  GameMemory::updateFromDolphin();

  // Load the current list of PIDs
  pids = MemoryAccess::getDolphinPids();

  if (pids.size() == 1) {
    MemoryAccess::attachToProcess(pids[0]);
  }

  return 0;
}

void PrimeWatch::initGlAndImgui(const int width, const int height) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
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
    auto start = std::chrono::high_resolution_clock::now();
    processInput();
    doFrame();
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(end - start).count();

    for (int i = 1; i < frameTimes.size(); i++) {
      frameTimes[i - 1] = frameTimes[i];
    }
    frameTimes[frameTimes.size() - 1] = ms;

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void PrimeWatch::shutdown() {
  std::cout << "goodbye" << std::endl;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();

}

void PrimeWatch::processInput() {

}

void PrimeWatch::doFrame() {
  // watch
  doMemoryParse();

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

void PrimeWatch::doImGui() {
  ImGui::ShowDemoWindow(nullptr);
  ImPlot::ShowDemoWindow(nullptr);
//  ImPlot::ShowMetricsWindow(nullptr);
//  ImGui::ShowMetricsWindow(nullptr);

//  if (ImGui::Begin("Frame time")) {
//    if (ImPlot::BeginPlot(
//        "frametime",
//        "ms", nullptr,
//        ImVec2(-1, 0),
//        ImPlotFlags_None,
//        ImPlotAxisFlags_AutoFit,
//        ImPlotAxisFlags_AutoFit
//    )) {
//      ImPlot::PlotLine("Lines", frameTimes.data(), frameTimes.size());
//      ImPlot::EndPlot();
//    }
//  }
//    ImGui::End();

  // Process window
  {
    char buff[32];
    int pid = MemoryAccess::getAttachedPid();
    if (pid > 0) {
      snprintf(buff, sizeof(buff), "Attached (%d)###Processes", pid);
    } else {
      snprintf(buff, sizeof(buff), "Detatched###Processes");
    }
    if (ImGui::Begin(buff)) {
      if (MemoryAccess::getAttachedPid() > 0) {
        ImGui::Text("Attached to process %d", MemoryAccess::getAttachedPid());
      } else {
        ImGui::Text("Not currently attached to process");
      }

      if (ImGui::Button("Refresh")) {
        pids = MemoryAccess::getDolphinPids();
      }
      ImGui::SameLine();
      if (ImGui::Button("Attach")) {
        if (selectedPidIndex < pids.size()) {
          int pid = pids[selectedPidIndex];
          MemoryAccess::attachToProcess(pid);
        }
      }

      ImGui::PushID("PID box");
      if (ImGui::BeginListBox("")) {
        for (int i = 0; i < pids.size(); i++) {
          int pid = pids[i];
          char buff[32];
          snprintf(buff, sizeof(buff), "PID: %d", pid);
          ImGui::Selectable(buff, i == selectedPidIndex);
        }
        ImGui::EndListBox();
      }
      ImGui::PopID();
    }
    ImGui::End();
  }


  if (ImGui::Begin("Player")) {
    CStateManager stateManager(CStateManager::LOCATION);
    CPlayer player = stateManager.player.deref();
    if (player.ptr() != 0) {
      player.doGui();
    }
  }
  ImGui::End();

  if (ImGui::Begin("CStateManager")) {
    GameObject stateManager("g_stateManager", GameDefinitions::structs_by_name["CStateManager"], CStateManager::LOCATION);
    stateManager.renderGui(false);
  }
  ImGui::End();

}

void PrimeWatch::doMemoryParse() {
  GameMemory::updateFromDolphin();
}
