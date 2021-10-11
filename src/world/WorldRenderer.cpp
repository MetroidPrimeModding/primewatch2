#include "WorldRenderer.hpp"

#include <fmt/format.h>
#include "imgui.h"
#include "GameMemory.h"
#include "defs/GameOffsets.hpp"
#include "defs/GameDefinitions.hpp"
#include "utils/AreaUtils.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>

using namespace GameDefinitions;
using namespace std;

const char *meshVertShader = R"src(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;

out vec4 vertexColor;
out vec3 normal;
out vec3 fragPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * vec4(aPos, 1.0f);
  vertexColor = aColor;
  normal = aNormal;
  fragPos = aPos;
}
)src";

const char *meshFragShader = R"src(#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec3 normal;
in vec3 fragPos;

uniform vec3 lightDir;

void main() {
  vec3 lightColor = vec3(1,1,1);
  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular
//  float specularStrength = 0.5;
//  vec3 viewDir = normalize(viewPos - fragPos);
//  vec3 reflectDir = reflect(-lightDir, norm);
//  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//  vec3 specular = specularStrength * spec * lightColor;
  vec3 specular = vec3(0,0,0);

  FragColor = vec4(ambient + diffuse + specular, 1.0) * vertexColor;
}
)src";

WorldRenderer::WorldRenderer() {
}

void WorldRenderer::update(const PrimeWatchInput &input) {
  updateAreas();

  float pitchSpeed = 0.03;
  float yawSpeed = 0.03;
  float distSpeed = 0.5;

  if (input.camUp) {
    pitch += pitchSpeed;
  } else if (input.camDown) {
    pitch -= pitchSpeed;
  } else if (input.camLeft) {
    yaw -= yawSpeed;
  } else if (input.camRight) {
    yaw += yawSpeed;
  } else if (input.camIn) {
    distance -= distSpeed;
  } else if (input.camOut) {
    distance += distSpeed;
  }

  pitch = glm::clamp(pitch, -(glm::pi<float>() / 2 - 0.1f), glm::pi<float>() / 2 - 0.1f);
  distance = glm::clamp(distance, 1.0f, 100.0f);

  GameMember stateManager{.name="g_stateManager", .type="CStateManager", .offset=CStateManager_ADDRESS};
  GameMember transform = stateManager["player"]["transform"];
  lookAt = glm::vec3{
      transform["posX"].read_f32(),
      transform["posY"].read_f32(),
      transform["posZ"].read_f32(),
  };
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
        static_cast<ECollisionMaterial>(GameMemory::read_u32(materialStart + i * 4))
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
        GameMemory::read_u8(vertMaterialStart + i)
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
        GameMemory::read_u8(edgeMaterialStart + i)
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
        GameMemory::read_u8(polyMaterialStart + i)
    );
  }

  res.initGlMesh();

  return res;
}

void WorldRenderer::render() {
  if (!shader) {
    shader = make_unique<OpenGLShader>(meshVertShader, meshFragShader);
  }

  glm::mat4 projection = glm::perspective(fov, aspect, zNear, zFar);

  glm::quat angle = glm::quat(glm::vec3(0, pitch, yaw));
  glm::vec3 eye = glm::vec4(lookAt, 1.0f) - (angle * glm::vec4{distance,0,0,1});
  glm::mat4 view = glm::lookAt(eye, lookAt, up);

  shader->setMat4("view", view);
  shader->setMat4("projection", projection);
  shader->setVec3("lightDir", glm::normalize(lightDir));

  shader->use();
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
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