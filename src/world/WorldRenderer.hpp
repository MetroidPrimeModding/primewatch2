#pragma once

#include <map>
#include <vector>
#include <memory>
#include <glm/fwd.hpp>
#include <defs/GameDefinitions.hpp>
#include <gl/OpenGLShader.hpp>
#include "./CollisionMesh.hpp"

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

  glm::vec3 lightDir{0,0.1f,0.9f};

  WorldRenderer();

  void update();
  void render();
  void renderImGui();

private:


  std::unique_ptr<OpenGLShader> shader{};
  std::map<uint32_t, CollisionMesh> mesh_by_mrea{};

  void updateAreas();
  std::optional<CollisionMesh> loadMesh(const GameDefinitions::GameMember &area);
};

