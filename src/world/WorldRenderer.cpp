#include "WorldRenderer.hpp"

#include <iostream>
#include <fmt/format.h>
#include "imgui.h"
#include "GameMemory.h"
#include "defs/GameOffsets.hpp"
#include "defs/GameDefinitions.hpp"
#include "utils/AreaUtils.hpp"
#include "utils/GameObjectUtils.hpp"
#include "gl/ShapeGenerator.hpp"
#include "defs/GameObjectRenderers.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <utils/MathUtils.hpp>

using namespace GameDefinitions;
using namespace std;

const char *meshVertShader = R"src(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;

out vec4 vertexColor;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  vertexColor = aColor;
  normal = mat3(transpose(inverse(model))) * aNormal;
  fragPos = vec3(model * vec4(aPos, 1.0f));
}
)src";

const char *meshFragShader = R"src(#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;
uniform vec3 lightDir;

void main() {
  vec3 lightColor = vec3(1,1,1);
  // ambient
  float ambientStrength = 0.2;
  vec3 ambient = ambientStrength * lightColor;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular
  float specularStrength = 0.3;
  vec3 viewDir = normalize(viewPos - fragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
  vec3 specular = specularStrength * spec * lightColor;

  FragColor = vec4(ambient + diffuse + specular, 1.0) * vertexColor;
}
)src";

void WorldRenderer::init() {
  renderBuff = make_unique<ImmediateModeBuffer>();
  translucentRenderBuff = make_unique<ImmediateModeBuffer>();
}

