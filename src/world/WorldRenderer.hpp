#pragma once

#include <map>
#include <vector>
#include <glm/fwd.hpp>
#include "./CollisionMesh.hpp"

class WorldRenderer {
public:
  void update();
  void render();
  void renderImGui();

private:
  std::map<glm::uint32, CollisionMesh> mesh_by_mrea{};
  void updateAreas() const;
};


