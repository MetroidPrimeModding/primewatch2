#include "CollisionMesh.hpp"

using namespace std;

void CollisionMesh::initGlMesh() {
  vector<Tri> tris{
      {
          .a={.pos={-0.5f, -0.5f, 0.0f}, .color={1, 1, 1, 1}, .normal={0, 0, -1}},
          .b={.pos={0.5f, -0.5f, 0.0f}, .color={1, 1, 1, 1}, .normal={0, 0, -1}},
          .c={.pos={0.0f, 0.5f, 0.0f}, .color={1, 1, 1, 1}, .normal={0, 0, -1}}
      }
  };

  this->mesh = make_unique<OpenGLMesh>(tris);
}

void CollisionMesh::draw() {
  if (this->mesh) {
    this->mesh->draw();
  }
}
