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
#include "defs/EItemType.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <utils/MathUtils.hpp>

#include "CollisionMesh.hpp"
#include "CollisionMesh.hpp"

using namespace GameDefinitions;
using namespace std;

const char *meshVertShader = R"src(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aBarycentric;

out vec4 vertexColor;
out vec3 normal;
out vec3 fragPos;
out vec3 barycentric;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
  vertexColor = aColor;
  normal = mat3(transpose(inverse(model))) * aNormal;
  fragPos = vec3(model * vec4(aPos, 1.0f));
  barycentric = aBarycentric;
}
)src";

const char *meshFragShader = R"src(#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec3 normal;
in vec3 fragPos;
in vec3 barycentric;

uniform vec3 viewPos;
uniform vec3 lightDir;

void main() {
  float edgeThickness = 0.015;
  float minBary = min(min(barycentric.x, barycentric.y), barycentric.z);
  if (minBary > 0 && minBary < edgeThickness) { // draw edges
    float color = 0.2;
    FragColor = vec4(color, color, color, 1);
  } else {
    vec3 lightColor = vec3(1,1,1);
    // ambient
    float ambientStrength = 0.7;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diffStrength = 0.5;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * diffStrength;

    // specular
    float specularStrength = 0.2;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    vec3 specular = specularStrength * spec * lightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0) * vertexColor;
  }
}
)src";

const char *lineFragShader = R"src(#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;
uniform vec3 lightDir;

void main() {
  vec3 lightColor = vec3(1,1,1);
  // ambient
  float ambientStrength = 1;
  vec3 ambient = ambientStrength * lightColor;

  FragColor = vec4(ambient, 1.0) * vertexColor;
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
  player.position = glm::column(tf, 3);
  player.velocity = MathUtils::readAsCVector3f(stateManager["player"]["velocity"]);
  lastKnownNonCollidingPos = MathUtils::readAsCVector3f(stateManager["player"]["lastNonCollidingState"]["translation"]);
  playerLookVec = MathUtils::readAsCVector3f(stateManager["player"]["lookDir"]);

  player.orientation = MathUtils::readAsCQuaternion(stateManager["player"]["orientation"]);
  player.isMorphed = stateManager["player"]["morphState"].read_u32() == 1; // Morphed

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

  res.min = MathUtils::readAsCVector3f(area["aabb"]["min"]);
  res.max = MathUtils::readAsCVector3f(area["aabb"]["max"]);

  res.initGlMesh();

  return res;
}

