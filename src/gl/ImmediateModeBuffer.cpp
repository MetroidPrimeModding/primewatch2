#include "ImmediateModeBuffer.hpp"

#include <algorithm>
#include <iterator>

ImmediateModeBuffer::ImmediateModeBuffer() {
  linesMesh = make_unique<OpenGLMesh>(
      lineVerts,
      RenderType::LINES,
      BufferUpdateType::STREAM
  );
  triMesh = make_unique<OpenGLMesh>(
      triVerts,
      RenderType::TRIANGLES,
      BufferUpdateType::STREAM
  );
}

void ImmediateModeBuffer::clear() {
  triVerts.clear();
  lineVerts.clear();
}

void ImmediateModeBuffer::draw() {
  triMesh->bufferNewData(triVerts, BufferUpdateType::STREAM);
  linesMesh->bufferNewData(lineVerts, BufferUpdateType::STREAM);

  triMesh->draw();
  linesMesh->draw();
}

void ImmediateModeBuffer::setColor(const glm::vec4 &color) {
  this->currentColor = color;
}

void ImmediateModeBuffer::setTransform(const glm::mat4 &transform) {
  this->vertTransform = transform;
  this->normalTransform = glm::transpose(glm::inverse(this->vertTransform));
}


void ImmediateModeBuffer::addLine(const glm::vec3 &start, const glm::vec3 &end) {
  return addLine(
      Vert{
          .pos = start,
          .color = currentColor,
          .normal = glm::vec3{0, 0, 1},
      },
      Vert{
          .pos = end,
          .color = currentColor,
          .normal = glm::vec3{0, 0, 1},
      }
  );
}

void ImmediateModeBuffer::addLine(const Vert &start, const Vert &end) {
  addLines(std::vector{start, end});
}

void ImmediateModeBuffer::addTri(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
  glm::vec3 n = glm::normalize(glm::cross(a - b, a - c));
  return addTri(
      Vert{
          .pos = a,
          .color = currentColor,
          .normal = n,
      },
      Vert{
          .pos = b,
          .color = currentColor,
          .normal = n,
      },
      Vert{
          .pos = c,
          .color = currentColor,
          .normal = n,
      }
  );
}

void ImmediateModeBuffer::addTri(const Vert &a, const Vert &b, const Vert &c) {
  addTris(std::vector{a, b, c});
}

void ImmediateModeBuffer::addQuad(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &d) {
  glm::vec3 n = glm::normalize(glm::cross(a - b, a - c));
  return addQuad(
      Vert{
          .pos = a,
          .color = currentColor,
          .normal = n,
      },
      Vert{
          .pos = b,
          .color = currentColor,
          .normal = n,
      },
      Vert{
          .pos = c,
          .color = currentColor,
          .normal = n,
      },
      Vert{
          .pos = d,
          .color = currentColor,
          .normal = n,
      }
  );
}

void ImmediateModeBuffer::addQuad(const Vert &a, const Vert &b, const Vert &c, const Vert &d) {
  addTris(std::vector{a, b, c, c, d, a});
}

void ImmediateModeBuffer::addLines(const std::vector<Vert> &newLineVerts) {
  glm::mat4 vt = this->vertTransform;
  glm::mat3 nt = this->normalTransform;
  std::transform(
      newLineVerts.cbegin(), newLineVerts.cend(), std::back_inserter(lineVerts),
      [&vt, &nt](const Vert &in) {
        Vert v{
            .pos = glm::vec3{vt * glm::vec4{in.pos, 1.0f}},
            .color = in.color,
            .normal = glm::normalize(nt * in.normal)
        };
        return v;
      }
  );
}

void ImmediateModeBuffer::addTris(const std::vector<Vert> &newTriVerts) {
  glm::mat4 vt = this->vertTransform;
  glm::mat3 nt = this->normalTransform;
  std::transform(
      newTriVerts.cbegin(), newTriVerts.cend(), std::back_inserter(triVerts),
      [&vt, &nt](const Vert &in) {
        Vert v{
            .pos = glm::vec3{vt * glm::vec4{in.pos, 1.0f}},
            .color = in.color,
            .normal = glm::normalize(nt * in.normal)
        };
        return v;
      }
  );
}
