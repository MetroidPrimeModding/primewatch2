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
  void update();
  void render();
  void renderImGui();

private:
  std::unique_ptr<OpenGLShader> shader{};
  std::map<uint32_t, CollisionMesh> mesh_by_mrea{};
  void updateAreas();
  std::optional<CollisionMesh> loadMesh(const GameDefinitions::GameMember &area);
};