void WorldRenderer::update(const PrimeWatchInput &input) {
  updateAreas();

  pitch += input.camPitch;
  yaw += input.camYaw;
  distance += input.camZoom;

  pitch = glm::clamp(pitch, -(glm::pi<float>() / 2 - 0.1f), glm::pi<float>() / 2 - 0.1f);
  distance = glm::clamp(distance, 1.0f, 100.0f);

  GameMember stateManager = g_stateManager;

  glm::mat4 tf = MathUtils::readAsCTransform(stateManager["player"]["transform"]);
  playerPos = glm::column(tf, 3);
  playerVelocity = MathUtils::readAsCVector3f(stateManager["player"]["velocity"]);
  lastKnownNonCollidingPos = MathUtils::readAsCVector3f(stateManager["player"]["lastNonCollidingState"]["translation"]);

  playerOrientation = MathUtils::readAsCQuaternion(stateManager["player"]["orientation"]);
  playerIsMorphed = stateManager["player"]["morphState"].read_u32() == 1; // Morphed

  GameMember cameraManager = stateManager["cameraManager"];
  uint16_t cameraID = cameraManager["curCameraId"].read_u16();
  GameMember camera = GameObjectUtils::getObjectByEntityID(cameraID);
  camera.type = "CGameCamera"; // hacky hack. We assume this is a CGameCamera for now.
  gameCam.perspective = MathUtils::readAsCMatrix4f(camera["perspectiveMatrix"]);
  gameCam.transform = MathUtils::readAsCTransform(camera["transform"]);
  gameCam.fov = camera["fov"].read_f32();
  gameCam.znear = camera["znear"].read_f32();
  gameCam.zfar = camera["zfar"].read_f32();
  gameCam.aspect = camera["aspect"].read_f32();
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

  if (edgeCount > 50000 || polyCount > 50000 || vertCount > 50000 || matCount > 50000) {
    cerr << "Bad read for polys" << endl;
    return {};
  }

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

void WorldRenderer::render(const std::vector<GameDefinitions::GameMember> &entities,
                           const set<uint16_t> &highlightedEids) {
  if (!shader) {
    shader = make_unique<OpenGLShader>(meshVertShader, meshFragShader);
  }

  renderBuff->clear();
  translucentRenderBuff->clear();

  glm::mat4 projection{1.0f};
  glm::mat4 view{1.0f};
  glm::vec3 eye;

  if (cameraMode == CameraMode::FOLLOW_PLAYER) {
    projection = glm::perspective(fov, aspect, zNear, zFar);
    glm::quat angle = glm::quat(glm::vec3(0, pitch, yaw));
    // we look at the lastKnownNonCollidingPos because it's less jumpy
    eye = glm::vec4(lastKnownNonCollidingPos, 1.0f) - (angle * glm::vec4{distance, 0, 0, 1});
    view = glm::lookAt(eye, lastKnownNonCollidingPos, up);
  } else if (cameraMode == CameraMode::GAME_CAM) {
    projection = gameCam.perspective;
    view = glm::inverse(gameCam.transform);

    glm::vec3 scale;
    glm::quat orient;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(
        view, // mat
        scale, //scale
        orient, //orient
        eye, // translation,
        skew, //skew
        perspective // perspective
    );

//    view = glm::translate(eye) * glm::toMat4(orient);
  }

  // Player collision
  if (playerIsMorphed) {
    glm::mat4 model =
        glm::translate(playerPos + glm::vec3(0, 0, 0.7)) *
        glm::toMat4(playerOrientation);
    renderBuff->setTransform(model);
    renderBuff->addTris(ShapeGenerator::generateSphere(
        glm::vec3{0, 0, 0},
        0.7f,
        glm::vec4{1, 1, 1, 1}
    ));
  } else {
    renderBuff->setTransform(glm::translate(playerPos));
    renderBuff->addTris(ShapeGenerator::generateCube(
        glm::vec3{-0.5, -0.5, 0},
        glm::vec3{0.5, 0.5, 2.7},
        glm::vec4{1, 1, 1, 1}
    ));
  }
  // player last known ghost
  translucentRenderBuff->setTransform(glm::translate(lastKnownNonCollidingPos));
  translucentRenderBuff->addTris(ShapeGenerator::generateCube(
      glm::vec3{-0.5, -0.5, 0},
      glm::vec3{0.5, 0.5, 2.7},
      glm::vec4{1, 0.5, 0.5, 0.5}
  ));

  // Camera
  renderBuff->setTransform(glm::mat3{1.0f});
  renderBuff->addLines(ShapeGenerator::generateCameraLineSegments(
      gameCam.perspective,
      gameCam.transform
  ));

  // speed
  if (playerIsMorphed) {
    renderBuff->setTransform(glm::translate(playerPos + glm::vec3{0, 0, 0.7f}));
  } else {
    renderBuff->setTransform(glm::translate(playerPos + glm::vec3{0, 0, 2.7f / 2.0f}));
  }
  // calculate "forward" and "degrees from forward
  glm::vec2 forward = glm::normalize(playerOrientation * glm::vec3(0, 1, 0));
  glm::vec2 movement = glm::normalize(playerVelocity);
  float angle = glm::acos(glm::dot(forward, movement) / (glm::length(forward) * glm::length(movement)));

  glm::vec4 color{0, 1, 0, 1};
  if (glm::abs(angle) > glm::pi<float>() / 2.0f || glm::isnan(angle)) {
    color = {1, 0, 0, 1};
  } else {
    float percent = angle / (glm::pi<float>() / 2.0f);
    color = {0, percent * 0.5f + 0.5f, 0, 1};
    if (percent > 0.95) {
      color = {0, 1, 1, 1};
    }
  }
  renderBuff->setColor({1, 1, 1, 1});
  renderBuff->addLine(glm::vec3{0, 0, 0}, glm::vec3{forward, 0});
  renderBuff->setColor(color);
  renderBuff->addLine(glm::vec3{0, 0, 0}, playerVelocity * 0.3f);

  renderEntities(entities, highlightedEids);

  shader->setMat4("model", glm::identity<glm::mat4>());
  shader->setMat4("view", view);
  shader->setMat4("projection", projection);
  shader->setVec3("lightDir", glm::normalize(lightDir));
  shader->setVec3("viewPos", eye);

  shader->use();
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glFrontFace(GL_CW);

  switch (culling) {
    case CullType::BACK:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      break;
    case CullType::FRONT:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      break;
    case CullType::NONE:
      glDisable(GL_CULL_FACE);
      break;
  }

  for (auto &[k, v]: mesh_by_mrea) {
    v.draw();
  }

  // then player
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glLineWidth(2.0f);
  shader->setMat4("model", glm::mat4{1.0f});
  renderBuff->draw();

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  shader->setMat4("model", glm::mat4{1.0f});
  translucentRenderBuff->draw();
  glEnable(GL_DEPTH_TEST);
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

  // and now we show the like, load stats
  // load queue
  auto loadingDatas = GameObjectUtils::getAllLoadingDatas();
  if (loadingDatas.size() > 0) {
    ImGui::Text("Loading %ld", loadingDatas.size());
    int shownLoading = 0;
    uint32_t shownSize = 0;
    uint32_t restSize = 0;
    for (auto &loadingData: loadingDatas) {
      uint32_t size = loadingData["resLen"].read_u32();
      if (shownLoading < 5) {
        string tag = GameObjectUtils::objectTagToString(loadingData["tag"]);

        string msg = fmt::format("{}: {}", tag.c_str(), size);
        ImGui::Text("%s", msg.c_str());
        shownLoading++;
        shownSize += size;
      } else {
        restSize += size;
      }
    }
    if (shownSize > 0 || restSize > 0) {
      ImGui::Text("+%dk = %dk", restSize / 1024, (shownSize + restSize) / 1024);
    }
  }


  ImGui::End();
}

void WorldRenderer::renderEntities(const std::vector<GameDefinitions::GameMember> &entities,
                                   const set<uint16_t> &highlightedEntities) {
  renderBuff->setTransform(glm::mat4{1.0f});

  uint32_t triggerRenderFlags = 0;
  if (triggerRenderConfig.detectPlayer) triggerRenderFlags |= 0x1;
  if (triggerRenderConfig.detectAi) triggerRenderFlags |= 0x2;
  if (triggerRenderConfig.detectProjectiles) triggerRenderFlags |= 0x4 | 0x8 | 0x10 | 0x20 | 0x100 | 0x200 | 0x400;
  if (triggerRenderConfig.detectBombs) triggerRenderFlags |= 0x40;
  if (triggerRenderConfig.detectPowerBombs) triggerRenderFlags |= 0x80;
  if (triggerRenderConfig.killOnEnter) triggerRenderFlags |= 0x800;
  if (triggerRenderConfig.detectMorphedPlayer) triggerRenderFlags |= 0x1000;
  if (triggerRenderConfig.useCollisionImpluses) triggerRenderFlags |= 0x2000;
  if (triggerRenderConfig.detectCamera) triggerRenderFlags |= 0x4000;
  if (triggerRenderConfig.useBooleanIntersection) triggerRenderFlags |= 0x8000;
  if (triggerRenderConfig.detectUnmorphedPlayer) triggerRenderFlags |= 0x10000;
  if (triggerRenderConfig.blockEnvironmentalEffects) triggerRenderFlags |= 0x20000;

  for (auto &entity: entities) {
    bool active = entity["active"].read_bool();
    if (!active) continue;

    uint16_t uid = entity["uniqueID"].read_u16();
    bool isHighlighted = highlightedEntities.contains(uid);

    if (entity.extendsClass("CScriptTrigger")) {
      uint32_t flags = entity["triggerFlags"].read_u32();
      if (entity.extendsClass("CScriptWater")) {
        if (triggerRenderConfig.water) {
          drawTrigger(entity, isHighlighted);
        }
      } else if (flags & triggerRenderFlags) {
        drawTrigger(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CScriptDock")) {
      if (triggerRenderConfig.docks) {
        drawDock(entity, isHighlighted);
      }
    }
  }
}

void WorldRenderer::drawTrigger(const GameMember &entity, bool isHighlighted) {
  glm::vec3 min = MathUtils::readAsCVector3f(entity["bounds"]["min"]);
  glm::vec3 max = MathUtils::readAsCVector3f(entity["bounds"]["max"]);
  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);

  glm::vec4 color{1, 1, 1, 0.5f};

  if (entity.extendsClass("CScriptWater")) {
    color = {0.5f, 0.5f, 1.0f, 0.5f};
  }

  if (isHighlighted) {
    color = {1, 0, 0, 0.5f};
  }

  translucentRenderBuff->setTransform(transform);

  translucentRenderBuff->addTris(
      ShapeGenerator::generateCube(min, max, color)
  );
}

void WorldRenderer::drawDock(const GameMember &entity, bool isHighlighted) {
  glm::vec3 min = MathUtils::readAsCVector3f(entity["collisionPrimitive"]["aabb"]["min"]);
  glm::vec3 max = MathUtils::readAsCVector3f(entity["collisionPrimitive"]["aabb"]["max"]);
  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);

  glm::vec4 color{0.5f, 1.0f, 0.5f, 0.5f};

  if (isHighlighted) {
    color = {1, 0, 0, 0.5f};
  }

  translucentRenderBuff->setTransform(transform);

  translucentRenderBuff->addTris(
      ShapeGenerator::generateCube(min, max, color)
  );
}
