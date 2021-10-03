#include <iostream>
#include <chrono>
#include <implot.h>

#include "PrimeWatch.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#define GLFW_EXPOSE_NATIVE_X11
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui/imgui_impl_bgfx.h"
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
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  constexpr int width = 1200;
  constexpr int height = 800;

  window = glfwCreateWindow(width, height, "Prime Watch 2", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  initBgfxAndImgui(width, height);
//  glfwSetFramebufferSizeCallback(window, framebuffer_size_cb);

  // This won't copy anything, but will init the memory buffer
  GameMemory::updateFromDolphin();

  // Load the current list of PIDs
  pids = MemoryAccess::getDolphinPids();

  if (pids.size() == 1) {
    MemoryAccess::attachToProcess(pids[0]);
  }

  return 0;
}

void PrimeWatch::initBgfxAndImgui(int width, int height) {
  bgfx::Init bgfx_init;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
  bgfx_init.platformData.ndt = glfwGetX11Display();
  bgfx_init.platformData.nwh = (void *) (uintptr_t) glfwGetX11Window(window);
#elif BX_PLATFORM_OSX
  init.platformData.nwh = glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
  init.platformData.nwh = glfwGetWin32Window(window);
#endif
  bgfx_init.type = bgfx::RendererType::Count; // auto choose renderer
  bgfx_init.resolution.width = width;
  bgfx_init.resolution.height = height;
  bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
  bgfx::init(bgfx_init);

  bgfx::setViewClear(
      0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);
  bgfx::setViewRect(0, 0, 0, width, height);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplGlfw_InitForOther(window, true);
  ImGui_Implbgfx_Init(255);
  io.Fonts->AddFontDefault();

}

void PrimeWatch::framebuffer_size_cb(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void PrimeWatch::mainLoop() {
  while (!glfwWindowShouldClose(window)) {
    auto start = std::chrono::high_resolution_clock::now();
    glfwPollEvents();
    processInput();
    bgfx::touch(0);
    doFrame();
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(end - start).count();

    for (int i = 1; i < frameTimes.size(); i++) {
      frameTimes[i - 1] = frameTimes[i];
    }
    frameTimes[frameTimes.size() - 1] = ms;

    bgfx::frame();
  }
}

void PrimeWatch::shutdown() {
  std::cout << "goodbye" << std::endl;
  ImGui_Implbgfx_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  bgfx::shutdown();

  glfwTerminate();

}

void PrimeWatch::processInput() {

}

void PrimeWatch::doFrame() {
  // watch
  doMemoryParse();

  // imgui
  ImGui_Implbgfx_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  doImGui();

  ImGui::Render();

  ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
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

  char buff[32];
  int pid = MemoryAccess::getAttachedPid();
  if (pid > 0) {
    snprintf(buff, sizeof(buff), "Attached (%d)###Processes", pid);
  } else {
    snprintf(buff, sizeof(buff), "Detatched###Processes", pid);
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


  if (ImGui::Begin("Player")) {
    CStateManager stateManager(CStateManager::LOCATION);
    CPlayer player = stateManager.player.deref();
    if (player.ptr() != 0) {
      player.doGui();
    }
  }
  ImGui::End();

}

void PrimeWatch::doMemoryParse() {
  GameMemory::updateFromDolphin();
}
