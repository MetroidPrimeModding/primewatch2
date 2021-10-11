#include <iostream>
#include <chrono>
#include <implot.h>
#include <defs/GameDefinitions.hpp>
#include <defs/GameObjectRenderers.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <defs/GameOffsets.hpp>

#include "PrimeWatch.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "MemoryAccess.hpp"
#include "GameMemory.h"

using namespace GameDefinitions;
using namespace std;

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
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);
  glfwSetKeyCallback(window, key_cb);

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
  worldRenderer.aspect = (float)width / (float) height;

  mem_edit.ReadOnly = true;

  // TODO: probably remove this
  if (std::filesystem::exists("../mem1.raw")) {
    GameMemory::loadFromPath("../mem1.raw");
  }
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

  // world render
  worldRenderer.update(input);
  worldRenderer.render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PrimeWatch::doImGui() {
  ImGui::ShowDemoWindow(nullptr);
  ImPlot::ShowDemoWindow(nullptr);
//  ImPlot::ShowMetricsWindow(nullptr);
//  ImGui::ShowMetricsWindow(nullptr);

  doMainMenu();

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

  if (ImGuiFileDialog::Instance()->Display("ChooseMemoryDump")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      MemoryAccess::detachFromProcess();
      GameMemory::loadFromPath(filePathName);
    }
    ImGuiFileDialog::Instance()->Close();
  }

  if (ImGui::Begin("CStateManager")) {
    GameMember stateManager{.name="g_stateManager", .type="CStateManager", .offset=CStateManager_ADDRESS};
    GameObjectRenderers::render(stateManager, false);
  }
  ImGui::End();

  if (ImGui::Begin("Areas")) {
    GameMember stateManager{.name="g_stateManager", .type="CStateManager", .offset=CStateManager_ADDRESS};
    auto world = stateManager.memberByName("world");
    if (!world) goto areaEnd;
//    auto areas = world->memberByName("areas");
//    if (!areas) goto areaEnd;
    GameObjectRenderers::render(*world, false);
  }
  areaEnd: ImGui::End();

  mem_edit.DrawWindow("Raw view", GameMemory::memory.data(), GameMemory::memory.size());

  worldRenderer.renderImGui();
}

void PrimeWatch::doMainMenu() {
  if (ImGui::BeginMainMenuBar()) {
    string attachMenuTitle;
    int attachedPid = MemoryAccess::getAttachedPid();
    if (attachedPid > 0) {
      attachMenuTitle = fmt::format("Attached ({})###Processes", attachedPid);
    } else {
      attachMenuTitle = fmt::format("Detatched###Processes");
    }
    if (ImGui::BeginMenu(attachMenuTitle.c_str())) {
      if (ImGui::BeginMenu("Attach")) {
        if (ImGui::MenuItem("Refresh")) {
          pids = MemoryAccess::getDolphinPids();
        }
        ImGui::Separator();
        for (auto pid: pids) {
          if (ImGui::MenuItem(fmt::format("{}", pid).c_str())) {
            MemoryAccess::attachToProcess(pid);
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Detatch", nullptr, false, attachedPid != 0)) {
        MemoryAccess::detachFromProcess();
      }
      if (ImGui::MenuItem("Load from file")) {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseMemoryDump", "Choose Memory Dump", ".raw", ".");
      }

      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void PrimeWatch::doMemoryParse() {
  GameMemory::updateFromDolphin();
}

void PrimeWatch::framebuffer_size_cb(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  PrimeWatch *ptr = (PrimeWatch*)glfwGetWindowUserPointer(window);
  ptr->worldRenderer.aspect = (float)width / (float) height;
}

void PrimeWatch::key_cb(GLFWwindow *window, int key, int scancode, int action, int mods) {
  PrimeWatch *ptr = (PrimeWatch*)glfwGetWindowUserPointer(window);
  PrimeWatchInput &input = ptr->input;

  switch (key) {
    case GLFW_KEY_UP:
      input.camUp = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_DOWN:
      input.camDown = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_LEFT:
      input.camLeft = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_RIGHT:
      input.camRight = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_PAGE_UP:
      input.camIn = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_PAGE_DOWN:
      input.camOut = action != GLFW_RELEASE;
      break;
    default:
      // don't care
      break;
  }
}
