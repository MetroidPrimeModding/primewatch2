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
    if ((polyFlags & ECollisionMaterial::FLIPPED_TRI) == ECollisionMaterial::FLIPPED_TRI) {
      uint16_t tmp = i1;
      i1 = i3;
      i3 = tmp;
    }

    auto v1 = raw_verts[i1];
    auto v2 = raw_verts[i2];
    auto v3 = raw_verts[i3];

    auto n = glm::normalize(glm::cross(v1 - v3, v1 - v2));

    tris.emplace_back(Tri{
        .a = {
            .pos = v1,
            .color = {1,1,1,1},
            .normal = n
        },
        .b = {
            .pos = v2,
            .color = {1,1,1,1},
            .normal = n
        },
        .c = {
            .pos = v3,
            .color = {1,1,1,1},
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
