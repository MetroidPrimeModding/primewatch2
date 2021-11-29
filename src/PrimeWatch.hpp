#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <set>

#include "imgui.h"
#include "imgui_memory_editor.h"
#include "world/WorldRenderer.hpp"
#include "PrimeWatchInput.hpp"

struct WatchedEditorId {
  uint32_t eid;
  uint16_t lastKnownUid;
  std::string type;
};

class PrimeWatch {
public:
  PrimeWatch();


  int initAndCreateWindow();

  void mainLoop();
  void shutdown();
private:
  bool initialized{false};
  GLFWwindow *window;
  WorldRenderer worldRenderer;
  PrimeWatchInput input;

  // imgui stuff
  MemoryEditor mem_edit;
  ImGuiTextFilter objectFilter;

  // imgui state controlling lists
  std::vector<int> pids{};
  std::vector<WatchedEditorId> editorIdsToWatch;
  bool showActiveInTableOnly{true};
  uint16_t tableHoveredUid{0xFFFF};
  bool showRawDataView{false};
  bool showDemoView{false};

  // caching the most recent list of entities
  std::vector<GameDefinitions::GameMember> entities;

  void initGlAndImgui(int width, int height);
  static void framebuffer_size_cb(GLFWwindow *window, int width, int height);

  void doFrame();
  void processInput();
  void doImGui();
  void doMemoryParse();
  void doMainMenu();
  void drawObjectsWindow();
  void loadDefs() const;
};


