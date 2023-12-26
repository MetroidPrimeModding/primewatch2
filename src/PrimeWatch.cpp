#include "PrimeWatch.hpp"

#include <iostream>
#include <chrono>
#include <implot.h>
#include <filesystem>
#include <fmt/format.h>
#include <set>

#include <defs/GameDefinitions.hpp>
#include <defs/GameVtables.hpp>
#include <defs/GameObjectRenderers.hpp>
#include <defs/GameOffsets.hpp>
#include <utils/GameObjectUtils.hpp>
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

  loadDefs();

  // This won't copy anything, but will init the memory buffer
  GameMemory::updateFromDolphin();

  // Load the current list of PIDs
  pids = MemoryAccess::getDolphinPids();

  if (pids.size() == 1) {
    MemoryAccess::attachToProcess(pids[0]);
  }

  worldRenderer.init();

  return 0;
}

void PrimeWatch::loadDefs() const { loadDefinitionsFromPath("prime_defs/prime1/**/*.bs"); }

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
  worldRenderer.camViewport = glm::vec4{0, 0, width, height};
  worldRenderer.aspect = (float) width / (float) height;

  mem_edit.ReadOnly = false;
  mem_edit.WriteFn = [](ImU8 *data, size_t off, ImU8 d) {
    // do nothing
  };

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
  // Reset
  input.camPitch = input.camYaw = input.camZoom = 0.0f;

  // get new stuff
  ImGuiIO &io = ImGui::GetIO();

  if (!io.WantCaptureMouse) {
    if (io.MouseDown[GLFW_MOUSE_BUTTON_LEFT] || io.MouseDown[GLFW_MOUSE_BUTTON_RIGHT]) {
      input.capturedMouse = true;
    }
  }
  if (!io.MouseDown[GLFW_MOUSE_BUTTON_LEFT] && !io.MouseDown[GLFW_MOUSE_BUTTON_RIGHT]) {
    input.capturedMouse = false;
  }

  if (input.capturedMouse) {
    float pitchSpeed = 0.005;
    float yawSpeed = -0.005;

    ImVec2 delta = io.MouseDelta;
    input.camPitch = delta.y * pitchSpeed;
    input.camYaw = delta.x * yawSpeed;
  }

  if (!io.WantCaptureMouse) {
    float distSpeed = -2;
    input.camZoom = io.MouseWheel * distSpeed;
  }

  if (!io.WantCaptureKeyboard) {
    float pitchSpeed = 0.03;
    float yawSpeed = 0.03;
    float zoomSpeed = 0.5;

    if (io.KeysDown[GLFW_KEY_UP]) {
      input.camPitch += pitchSpeed;
    }
    if (io.KeysDown[GLFW_KEY_DOWN]) {
      input.camPitch -= pitchSpeed;
    }
    if (io.KeysDown[GLFW_KEY_LEFT]) {
      input.camYaw += yawSpeed;
    }
    if (io.KeysDown[GLFW_KEY_RIGHT]) {
      input.camYaw -= yawSpeed;
    }
    if (io.KeysDown[GLFW_KEY_PAGE_UP]) {
      input.camZoom -= zoomSpeed;
    }
    if (io.KeysDown[GLFW_KEY_PAGE_DOWN]) {
      input.camZoom += zoomSpeed;
    }

    if (worldRenderer.cameraMode == CameraMode::DETATCHED) {
      float dist = 1;
      glm::quat angle = glm::quat(glm::vec3(0, 0, worldRenderer.yaw));
      glm::vec3 forward = angle * glm::vec4{dist, 0, 0, 1};
      glm::vec3 right = angle * glm::vec4{0, dist, 0, 1};
      glm::vec3 up{0, 0, dist};
      if (io.KeysDown[GLFW_KEY_W]) {
        worldRenderer.manualCameraPos += forward;
      }
      if (io.KeysDown[GLFW_KEY_S]) {
        worldRenderer.manualCameraPos -= forward;
      }
      if (io.KeysDown[GLFW_KEY_A]) {
        worldRenderer.manualCameraPos += right;
      }
      if (io.KeysDown[GLFW_KEY_D]) {
        worldRenderer.manualCameraPos -= right;
      }
      if (io.KeysDown[GLFW_KEY_Q]) {
        worldRenderer.manualCameraPos -= up;
      }
      if (io.KeysDown[GLFW_KEY_E]) {
        worldRenderer.manualCameraPos += up;
      }
    }
  }
}

