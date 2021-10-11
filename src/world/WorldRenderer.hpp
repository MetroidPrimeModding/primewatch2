#pragma once

#include <map>
#include <vector>
#include <memory>
#include <glm/fwd.hpp>
#include <defs/GameDefinitions.hpp>
#include <gl/OpenGLShader.hpp>
#include <PrimeWatchInput.hpp>
#include "./CollisionMesh.hpp"

enum class CullType {
  BACK,
  FRONT,
  NONE
};

class WorldRenderer {
public:
  float aspect{0};
  float fov{45};
  float zNear{0.1f};
  float zFar{10000.0f};

  glm::vec3 lookAt{0.0, 0.0, 0.0};
  float yaw{0};
  float pitch{0.3f};
  float distance{10.f};
  glm::vec3 up{0, 0, 1};

  CullType culling{CullType::BACK};

  glm::vec3 lightDir{0.1,0.2f,0.9f};

  void init();
  void update(const PrimeWatchInput &input);
  void render();
  void renderImGui();

private:


  std::unique_ptr<OpenGLShader> shader{};
  std::unique_ptr<OpenGLMesh> playerUnmorphedMesh{};
  std::map<uint32_t, CollisionMesh> mesh_by_mrea{};

  void updateAreas();
  std::optional<CollisionMesh> loadMesh(const GameDefinitions::GameMember &area);
};


