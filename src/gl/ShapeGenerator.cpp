#include "ShapeGenerator.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext/scalar_constants.hpp>

using namespace std;

struct LineSeg {
  glm::vec3 start;
  glm::vec3 end;
};

namespace ShapeGenerator {
  std::vector<Vert> generateCube(glm::vec3 min, glm::vec3 max, glm::vec4 color) {
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

    return verts;
  }

  std::vector<Vert> generateCubeFromCenter(glm::vec3 center, glm::vec3 size, glm::vec4 color) {
    return generateCube(center - size / 2.0f, center + size / 2.0f, color);
  }

  std::vector<Vert> generateSphere(glm::vec3 center, float radius, glm::vec4 color) {
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

    return verts;
  }

  inline glm::vec4 invertHelper(glm::mat4 inv, glm::vec3 v) {
    glm::vec4 res = inv * glm::vec4(v, 1.0f);
    return res / res.w;
  }

  std::vector<Vert> generateCameraLineSegments(glm::mat4 perspective, glm::mat4 transform) {
    vector <Vert> res;

    glm::mat4 inverted = glm::inverse(perspective);
//    glm::mat4 inverted;


    auto addCamLine = [&transform, &inverted, &res](glm::vec3 a, glm::vec3 b, float len, glm::vec4 color) {
      glm::vec4 v1 = invertHelper(inverted, a);
      glm::vec4 v2 = invertHelper(inverted, b);
      glm::vec4 dir = glm::normalize(v2 - v1);

      LineSeg seg{
        .start = glm::vec3(transform * v1),
        .end = glm::vec3(transform * (v1 + dir * len))
      };

      res.emplace_back(Vert{.pos=seg.start, .color=color});
      res.emplace_back(Vert{.pos=seg.end, .color=color});
      
      return seg;
    };

    glm::vec4 red{1, 0, 0, 1};
    glm::vec4 white{1, 1, 1, 1};


//    glm::vec3 pos = glm::unProject(glm::vec3{0, 0, 0}, modelView, perspective, viewport)

    auto center = addCamLine(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1}, 10.0f, red);

    auto bl = addCamLine(glm::vec3{-1, -1, 0}, glm::vec3{-1, -1, 1}, 2.0f, white);
    auto tl = addCamLine(glm::vec3{-1, 1, 0}, glm::vec3{-1, 1, 1}, 2.0f, white);
    auto tr = addCamLine(glm::vec3{1, 1, 0}, glm::vec3{1, 1, 1}, 2.0f, white);
    auto br = addCamLine(glm::vec3{1, -1, 0}, glm::vec3{1, -1, 1}, 2.0f, white);

    res.emplace_back(Vert{.pos=bl.end, .color=white});
    res.emplace_back(Vert{.pos=br.end, .color=white});

    res.emplace_back(Vert{.pos=tl.end, .color=white});
    res.emplace_back(Vert{.pos=tr.end, .color=white});

    res.emplace_back(Vert{.pos=tl.end, .color=white});
    res.emplace_back(Vert{.pos=bl.end, .color=white});

    res.emplace_back(Vert{.pos=tr.end, .color=white});
    res.emplace_back(Vert{.pos=br.end, .color=white});


    return res;
  }
}