void PrimeWatch::doFrame() {
  // watch
  doMemoryParse();

  // imgui
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (GameDefinitions::isLoaded()) {
    doImGui();
  } else {
    if (ImGui::Begin("NOT LOADED", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
      ImGui::Text("Script definitions are not currently loaded.");
      ImGui::Text("These are required to function.");
      ImGui::Text("Current error:");
      ImGui::Text("%s", GameDefinitions::getError().c_str());
      if (ImGui::Button("Reload")) {
        this->loadDefs();
      }
    }
    ImGui::End();
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // world render
  if (GameDefinitions::isLoaded()) {
    worldRenderer.update(input);

    set<uint16_t> highlightedEntities{
        tableHoveredUid
    };
    for (auto &watch: editorIdsToWatch) {
      highlightedEntities.insert(watch.lastKnownUid);
    }
    worldRenderer.render(entities, highlightedEntities);
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PrimeWatch::doImGui() {
  if (showDemoView) {
    ImGui::ShowDemoWindow(&showDemoView);
  }
//  ImPlot::ShowDemoWindow(nullptr);
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

  if (ImGui::Begin("globals")) {
    GameObjectRenderers::render(g_stateManager, true);
    GameObjectRenderers::render(g_main, true);
  }
  ImGui::End();

  if (showExactCameraControls) {
    if (ImGui::Begin("Camera Controls", &showExactCameraControls, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::DragFloat("Camera line length", &worldRenderer.camLineLength, 1.f, 2.f, 250.f, "%0.3f");
      float yawDeg = fmod(glm::degrees(worldRenderer.yaw), 360.f);
      float pitchDeg = glm::degrees(worldRenderer.pitch);
      ImGui::DragFloat3("Position", &worldRenderer.manualCameraPos.x, 1, -FLT_MAX, FLT_MAX, "%0.3f");
      if (ImGui::DragFloat("Yaw", &yawDeg, 1, -360, 360, "%0.3f")) {
        worldRenderer.yaw = glm::radians(yawDeg);
      }
      if (ImGui::DragFloat("Pitch", &pitchDeg, 1, -89, 89, "%0.3f")) {
        worldRenderer.pitch = glm::radians(pitchDeg);
      }
    }
    ImGui::End();
  }

  drawObjectsWindow();

  if (showRawDataView) {
    if (ImGui::Begin("Raw view", &showRawDataView)) {
      mem_edit.DrawContents(GameMemory::memory.data(), GameMemory::memory.size());
    }
    ImGui::End();
  }

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

    if (ImGui::BeginMenu("Culling")) {
      if (ImGui::MenuItem("Show Front", nullptr, worldRenderer.culling == CullType::BACK)) {
        worldRenderer.culling = CullType::BACK;
      }
      if (ImGui::MenuItem("Show Back", nullptr, worldRenderer.culling == CullType::FRONT)) {
        worldRenderer.culling = CullType::FRONT;
      }
      if (ImGui::MenuItem("Show All", nullptr, worldRenderer.culling == CullType::NONE)) {
        worldRenderer.culling = CullType::NONE;
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Camera")) {
      if (ImGui::MenuItem("Follow Player", nullptr, worldRenderer.cameraMode == CameraMode::FOLLOW_PLAYER)) {
        worldRenderer.cameraMode = CameraMode::FOLLOW_PLAYER;
      }
      if (ImGui::MenuItem("Game Cam", nullptr, worldRenderer.cameraMode == CameraMode::GAME_CAM)) {
        worldRenderer.cameraMode = CameraMode::GAME_CAM;
      }
      if (ImGui::MenuItem("Detatched", nullptr, worldRenderer.cameraMode == CameraMode::DETATCHED)) {
        worldRenderer.cameraMode = CameraMode::DETATCHED;
      }
      if (ImGui::MenuItem("Show camera controls", nullptr, showExactCameraControls)) {
        showExactCameraControls = !showExactCameraControls;
      }
      ImGui::EndMenu();
    }

#define TOGGLE_MENU(label, path)  if (ImGui::MenuItem((label), nullptr, (path))) (path) = !(path);
    if (ImGui::BeginMenu("Triggers")) {
      TOGGLE_MENU("detectPlayer", worldRenderer.triggerRenderConfig.detectPlayer)
      TOGGLE_MENU("detectAi", worldRenderer.triggerRenderConfig.detectAi)
      TOGGLE_MENU("detectProjectiles", worldRenderer.triggerRenderConfig.detectProjectiles)
      TOGGLE_MENU("detectBombs", worldRenderer.triggerRenderConfig.detectBombs)
      TOGGLE_MENU("detectPowerBombs", worldRenderer.triggerRenderConfig.detectPowerBombs)
      TOGGLE_MENU("killOnEnter", worldRenderer.triggerRenderConfig.killOnEnter)
      TOGGLE_MENU("detectMorphedPlayer", worldRenderer.triggerRenderConfig.detectMorphedPlayer)
      TOGGLE_MENU("useCollisionImpluses", worldRenderer.triggerRenderConfig.useCollisionImpluses)
      TOGGLE_MENU("detectCamera", worldRenderer.triggerRenderConfig.detectCamera)
      TOGGLE_MENU("useBooleanIntersection", worldRenderer.triggerRenderConfig.useBooleanIntersection)
      TOGGLE_MENU("detectUnmorphedPlayer", worldRenderer.triggerRenderConfig.detectUnmorphedPlayer)
      TOGGLE_MENU("blockEnvironmentalEffects", worldRenderer.triggerRenderConfig.blockEnvironmentalEffects)
      ImGui::Separator();
      TOGGLE_MENU("Water", worldRenderer.triggerRenderConfig.water)
      TOGGLE_MENU("Docks", worldRenderer.triggerRenderConfig.docks)
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Actors")) {
      TOGGLE_MENU("Render projectiles", worldRenderer.actorRenderConfig.renderProjectiles)
      TOGGLE_MENU("Render AI", worldRenderer.actorRenderConfig.renderAI)
      TOGGLE_MENU("Render Pickups", worldRenderer.actorRenderConfig.renderPickups)
      TOGGLE_MENU("Render physics actors", worldRenderer.actorRenderConfig.renderPhysicsActors)
      TOGGLE_MENU("Render actors", worldRenderer.actorRenderConfig.renderActors)
      TOGGLE_MENU("Render all actors", worldRenderer.actorRenderConfig.renderAllActors)

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
      if (ImGui::MenuItem("Reload Definitions")) {
        this->loadDefs();
      }
      if (ImGui::MenuItem("Raw Data View", nullptr, showRawDataView)) {
        showRawDataView = !showRawDataView;
      }
      if (ImGui::MenuItem("Raw Demo View", nullptr, showDemoView)) {
        showDemoView = !showDemoView;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void PrimeWatch::doMemoryParse() {
  if (GameDefinitions::isLoaded()) {
    GameMemory::updateFromDolphin();
    entities = GameObjectUtils::getAllObjects();
  }
}

void PrimeWatch::framebuffer_size_cb(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  PrimeWatch *ptr = (PrimeWatch *) glfwGetWindowUserPointer(window);
  ptr->worldRenderer.aspect = (float) width / (float) height;
  ptr->worldRenderer.camViewport = glm::vec4{0, 0, width, height};
}

struct VtableInfo {
  int active{0};
  int inactive{0};
};

void PrimeWatch::drawObjectsWindow() {
  // TODO: maybe cache this if I start looking at it in multiple places
  map<uint32_t, GameMember> eidToEntity;
  map<uint16_t, GameMember> uidToEntity;

  map<uint32_t, VtableInfo> vtables;
  for (auto &pair: entities) {
    auto &entity = pair.second;
    uint32_t vtable = entity["vtable"].read_u32();
    if (entity["active"].read_bool()) {
      vtables[vtable].active += 1;
    } else {
      vtables[vtable].inactive += 1;
    }
    uint32_t eid = entity["editorID"].read_u32();
    uint32_t uid = entity["uniqueID"].read_u16();
    eidToEntity[eid] = entity;
    uidToEntity[uid] = entity;
  }

  if (ImGui::Begin("Objects")) {
    ImGui::Text("%s", fmt::format("Current object count: {}", entities.size()).c_str());

    static set<uint32_t> unknowns;
    for (auto &vtable: vtables) {
      if (MP1_VTABLES.count(vtable.first) == 0 && vtable.first > 0x80000000u && vtable.first < 0x80700000u) {
        // if it's <80000 it's invalid and just not up to date yet
        unknowns.insert(vtable.first);
      }
    }

    if (ImGui::Button(fmt::format("Copy unknowns ({})", unknowns.size()).c_str())) {
      string clip;
      for (auto &vt: unknowns) {
        clip += fmt::format("{{0x{:08x}, \"\"}},\n", vt);
      }
      ImGui::SetClipboardText(clip.c_str());
    }

    if (ImGui::CollapsingHeader("List of types")) {
      ImGui::Indent();
      if (ImGui::BeginTable(
          "vtables", 4,
          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit
      )) {
        ImGui::TableSetupColumn("address", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("active", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("inactive", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (auto &entry: vtables) {
          auto vtable = entry.first;
          auto count = entry.second;

          ImGui::TableNextRow();

          ImGui::TableNextColumn();
          string vtable_string = fmt::format("{:08x}", vtable);
          if (ImGui::Selectable(vtable_string.c_str())) {
            string clip = fmt::format("{{0x{:08x}, \"\"}},", vtable);
            ImGui::SetClipboardText(clip.c_str());
          }

          ImGui::TableNextColumn();
          string name = "unknown";
          if (MP1_VTABLES.count(vtable)) {
            name = MP1_VTABLES[vtable];
          }
          ImGui::Text("%s", name.c_str());

          ImGui::TableNextColumn();
          ImGui::Text("%d", count.active);

          ImGui::TableNextColumn();
          ImGui::Text("%d", count.inactive);

        }
        ImGui::EndTable();
      }
      ImGui::Unindent();
    }

    ImGui::Text("Editor ID: #38 Class: @CPlayer name: &name");
    ImGui::Text("(or just type what you're looking for)");
    objectFilter.Draw();
    ImGui::Checkbox("Show active only", &showActiveInTableOnly);

    tableHoveredUid = 0xFFFF;
    ImVec2 outer_size = ImVec2(0.0f, 400);
    if (ImGui::BeginTable(
        "entities", 5,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_Hideable
    )) {
      ImGui::TableSetupColumn("class", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("editorID", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("uniqueID", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide);
      ImGui::TableSetupColumn("active", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide);
      ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      for (auto &pair: entities) {
        auto &entity = pair.second;
        bool active = entity["active"].read_bool();
        if (showActiveInTableOnly && !active) continue;

        uint16_t uid = entity["uniqueID"].read_u16();
        uint32_t eid = entity["editorID"].read_u32();
        string name = entity["name"].read_string();

        string filterCheck = fmt::format("#{:08x}#{:08}@{}&{}", eid, eid, entity.type, name);
        if (!objectFilter.PassFilter(filterCheck.c_str())) continue;

        ImGui::TableNextRow();
        //class
        ImGui::TableNextColumn();
        string str = fmt::format("{}##eid-{}uid-{}", entity.type, eid, uid);
        if (ImGui::Selectable(str.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
          bool alreadyWatched = false;
          for (auto &toWatch: editorIdsToWatch) {
            if (toWatch.eid == eid) {
              toWatch.lastKnownUid = uid;
              toWatch.type = entity.type;
              alreadyWatched = true;
            }
          }
          if (!alreadyWatched) {
            editorIdsToWatch.push_back(WatchedEditorId{
                .eid = eid,
                .lastKnownUid = uid,
                .type = entity.type,
            });
          }
        }
        if (ImGui::IsItemHovered()) {
          tableHoveredUid = uid;
        }

        //eid
        ImGui::TableNextColumn();
        str = fmt::format("{:08x}", eid);
        ImGui::Text("%s", str.c_str());

        //uid
        ImGui::TableNextColumn();
        str = fmt::format("{:04x}", uid);
        ImGui::Text("%s", str.c_str());

        //active
        ImGui::TableNextColumn();
        if (active) {
          ImGui::Text("yes");
        } else {
          ImGui::Text("no");
        }

        //name
        ImGui::TableNextColumn();
        str = fmt::format("{}", name);
        ImGui::Text("%s", str.c_str());
      }
      ImGui::EndTable();
      ImGui::Text("tableHoveredUid: %d", tableHoveredUid);
    }
  }
  ImGui::End();

  // Render individual object windows
  for (auto iter = editorIdsToWatch.begin(); iter != editorIdsToWatch.end();) {
    WatchedEditorId &eidToWatch = *iter;
    bool open = true;
    ImGui::SetNextWindowSizeConstraints(ImVec2(240, 200), ImVec2(1000000, 1000000));
    string windowTitle = fmt::format("{} {:08x}##{:08x}", eidToWatch.type, eidToWatch.eid, eidToWatch.eid);
    if (ImGui::Begin(windowTitle.c_str(), &open)) {
      // first check if we have something based on the last known unique id
      bool handled = false;
      if (!handled && uidToEntity.count(eidToWatch.lastKnownUid)) {
        GameMember entity = uidToEntity[eidToWatch.lastKnownUid];
        uint32_t eid = entity["editorID"].read_u32();
        if (eid == eidToWatch.eid && entity.type == eidToWatch.type) {
          GameObjectRenderers::render(entity, false);
          handled = true;
        }
      }
      if (!handled && eidToEntity.count(eidToWatch.eid)) {
        auto entity = eidToEntity[eidToWatch.eid];
        uint16_t uid = entity["uniqueID"].read_u16();
        eidToWatch.lastKnownUid = uid;
        GameObjectRenderers::render(entity, false);
        handled = true;
      }
      if (!handled) {
        ImGui::Text("Not loaded");
      }
    }
    ImGui::End();
    if (!open) {
      iter = editorIdsToWatch.erase(iter);
    } else {
      iter++;
    }
  }
}
