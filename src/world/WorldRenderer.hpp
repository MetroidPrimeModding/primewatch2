#pragma once

#include <map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <defs/GameDefinitions.hpp>
#include <gl/OpenGLShader.hpp>
#include <gl/ImmediateModeBuffer.hpp>
#include <PrimeWatchInput.hpp>
#include "./CollisionMesh.hpp"

enum class CullType {
  BACK,
  FRONT,
  NONE
};

enum class CameraMode {
  FOLLOW_PLAYER,
  GAME_CAM
};

struct GameCamera {
  glm::mat4 perspective;
  glm::mat4 transform;
  float fov;
  float znear;
  float zfar;
  float aspect;
};

class WorldRenderer {
public:
  float aspect{0};
  float fov{45};
  float zNear{0.1f};
  float zFar{10000.0f};

  float yaw{0};
  float pitch{0.3f};
  float distance{10.f};
  glm::vec3 up{0, 0, 1};

  CullType culling{CullType::BACK};
  CameraMode cameraMode{CameraMode::FOLLOW_PLAYER};

  glm::vec3 lightDir{0.1,0.2f,0.9f};

  bool playerIsMorphed{false};
  glm::vec3 lastKnownNonCollidingPos;
  glm::vec3 playerPos;
  glm::vec3 playerVelocity;
  glm::quat playerOrientation;

  void init();
  void update(const PrimeWatchInput &input);
  void render();
  void renderImGui();

private:
  std::unique_ptr<OpenGLShader> shader{};
  std::unique_ptr<ImmediateModeBuffer> renderBuff;
  std::unique_ptr<ImmediateModeBuffer> translucentRenderBuff;

//  std::unique_ptr<OpenGLMesh> playerUnmorphedMesh{};
//  std::unique_ptr<OpenGLMesh> playerUnmorphedGhostMesh{};
//  std::unique_ptr<OpenGLMesh> playerMorphedMesh{};
//  std::unique_ptr<OpenGLMesh> playerMorphedGhostMesh{};

//  std::unique_ptr<OpenGLMesh> cameraMesh;
//  std::unique_ptr<OpenGLMesh> speedMesh;

  std::map<uint32_t, CollisionMesh> mesh_by_mrea{};

  GameCamera gameCam;

  void updateAreas();
  std::optional<CollisionMesh> loadMesh(const GameDefinitions::GameMember &area);
};


