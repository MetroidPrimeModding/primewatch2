#include "ShapeGenerator.hpp"

#include <vector>

using namespace std;

namespace ShapeGenerator {
  unique_ptr<OpenGLMesh> generateCube(glm::vec3 min, glm::vec3 max, glm::vec4 color) {
    vector<Vert> verts;

    // -Z
    verts.emplace_back(Vert{.pos = {max.x, max.y, min.z}, .color = color, .normal = {0, 0, -1},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, min.z}, .color = color, .normal = {0, 0, -1},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, min.z}, .color = color, .normal = {0, 0, -1},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, min.z}, .color = color, .normal = {0, 0, -1},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, min.z}, .color = color, .normal = {0, 0, -1},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, min.z}, .color = color, .normal = {0, 0, -1},});


    // +Z
    verts.emplace_back(Vert{.pos = {min.x, min.y, max.z}, .color = color, .normal = {0, 0, 1},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, max.z}, .color = color, .normal = {0, 0, 1},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, max.z}, .color = color, .normal = {0, 0, 1},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, max.z}, .color = color, .normal = {0, 0, 1},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, max.z}, .color = color, .normal = {0, 0, 1},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, max.z}, .color = color, .normal = {0, 0, 1},});

    // -X
    verts.emplace_back(Vert{.pos = {min.x, min.y, min.z}, .color = color, .normal = {-1, 0, 0},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, min.z}, .color = color, .normal = {-1, 0, 0},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, max.z}, .color = color, .normal = {-1, 0, 0},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, max.z}, .color = color, .normal = {-1, 0, 0},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, max.z}, .color = color, .normal = {-1, 0, 0},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, min.z}, .color = color, .normal = {-1, 0, 0},});

    // +X
    verts.emplace_back(Vert{.pos = {max.x, max.y, max.z}, .color = color, .normal = {1, 0, 0},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, min.z}, .color = color, .normal = {1, 0, 0},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, min.z}, .color = color, .normal = {1, 0, 0},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, min.z}, .color = color, .normal = {1, 0, 0},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, max.z}, .color = color, .normal = {1, 0, 0},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, max.z}, .color = color, .normal = {1, 0, 0},});

    // -Y
    verts.emplace_back(Vert{.pos = {max.x, min.y, max.z}, .color = color, .normal = {0, -1, 0},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, min.z}, .color = color, .normal = {0, -1, 0},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, min.z}, .color = color, .normal = {0, -1, 0},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, min.z}, .color = color, .normal = {0, -1, 0},});
    verts.emplace_back(Vert{.pos = {min.x, min.y, max.z}, .color = color, .normal = {0, -1, 0},});
    verts.emplace_back(Vert{.pos = {max.x, min.y, max.z}, .color = color, .normal = {0, -1, 0},});

    // +Y
    verts.emplace_back(Vert{.pos = {min.x, max.y, min.z}, .color = color, .normal = {0, 1, 0},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, min.z}, .color = color, .normal = {0, 1, 0},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, max.z}, .color = color, .normal = {0, 1, 0},});
    verts.emplace_back(Vert{.pos = {max.x, max.y, max.z}, .color = color, .normal = {0, 1, 0},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, max.z}, .color = color, .normal = {0, 1, 0},});
    verts.emplace_back(Vert{.pos = {min.x, max.y, min.z}, .color = color, .normal = {0, 1, 0},});

    return make_unique<OpenGLMesh>(verts, RenderType::TRIANGLES);
  }

  std::unique_ptr<OpenGLMesh> generateCubeFromCenter(glm::vec3 center, glm::vec3 size, glm::vec4 color) {
    return std::move(generateCube(center - size / 2.0f, center + size / 2.0f, color));
  }
}