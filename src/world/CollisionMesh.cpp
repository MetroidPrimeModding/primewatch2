#include "CollisionMesh.hpp"

using namespace std;

void CollisionMesh::initGlMesh() {
  vector<Tri> tris{};

  for (int i = 0; i < raw_polys.size(); i++) {
    auto polyFlags = materials[raw_poly_materials[i]];
    auto edges = raw_polys[i];
    auto line1 = raw_edges[edges[0]];
    auto line2 = raw_edges[edges[1]];
    auto line3 = raw_edges[edges[2]];

    uint16_t i1, i2, i3;
    auto otherLine = line1;
    // point 1
    i1 = line1[0];

    // Point 2
    if (line1[0] == line2[0]) {
      i2 = line2[1];
      otherLine = line3;
    } else if (line1[0] == line2[1]) {
      i2 = line2[0];
      otherLine = line3;
    } else if (line1[0] == line3[0]) {
      i2 = line3[1];
      otherLine = line2;
    } else {
      i2 = line3[0];
      otherLine = line2;
    }

    // point 3
    if (i2 == otherLine[0]) {
      i3 = otherLine[1];
    } else {
      i3 = otherLine[0];
    }

    // swap if needed
    if (!!(polyFlags & ECollisionMaterial::FLIPPED_TRI)) {
      uint16_t tmp = i1;
      i1 = i3;
      i3 = tmp;
    }

    auto v1 = raw_verts[i1];
    auto v2 = raw_verts[i2];
    auto v3 = raw_verts[i3];

    auto n = glm::normalize(glm::cross(v1 - v3, v1 - v2));

    glm::vec4 color{1,1,1,1};

    // this is how the game calculates standability
    if (!!(polyFlags & ECollisionMaterial::FLOOR) || n.z > 0.85) {
      color = glm::vec4{0.5f,1.0f,0.5f,1.0f};
    } else if (!!(polyFlags & ECollisionMaterial::WALL) || n.z > 0.85) {
      color = glm::vec4{0.5f,0.5f,1.0f,1.0f};
    } else if (!!(polyFlags & ECollisionMaterial::CEILING) || n.z > 0.85) {
      color = glm::vec4{1.0f,0.5f,0.5f,1.0f};
    }


    tris.emplace_back(Tri{
        .a = {
            .pos = v1,
            .color = color,
            .normal = n
        },
        .b = {
            .pos = v2,
            .color = color,
            .normal = n
        },
        .c = {
            .pos = v3,
            .color = color,
            .normal = n
        }
    });
  }

  this->mesh = make_unique<OpenGLMesh>(tris);
}

void CollisionMesh::draw() {
  if (this->mesh) {
    this->mesh->draw();
  }
}
