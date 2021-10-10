#include "WorldRenderer.hpp"

#include <fmt/format.h>
#include "imgui.h"
#include "GameMemory.h"
#include "defs/GameOffsets.hpp"
#include "defs/GameDefinitions.hpp"
#include "utils/AreaUtils.hpp"

using namespace GameDefinitions;
using namespace std;

void WorldRenderer::update() {
  updateAreas();
}

void
WorldRenderer::updateAreas() const {// First of all, get the list of areas. If we don't have them, then just leave what we have

}

void WorldRenderer::render() {

}

void WorldRenderer::renderImGui() {
  ImGui::SetNextWindowPos(
      ImVec2(ImGui::GetIO().DisplaySize.x - 10, 20),
      0,
      ImVec2(1, 0)
  );
  ImGui::Begin(
      "WorldStatus", nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_AlwaysAutoResize
  );

  auto areas = AreaUtils::getAreas();

  GameEnum EChain = *GameDefinitions::enumByName("EChain");
  GameEnum EPhase = *GameDefinitions::enumByName("EPhase");

  ImGuiTableFlags flags = ImGuiTableFlags_Borders;
  if (ImGui::BeginTable("areas", 4, flags)) {
    // Display headers so we can inspect their interaction with borders.
    // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
    ImGui::TableSetupColumn("MREA");
    ImGui::TableSetupColumn("Chain");
    ImGui::TableSetupColumn("Phase");
    ImGui::TableSetupColumn("Occluded");
    ImGui::TableHeadersRow();

    for (auto area: areas) {
      uint32_t chain = area["curChain"].read_u32();
      if (chain == 1 /* deallocated */) continue;

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      uint32_t mrea = area["mrea"].read_u32();
      ImGui::Text("%s", fmt::format("{:08x}", mrea).c_str());

      ImGui::TableNextColumn();
      string chainText = EChain.valueByValue(chain).name;
      ImGui::Text("%s", chainText.c_str());

      ImGui::TableNextColumn();
      uint32_t phase = area["phase"].read_u32();
      string phaseText = EPhase.valueByValue(phase).name;
      ImGui::Text("%s", phaseText.c_str());

      ImGui::TableNextColumn();
      string occludedText = "yes";
      if (area["isPostConstructed"].read_bool()) {
        uint32_t occluded = area["postConstructed"]["occlusionState"].read_u32();
        if (occluded == 1) {
          occludedText = "no";
        }
      }
      ImGui::Text("%s", occludedText.c_str());
    }
    ImGui::EndTable();
  }

  ImGui::End();
}

