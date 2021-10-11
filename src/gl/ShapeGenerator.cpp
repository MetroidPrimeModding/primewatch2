#include "ShapeGenerator.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/scalar_constants.hpp>

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

  std::unique_ptr<OpenGLMesh> generateSphere(glm::vec3 center, float radius, glm::vec4 color) {
    vector<Vert> verts;

    // I wrote this, figuring it out by hand so... probably not optimal

    constexpr int latitudeLines = 15;
    constexpr int longitudeLines = 20;
    constexpr float radiansPerLatitude = glm::pi<float>() / (float) latitudeLines;
    constexpr float radiansPerLongitude = glm::pi<float>() * 2.0f / (float) longitudeLines;

    for (int latitudeLine = 0; latitudeLine < latitudeLines; latitudeLine++) {
      float latitude = radiansPerLatitude * (float) latitudeLine;
      float nextLatitude = radiansPerLatitude * (float) (latitudeLine + 1);

      for (int longitudeLine = 0; longitudeLine < longitudeLines; longitudeLine++) {
        float longitude = radiansPerLongitude * (float) longitudeLine;
        float nextLongitude = radiansPerLongitude * (float) (longitudeLine + 1);

        glm::vec3 topLeft =
            glm::quat(glm::vec3(0, 0, longitude)) *
            glm::quat(glm::vec3(0, latitude, 0)) *
            glm::vec3{0, 0, 1};
        glm::vec3 topRight =
            glm::quat(glm::vec3(0, 0, nextLongitude)) *
            glm::quat(glm::vec3(0, latitude, 0)) *
            glm::vec3{0, 0, 1};
        glm::vec3 bottomLeft =
            glm::quat(glm::vec3(0, 0, longitude)) *
            glm::quat(glm::vec3(0, nextLatitude, 0)) *
            glm::vec3{0, 0, 1};
        glm::vec3 bottomRight =
            glm::quat(glm::vec3(0, 0, nextLongitude)) *
            glm::quat(glm::vec3(0, nextLatitude, 0)) *
            glm::vec3{0, 0, 1};

        glm::vec3 n = glm::normalize(glm::cross(topRight - topLeft, topRight - bottomRight));
        // hack
        if (latitudeLine == 0) {
          n = glm::normalize(glm::cross(topRight - bottomLeft, topRight - bottomRight));
        }

        verts.emplace_back(Vert{.pos = center + topLeft * radius, .color = color, .normal = n});
        verts.emplace_back(Vert{.pos = center + topRight * radius, .color = color, .normal = n});
        verts.emplace_back(Vert{.pos = center + bottomRight * radius, .color = color, .normal = n});

        verts.emplace_back(Vert{.pos = center + bottomRight * radius, .color = color, .normal = n});
        verts.emplace_back(Vert{.pos = center + bottomLeft * radius, .color = color, .normal = n});
        verts.emplace_back(Vert{.pos = center + topLeft * radius, .color = color, .normal = n});
      }
    }

    return make_unique<OpenGLMesh>(verts, RenderType::TRIANGLES);
  }
}