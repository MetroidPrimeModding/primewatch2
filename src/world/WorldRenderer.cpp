#include "WorldRenderer.hpp"

#include <fmt/format.h>
#include "imgui.h"
#include "GameMemory.h"
#include "defs/GameOffsets.hpp"
#include "defs/GameDefinitions.hpp"
#include "utils/AreaUtils.hpp"

using namespace GameDefinitions;
using namespace std;

const char *meshVertShader = R"src(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec4 normal;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)src";

const char *meshFragShader = R"src(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)src";

void WorldRenderer::update() {
  updateAreas();
}

void WorldRenderer::updateAreas() {
  auto areas = AreaUtils::getAreas();
  for (auto &area: areas) {
    uint32_t mrea = area["mrea"].read_u32();
    if (area["isPostConstructed"].read_bool()) {
      // Ok, it's loaded. Let's grab the collision, if we don't have it
      if (mesh_by_mrea.count(mrea) == 0) {
        auto mesh = loadMesh(area);
        if (mesh.has_value()) mesh_by_mrea[mrea] = std::move(*mesh);
      }
    } else {
      // It's gone. Remove it from the map.
      mesh_by_mrea.erase(mrea);
    }
  }
}

optional<CollisionMesh> WorldRenderer::loadMesh(const GameMember &area) {
  auto postConstructed = area["postConstructed"];
  auto collision = postConstructed["collision"]["value"];
  if (collision.offset == 0) return {};

  CollisionMesh res{};
  auto matCount = collision["matCount"].read_u32();
  auto edgeCount = collision["edgeCount"].read_u32();
  auto polyCount = collision["polyCount"].read_u32();
  auto vertCount = collision["vertCount"].read_u32();

  auto materialStart = collision["materials"].offset;
  auto edgeStart = collision["edges"].offset;
  auto polyStart = collision["polyEdges"].offset;
  auto vertStart = collision["verts"].offset;

  auto vertMaterialStart = collision["polyEdges"].offset;
  auto edgeMaterialStart = collision["edgeMats"].offset;
  auto polyMaterialStart = collision["polyMats"].offset;

  for (int i = 0; i < matCount; i++) {
    res.materials.push_back(
        GameMemory::read_u32(materialStart + i * 4)
    );
  }

  for (int i = 0; i < vertCount; i++) {
    res.raw_verts.emplace_back(
        GameMemory::read_float(vertStart + (i * 3 + 0) * 4),
        GameMemory::read_float(vertStart + (i * 3 + 1) * 4),
        GameMemory::read_float(vertStart + (i * 3 + 2) * 4)
    );
  }
  // separate loop for locality reasons
  for (int i = 0; i < vertCount; i++) {
    res.raw_vert_materials.emplace_back(
        GameMemory::read_u8(vertMaterialStart + i * 2)
    );
  }

  for (int i = 0; i < edgeCount; i++) {
    res.raw_edges.emplace_back(
        GameMemory::read_u16(edgeStart + (i * 2 + 0) * 2),
        GameMemory::read_u16(edgeStart + (i * 2 + 1) * 2)
    );
  }
  for (int i = 0; i < edgeCount; i++) {
    res.raw_edge_materials.emplace_back(
        GameMemory::read_u8(edgeMaterialStart + i * 2)
    );
  }

  for (int i = 0; i < polyCount; i++) {
    res.raw_polys.emplace_back(
        GameMemory::read_u16(polyStart + (i * 3 + 0) * 2),
        GameMemory::read_u16(polyStart + (i * 3 + 1) * 2),
        GameMemory::read_u16(polyStart + (i * 3 + 2) * 2)
    );
  }
  for (int i = 0; i < polyCount; i++) {
    res.raw_poly_materials.emplace_back(
        GameMemory::read_u8(polyMaterialStart + i * 2)
    );
  }

  res.initGlMesh();

  return res;
}


void WorldRenderer::render() {
  if (!shader) {
    shader = make_unique<OpenGLShader>(meshVertShader, meshFragShader);
  }

  shader->use();
  for (auto &[k, v]:mesh_by_mrea) {
    v.draw();
  }
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

    for (auto &area: areas) {
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