void WorldRenderer::render(const std::map<TUniqueID, GameDefinitions::GameMember> &entities,
                           const set<uint16_t> &highlightedEids) {
  if (!meshShader) {
    meshShader = make_unique<OpenGLShader>(meshVertShader, meshFragShader);
    lineShader = make_unique<OpenGLShader>(meshVertShader, lineFragShader);
  }

  renderBuff->clear();
  translucentRenderBuff->clear();

  if (cameraMode == CameraMode::FOLLOW_PLAYER) {
    camProjection = glm::perspective(fov, aspect, zNear, zFar);
    glm::quat angle = glm::quat(glm::vec3(0, pitch, yaw));
    // we look at the lastKnownNonCollidingPos because it's less jumpy
    auto lookPos = lastKnownNonCollidingPos;
    switch (orbitPlayerCameraOrigin) {
      case OrbitPlayerCameraOrigin::TOP:
        if (player.isMorphed) {
          lookPos.z += 1.4f;
        } else {
          lookPos.z += 2.7f;
        }
        break;
      case OrbitPlayerCameraOrigin::CENTER:
        if (player.isMorphed) {
          lookPos.z += 0.7f;
        } else {
          lookPos.z += 1.35f;
        }
        break;
      case OrbitPlayerCameraOrigin::BOTTOM:
        // already correct
        break;
    }

    camEye = glm::vec4(lookPos, 1.0f) - (angle * glm::vec4{distance, 0, 0, 1});
    camView = glm::lookAt(camEye, lookPos, up);
    manualCameraPos = lookPos;
  } else if (cameraMode == CameraMode::GAME_CAM) {
    camProjection = gameCam.perspective;
    camView = glm::inverse(gameCam.transform);
//    view = glm::translate(eye) * glm::toMat4(orient);
  } else if (cameraMode == CameraMode::DETATCHED) {
    camProjection = glm::perspective(fov, aspect, zNear, zFar);
    glm::quat angle = glm::quat(glm::vec3(0, pitch, yaw));

    camEye = glm::vec4(manualCameraPos, 1.0f) - (angle * glm::vec4{distance, 0, 0, 1});
    camView = glm::lookAt(camEye, manualCameraPos, up);
  }
  {
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(
        camView, // mat
        scale, //scale
        camPointing, //orient
        camEye, // translation,
        skew, //skew
        perspective // perspective
    );
  }

  drawPlayer(player, glm::vec4{1,1,1,1});
  for (PlayerGhost &ghost: playerGhosts) {
    if (ghost.enabled) {
      // teal
      drawPlayer(ghost, glm::vec4{0, 1, 1, 0.5f});
    }
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
      gameCam.transform,
      camLineLength
  ));

  renderEntities(entities, highlightedEids);

  meshShader->use();
  meshShader->setMat4("model", glm::identity<glm::mat4>());
  meshShader->setMat4("view", camView);
  meshShader->setMat4("projection", camProjection);
  meshShader->setVec3("lightDir", glm::normalize(lightDir));
  meshShader->setVec3("viewPos", camEye);

  lineShader->use();
  lineShader->setMat4("model", glm::identity<glm::mat4>());
  lineShader->setMat4("view", camView);
  lineShader->setMat4("projection", camProjection);
  lineShader->setVec3("lightDir", glm::normalize(lightDir));
  lineShader->setVec3("viewPos", camEye);

  meshShader->use();
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

    renderBuff->addLines(ShapeGenerator::generateCubeLines(
        v.min,
        v.max,
        glm::vec4{1, 1, 1, 1}
    ));
  }

  // then player
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glLineWidth(2.0f);
  meshShader->use();
  meshShader->setMat4("model", glm::mat4{1.0f});
  renderBuff->drawTris();

  lineShader->use();
  lineShader->setMat4("model", glm::mat4{1.0f});
  renderBuff->drawLines();

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  meshShader->use();
  meshShader->setMat4("model", glm::mat4{1.0f});
  translucentRenderBuff->drawTris();

  lineShader->use();
  lineShader->setMat4("model", glm::mat4{1.0f});
  translucentRenderBuff->drawLines();
  glDepthMask(GL_TRUE);

  meshShader->use();
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
  {
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
  }
  ImGui::End();

  ImGui::SetNextWindowPos(
      ImVec2(10, ImGui::GetIO().DisplaySize.y - 10),
      0,
      ImVec2(0, 1)
  );
  ImGui::Begin(
      "PlayerStatus", nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
  );
  {
    auto forward = playerLookVec;

    auto hforward = normalize(glm::vec2(forward.x, forward.y));
    auto hvel = glm::vec2(player.velocity.x, player.velocity.y);

    ImGui::Text(
        "pos: %8.3fx %8.3fy %8.3fz",
        player.position.x, player.position.y, player.position.z);

    ImGui::Text("vel: %8.3fx %8.3fy %8.3fz %8.3fh",
                player.velocity.x, player.velocity.y, player.velocity.z, glm::length(hvel));

    auto hveldir = normalize(hvel);
    auto forwardAngle = glm::atan(hforward.y, hforward.x);
    auto velAngle = glm::atan(hveldir.y, hveldir.x);
    auto angle = forwardAngle - velAngle;
    ImGui::Text("look: %6.3fx %6.3fy %6.1fdeg | vel %6.3fx %6.3fy %6.1fdeg | %6.1f deg",
                hforward.x, hforward.y, glm::degrees(forwardAngle),
                hveldir.x, hveldir.y, glm::degrees(velAngle),
                glm::degrees(angle));
  }
  ImGui::End();
}

