#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <gl/OpenGLMesh.hpp>
#include <memory>

enum class CollisionMaterial: uint32_t {
  UNKNOWN_1 = 0x1,
  STONE = 0x2,
  METAL = 0x4,
  GRASS = 0x8,
  ICE = 0x10,
  PILLAR = 0x20,
  METAL_GRATING = 0x40,
  PHAZON = 0x80,
  DIRT = 0x100,
  LAVA = 0x200,
  UNKNOWN_2 = 0x400,
  SNOW = 0x800,
  SLOW_MUD = 0x1000,
  HALFPIPE = 0x2000,
  MUD = 0x4000,
  GLASS = 0x8000,
  SHIELD = 0x10000,
  SAND = 0x20000,
  SHOOT_THRU = 0x40000,
  SOLID = 0x80000,
  UNKNOWN_3 = 0x100000,
  CAMERA_THRU = 0x200000,
  WOOD = 0x400000,
  ORGANIC = 0x800000,
  UNKNOWN_4 = 0x1000000,
  REDUNDANT_EDGE = 0x2000000,
  FLIPPED_TRI = 0x2000000,
  SEE_THRU = 0x4000000,
  SCAN_THRU = 0x8000000,
  AI_WALK_THRU = 0x10000000,
  CEILING = 0x20000000,
  WALL = 0x40000000,
  FLOOR = 0x80000000,
};

class CollisionMesh {
public:
  std::vector<glm::vec3> raw_verts;
  std::vector<uint16_t> raw_vert_materials;

  std::vector<glm::vec<2, glm::uint16_t, glm::defaultp>> raw_edges;
  std::vector<uint16_t> raw_edge_materials;

  std::vector<glm::vec<3, glm::uint16_t, glm::defaultp>> raw_polys;
  std::vector<uint16_t> raw_poly_materials;


  std::vector<uint32_t> materials;

  void initGlMesh();
  void draw();

private:
  std::unique_ptr<OpenGLMesh> mesh{nullptr};
};
