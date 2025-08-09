#pragma once

#include <map>
#include <vector>
#include <set>
#include <array>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <defs/GameDefinitions.hpp>
#include <gl/OpenGLShader.hpp>
#include <gl/ImmediateModeBuffer.hpp>
#include <PrimeWatchInput.hpp>
#include "./CollisionMesh.hpp"
#include "utils/GameObjectUtils.hpp"

enum class CullType {
  BACK,
  FRONT,
  NONE
};

enum class CameraMode {
  FOLLOW_PLAYER,
  DETATCHED,
  GAME_CAM
};

enum class OrbitPlayerCameraOrigin {
  TOP,
  CENTER,
  BOTTOM
};

struct GameCamera {
  glm::mat4 perspective;
  glm::mat4 transform;
  float fov;
  float znear;
  float zfar;
  float aspect;
};

struct TriggerRenderConfig {
  bool detectPlayer: 1{true};
  bool detectAi: 1{false};
  bool detectProjectiles: 1{false};
  bool detectBombs: 1{false};
  bool detectPowerBombs: 1{false};
  bool killOnEnter: 1{false};
  bool detectMorphedPlayer: 1{false};
  bool useCollisionImpluses: 1{false};
  bool detectCamera: 1{false};
  bool useBooleanIntersection: 1{false};
  bool detectUnmorphedPlayer: 1{true};
  bool blockEnvironmentalEffects: 1{false};
  bool water: 1{true};
  bool docks: 1{true};
};

struct ActorRenderConfig {
  bool renderProjectiles: 1{true};
  bool renderAI: 1{true};
  bool renderPickups: 1{true};
  bool renderCollisionActors: 1{true};
  bool renderPhysicsActors: 1{false};
  bool renderActors: 1{false};
  bool renderAllActors: 1{false};
};

struct PlayerGhost {
  bool enabled{false};
  glm::vec3 position;
  glm::quat orientation;
  glm::vec3 velocity;
  bool isMorphed{false};
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
  glm::vec3 manualCameraPos;
  float manualCameraSpeed{1.0f};
  glm::mat4 camProjection{1.0f};
  glm::mat4 camView{1.0f};
  glm::vec3 camEye;
  glm::quat camPointing;
  glm::vec4 camViewport;
  float camLineLength{10.f};

  CullType culling{CullType::BACK};
  CameraMode cameraMode{CameraMode::FOLLOW_PLAYER};
  OrbitPlayerCameraOrigin orbitPlayerCameraOrigin{OrbitPlayerCameraOrigin::CENTER};
  TriggerRenderConfig triggerRenderConfig;
  ActorRenderConfig actorRenderConfig;

  glm::vec3 lightDir{0.1, 0.2f, 0.9f};

  glm::vec3 lastKnownNonCollidingPos;
  glm::vec3 playerLookVec;

  PlayerGhost player;
  std::array<PlayerGhost, 5> playerGhosts;

  void init();
  void update(const PrimeWatchInput &input);\
  void
  render(const std::map<TUniqueID, GameDefinitions::GameMember> &entities, const std::set<uint16_t> &highlightedEids);
  void renderImGui();

private:
  std::unique_ptr<OpenGLShader> meshShader{};
  std::unique_ptr<OpenGLShader> lineShader{};
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
  void renderEntities(const std::map<TUniqueID, GameDefinitions::GameMember> &entities,
                      const std::set<uint16_t> &highlightedEntities);
  void drawPlayer(const PlayerGhost &ghost, glm::vec4 color);
  void drawTrigger(const GameDefinitions::GameMember &entity, bool isHighlighted);
  void drawDock(const GameDefinitions::GameMember &entity, bool isHighlighted);
  void drawActor(const GameDefinitions::GameMember &member, bool highlighted);
  void drawPhysicsActor(const GameDefinitions::GameMember &member, bool highlighted);
  void drawChozoGhost(const GameDefinitions::GameMember &ghost, bool highlighted,
                      const std::map<TUniqueID, GameDefinitions::GameMember> &entities);
  void drawProjectile(const GameDefinitions::GameMember &member, bool highlighted);
  void drawAi(const GameDefinitions::GameMember &member, bool highlighted);
  void drawPickup(const GameDefinitions::GameMember &member, bool highlighted);
  glm::vec3 getScreenspacePosForPhysicsActor(const GameDefinitions::GameMember &physicsActor);
  void drawCollisionActor(const GameDefinitions::GameMember &entity, bool isHighlighted);
};