void WorldRenderer::drawPlayer(const PlayerGhost &ghost, glm::vec4 color) {
  auto *buf = renderBuff.get();
  if (color.a < 0.99f) {
    buf = translucentRenderBuff.get();
  }
  // Player collision
  if (ghost.isMorphed) {
    glm::mat4 model =
        glm::translate(ghost.position + glm::vec3(0, 0, 0.7)) *
        glm::toMat4(ghost.orientation);
    buf->setTransform(model);
    buf->addTris(ShapeGenerator::generateSphere(
        glm::vec3{0, 0, 0},
        0.7f,
        color
    ));
  } else {
    buf->setTransform(glm::translate(ghost.position));
    buf->addTris(ShapeGenerator::generateCube(
        glm::vec3{-0.5, -0.5, 0},
        glm::vec3{0.5, 0.5, 2.7},
        color
    ));
  }

  // speed
  if (ghost.isMorphed) {
    renderBuff->setTransform(glm::translate(ghost.position + glm::vec3{0, 0, 0.7f}));
  } else {
    renderBuff->setTransform(glm::translate(ghost.position + glm::vec3{0, 0, 2.7f / 2.0f}));
  }
  // calculate "forward" and "degrees from forward
  glm::vec2 forward = glm::normalize(ghost.orientation * glm::vec3(0, 1, 0));
  glm::vec2 movement = glm::normalize(ghost.velocity);
  float angle = glm::acos(glm::dot(forward, movement) / (glm::length(forward) * glm::length(movement)));

  glm::vec4 speedColor{0, 1, 0, 1};
  if (glm::abs(angle) > glm::pi<float>() / 2.0f || glm::isnan(angle)) {
    speedColor = {1, 0, 0, 1};
  } else {
    float percent = angle / (glm::pi<float>() / 2.0f);
    speedColor = {0, percent * 0.5f + 0.5f, 0, 1};
    if (percent > 0.95) {
      speedColor = {0, 1, 1, 1};
    }
  }
  renderBuff->setColor({1, 1, 1, 1});
  renderBuff->addLine(glm::vec3{0, 0, 0}, glm::vec3{forward, 0});
  renderBuff->setColor(speedColor);
  renderBuff->addLine(glm::vec3{0, 0, 0}, ghost.velocity * 0.3f);
}

void WorldRenderer::renderEntities(const map<TUniqueID, GameMember> &entities,
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

  for (auto &pair: entities) {
    auto &entity = pair.second;
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
    } else if (entity.extendsClass("CGameProjectile")) {
      if (actorRenderConfig.renderProjectiles) {
        drawProjectile(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CBomb")) {
      if (actorRenderConfig.renderProjectiles) {
        drawBomb(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CPowerBomb")) {
      if (actorRenderConfig.renderProjectiles) {
        drawPowerBomb(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CPlayer")) {
      // player render is handled elsewhere
    } else if (entity.extendsClass("CChozoGhost")) {
      if (actorRenderConfig.renderAI) {
        drawChozoGhost(entity, isHighlighted, entities);
      }
    } else if (entity.extendsClass("CScriptPickup")) {
      if (actorRenderConfig.renderPickups) {
        drawPickup(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CCollisionActor")) {
      if (actorRenderConfig.renderCollisionActors) {
        drawCollisionActor(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CAi")) {
      if (actorRenderConfig.renderAI) {
        drawAi(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CPhysicsActor")) {
      if (actorRenderConfig.renderPhysicsActors) {
        drawPhysicsActor(entity, isHighlighted);
      }
    } else if (entity.extendsClass("CActor")) {
      if (actorRenderConfig.renderActors) {
        drawActor(entity, isHighlighted);
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

void WorldRenderer::drawPhysicsActor(const GameMember &entity, bool isHighlighted) {
  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);
  glm::vec3 pos = transform[3];

  glm::vec3 min = pos + MathUtils::readAsCVector3f(entity["collisionPrimitive"]["aabb"]["min"]);
  glm::vec3 max = pos + MathUtils::readAsCVector3f(entity["collisionPrimitive"]["aabb"]["max"]);

  if (glm::abs(glm::length(min - max)) < 0.1) {
    min = pos + MathUtils::readAsCVector3f(entity["baseBoundingBox"]["min"]);
    max = pos + MathUtils::readAsCVector3f(entity["baseBoundingBox"]["max"]);
  }

  if (glm::abs(glm::length(min - max)) < 0.1) {
    min = MathUtils::readAsCVector3f(entity["renderBounds"]["min"]);
    max = MathUtils::readAsCVector3f(entity["renderBounds"]["max"]);
  }


  glm::vec4 color{1, 1, 1, 0.5f};

  if (isHighlighted) {
    color = {1, 0, 0, 0.5f};
  }

  translucentRenderBuff->setColor(color);
  translucentRenderBuff->setTransform(glm::identity<glm::mat4>());

  translucentRenderBuff->addTris(
      ShapeGenerator::generateCube(min, max, color)
  );

  translucentRenderBuff->setTransform(transform);
  translucentRenderBuff->addLine(glm::vec3{0, -0.5f, 0}, glm::vec3{0, 0.5f, 0});
  translucentRenderBuff->addLine(glm::vec3{-0.5f, 0, 0}, glm::vec3{0.5f, 0, 0});
  translucentRenderBuff->addLine(glm::vec3{0, 0, -0.5f}, glm::vec3{0, 0, 0.5f});
}

void WorldRenderer::drawActor(const GameMember &entity, bool isHighlighted) {
  GameMember model = entity["modelData"];

  if (model.offset == 0) {
    if (!isHighlighted && !actorRenderConfig.renderAllActors) {

      return;
    }
  }

  glm::vec3 min = MathUtils::readAsCVector3f(entity["renderBounds"]["min"]);
  glm::vec3 max = MathUtils::readAsCVector3f(entity["renderBounds"]["max"]);

  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);

  glm::vec4 color{1, 1, 1, 0.5f};

  if (isHighlighted) {
    color = {1, 0, 0, 0.5f};
  }

  translucentRenderBuff->setColor(color);
  translucentRenderBuff->setTransform(glm::identity<glm::mat4>());

  translucentRenderBuff->addTris(
      ShapeGenerator::generateCube(min, max, color)
  );

  translucentRenderBuff->setTransform(transform);
  translucentRenderBuff->addLine(glm::vec3{0, -0.5f, 0}, glm::vec3{0, 0.5f, 0});
  translucentRenderBuff->addLine(glm::vec3{-0.5f, 0, 0}, glm::vec3{0.5f, 0, 0});
  translucentRenderBuff->addLine(glm::vec3{0, 0, -0.5f}, glm::vec3{0, 0, 0.5f});
}

void WorldRenderer::drawChozoGhost(const GameMember &ghost, bool highlighted,
                                   const std::map<TUniqueID, GameDefinitions::GameMember> &entities) {
  drawAi(ghost, highlighted);

  glm::vec3 spaceWarpPos = MathUtils::readAsCVector3f(ghost["spaceWarpPosition"]);
  glm::mat4 ghostTransform = MathUtils::readAsCTransform(ghost["transform"]);
  glm::vec3 ghostPos = ghostTransform[3];

//  translucentRenderBuff->setTransform(glm::identity<glm::mat4>());
//  translucentRenderBuff->setColor(glm::vec4{0,1,1,1});
//  translucentRenderBuff->addLine(ghostPos, spaceWarpPos);

  TUniqueID coverPointId = ghost["coverPoint"].read_u16() & 0x3FF;
  if (entities.contains(coverPointId)) {
    // find the entity
    const GameMember &coverPoint = entities.at(coverPointId);
    glm::mat4 coverTransform = MathUtils::readAsCTransform(coverPoint["transform"]);
    glm::vec3 coverPos = coverTransform[3];

    translucentRenderBuff->setTransform(glm::identity<glm::mat4>());
    translucentRenderBuff->setColor(glm::vec4{1, 0, 1, 1});
    translucentRenderBuff->addLine(ghostPos, coverPos);
  }
}

void WorldRenderer::drawProjectile(const GameMember &entity, bool isHighlighted) {
  bool projectileActive = entity["projectileActive"].read_bool();
  if (!projectileActive) return;

  GameMember projectile = entity["projectile"];
  glm::mat4 localToWorldXf = MathUtils::readAsCTransform(projectile["localToWorldXf"]);
  glm::mat4 localXf = MathUtils::readAsCTransform(projectile["localXf"]);
  glm::vec4 projOffset = glm::vec4(MathUtils::readAsCVector3f(projectile["projOffset"]), 0.0f);
  glm::vec4 localOffset = glm::vec4(MathUtils::readAsCVector3f(projectile["localOffset"]), 0.0f);
  glm::vec4 worldOffset = glm::vec4(MathUtils::readAsCVector3f(projectile["worldOffset"]), 0.0f);

  glm::vec3 pos = localToWorldXf * (localXf * projOffset + localOffset) + worldOffset;
  glm::vec3 scale = MathUtils::readAsCVector3f(projectile["scale"]);
  glm::vec3 vel = localToWorldXf * localXf * glm::vec4(MathUtils::readAsCVector3f(projectile["velocity"]), 0.0f);

  float extent = entity["projExtent"].read_f32();

  const glm::vec3 &size = glm::vec3{extent, extent, extent} / 2.0f * scale;
  glm::vec3 min = pos - size;
  glm::vec3 max = pos + size;

  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);

  glm::vec4 color{0.8f, 0.4f, 0.4f, 0.8f};

  if (isHighlighted) {
    color = {1, 0, 0, 0.5f};
  }

  translucentRenderBuff->setColor(color);
  translucentRenderBuff->setTransform(glm::identity<glm::mat4>());

  translucentRenderBuff->addTris(
      ShapeGenerator::generateCube(min, max, color)
  );

  if (isHighlighted) {
    translucentRenderBuff->setColor(glm::vec4{0.8, 0.8, 0.8, 0.5f});
    translucentRenderBuff->addLine(pos, pos + (glm::normalize(vel) * 1000.f));
  }
  translucentRenderBuff->setColor(glm::vec4{1, 0.5, 0.5, 1});
  translucentRenderBuff->addLine(pos, pos + (glm::normalize(vel) * 0.5f));
}

void WorldRenderer::drawBomb(const GameMember &entity, bool isHighlighted) {
  float fuseTimeSeconds = entity["fuseTime"].read_f32();
  int fuseTimeFrames = static_cast<int>(ceil(fuseTimeSeconds * 60.0f)) + 1;
  if (fuseTimeFrames <= 0) return; // it's done

  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);
  glm::vec3 pos = transform[3];
  glm::vec3 posToBall = player.position + glm::vec3{0, 0, 0.7f} - pos;
  float maxDistance = 1.5f; // from a tweak, could load it but eh

  // float dotUp = glm::dot(glm::normalize(posToBall), glm::vec3{0,0,1});
  if (glm::length(posToBall) < maxDistance && posToBall.z >= -0.7f) {
    isHighlighted = true;
  } else {
    isHighlighted = false;
  }

  glm::vec4 color{0.7f, 0.5f, 0.5f, 0.5f};
  if (isHighlighted) {
    color = {0.8, 0, 0, 0.8f};
  }

  translucentRenderBuff->setColor(color);
  translucentRenderBuff->setTransform(transform);

  translucentRenderBuff->addTris(
        ShapeGenerator::generateTruncatedSphere({0, 0, 0}, maxDistance - 0.7f, 0.0f, color)
  );

  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  string hp = fmt::format("{:d}", fuseTimeFrames);
  float centerX = ImGui::CalcTextSize(hp.c_str()).x / 2;
  glm::vec3 screenSpace = getScreenspacePosForActor(entity);
  dl->AddText(ImVec2(screenSpace.x - centerX, screenSpace.y - ImGui::GetTextLineHeight() / 2), ImColor(0xFFFFFFFF),
              hp.c_str());
}

void WorldRenderer::drawPowerBomb(const GameMember &entity, bool isHighlighted) {
  float curTime = entity["curTime"].read_f32();
  if (curTime < 1.0f || curTime > 4.0f) return; // it's done

  float curRadius = entity["curRadius"].read_f32();
  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);

  glm::vec4 color{0.8f, 0.4f, 0.4f, 0.4f};

  translucentRenderBuff->setColor(color);
  translucentRenderBuff->setTransform(transform);

  translucentRenderBuff->addTris(
      ShapeGenerator::generateSphere({0, 0, 0}, curRadius, color)
  );
}

void WorldRenderer::drawAi(const GameMember &ai, bool highlighted) {
  drawPhysicsActor(ai, highlighted);
  glm::vec3 screenSpace = getScreenspacePosForPhysicsActor(ai);

  float health = ai["healthInfo"]["health"].read_f32();

//  cout << screenSpace.x << "," << screenSpace.y << "," << screenSpace.z << endl;
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  string hp = fmt::format("{:0.1f}", health);
  float centerX = ImGui::CalcTextSize(hp.c_str()).x / 2;
  dl->AddText(ImVec2(screenSpace.x - centerX, screenSpace.y - ImGui::GetTextLineHeight() / 2), ImColor(0xFFFFFFFF),
              hp.c_str());
}

glm::vec3 WorldRenderer::getScreenspacePosForActor(const GameMember &physicsActor) {
  glm::mat4 transform = MathUtils::readAsCTransform(physicsActor["transform"]);
  glm::vec3 pos = transform[3];
  glm::vec3 screenSpace = glm::project(pos, camView, camProjection, camViewport);
  screenSpace.y = camViewport[3] - screenSpace.y;
  return screenSpace;
}

glm::vec3 WorldRenderer::getScreenspacePosForPhysicsActor(const GameMember &physicsActor) {
  glm::mat4 transform = MathUtils::readAsCTransform(physicsActor["transform"]);
  glm::vec3 pos = transform[3];

  glm::vec3 min = MathUtils::readAsCVector3f(physicsActor["collisionPrimitive"]["aabb"]["min"]);
  glm::vec3 max = MathUtils::readAsCVector3f(physicsActor["collisionPrimitive"]["aabb"]["max"]);

  if (glm::abs(glm::length(min - max)) < 0.1) {
    min = MathUtils::readAsCVector3f(physicsActor["baseBoundingBox"]["min"]);
    max = MathUtils::readAsCVector3f(physicsActor["baseBoundingBox"]["max"]);
  }

  if (glm::abs(glm::length(min - max)) < 0.1) {
    min = MathUtils::readAsCVector3f(physicsActor["renderBounds"]["min"]) - pos;
    max = MathUtils::readAsCVector3f(physicsActor["renderBounds"]["max"]) - pos;
  }

  glm::vec3 textPos = (min + max) / 2.0f;// Y is inverted in imgui
  glm::vec3 screenSpace = glm::project(pos + textPos, camView, camProjection, camViewport);
  screenSpace.y = camViewport[3] - screenSpace.y;
  return screenSpace;
}

void WorldRenderer::drawPickup(const GameMember &pickup, bool highlighted) {
  drawPhysicsActor(pickup, highlighted);
  glm::vec3 screenSpace = getScreenspacePosForPhysicsActor(pickup);

  EItemType itemType = static_cast<EItemType>(pickup["itemType"].read_u32());
  int32_t amount = static_cast<int32_t>(pickup["amount"].read_u32());
  int32_t capacity = static_cast<int32_t>(pickup["capacity"].read_u32());;
  float lifeTime = pickup["lifeTime"].read_f32();
  float curTime = pickup["curTime"].read_f32();

//  cout << screenSpace.x << "," << screenSpace.y << "," << screenSpace.z << endl;
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  {
    string line1 = fmt::format("{} {}/{}", itemTypeToName(itemType), amount, capacity);
    float centerX = ImGui::CalcTextSize(line1.c_str()).x / 2;
    dl->AddText(
        ImVec2(screenSpace.x - centerX, screenSpace.y - ImGui::GetTextLineHeight()),
        ImColor(0xFFFFFFFF),
        line1.c_str()
    );
  }
  {
    string line2 = fmt::format("{:0.1f}/{:0.1f}", curTime, lifeTime);
    float centerX = ImGui::CalcTextSize(line2.c_str()).x / 2;
    dl->AddText(
        ImVec2(screenSpace.x - centerX, screenSpace.y),
        ImColor(0xFFFFFFFF),
        line2.c_str()
    );
  }
}

void WorldRenderer::drawCollisionActor(const GameMember &entity, bool isHighlighted) {
  glm::mat4 transform = MathUtils::readAsCTransform(entity["transform"]);
  glm::vec3 pos = transform[3];

  glm::vec4 color{1, 1, 1, 0.5f};

  if (isHighlighted) {
    color = {1, 0, 0, 0.5f};
  }
  glm::vec4 solidColor = color;
  solidColor.a = 1;

  GameMember aabbPrimitive = entity["aabbPrimitive"];
  GameMember spherePrimitive = entity["spherePrimitive"];
  GameMember obbTreeGroupPrimitive = entity["obbTreeGroupPrimitive"];
  // this is handled by the TreeGroup, it's always there
//  GameMember obbContainer = entity["obbContainer"];

  translucentRenderBuff->setColor(color);
  translucentRenderBuff->setTransform(transform);
  renderBuff->setColor(solidColor);
  renderBuff->setTransform(transform);

  renderBuff->addLine(glm::vec3{-0.2, 0, 0}, glm::vec3{0.2, 0, 0});
  renderBuff->addLine(glm::vec3{0, -0.2, 0}, glm::vec3{0, 0.2, 0});
  renderBuff->addLine(glm::vec3{0, 0, -0.2}, glm::vec3{0, 0, 0.2});

  if (aabbPrimitive.offset) {
    glm::vec3 min = MathUtils::readAsCVector3f(aabbPrimitive["aabb"]["min"]);
    glm::vec3 max = MathUtils::readAsCVector3f(aabbPrimitive["aabb"]["max"]);
    translucentRenderBuff->addTris(
        ShapeGenerator::generateCube(min, max, color)
    );
  } else if (spherePrimitive.offset) {
    glm::vec3 center = MathUtils::readAsCVector3f(spherePrimitive["sphere"]["origin"]);
    float radius = spherePrimitive["sphere"]["radius"].read_f32();
    translucentRenderBuff->addTris(
        ShapeGenerator::generateSphere(center, radius, color)
    );
  } else if (obbTreeGroupPrimitive.offset) {
    glm::vec3 min = MathUtils::readAsCVector3f(obbTreeGroupPrimitive["container"]["aabb"]["min"]);
    glm::vec3 max = MathUtils::readAsCVector3f(obbTreeGroupPrimitive["container"]["aabb"]["max"]);
    renderBuff->addLines(
        ShapeGenerator::generateCubeLines(min, max, color)
    );
  } else {
    fprintf(stderr, "Uhoh! unknown collision actor!\n");
  }
}

