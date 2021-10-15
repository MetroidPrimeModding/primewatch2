#include <GameMemory.h>
#include "MathUtils.hpp"

glm::vec3 MathUtils::readAsCVector3f(const GameDefinitions::GameMember &member) {
  return glm::vec3{
      member["x"].read_f32(),
      member["y"].read_f32(),
      member["z"].read_f32()
  };
}

glm::quat MathUtils::readAsCQuaternion(const GameDefinitions::GameMember &member) {
  return glm::quat{
      member["x"].read_f32(),
      member["y"].read_f32(),
      member["z"].read_f32(),
      member["w"].read_f32()
  };
}

glm::mat4 MathUtils::readAsCMatrix4f(const GameDefinitions::GameMember &member) {
  uint32_t offset = member["matrix"].offset;

#define RC(r, c) (r + c * 4) * 4

  return glm::mat4{
      GameMemory::read_float(offset + RC(0, 0)),
      GameMemory::read_float(offset + RC(0, 1)),
      GameMemory::read_float(offset + RC(0, 2)),
      GameMemory::read_float(offset + RC(0, 3)),

      GameMemory::read_float(offset + RC(1, 0)),
      GameMemory::read_float(offset + RC(1, 1)),
      GameMemory::read_float(offset + RC(1, 2)),
      GameMemory::read_float(offset + RC(1, 3)),

      GameMemory::read_float(offset + RC(2, 0)),
      GameMemory::read_float(offset + RC(2, 1)),
      GameMemory::read_float(offset + RC(2, 2)),
      GameMemory::read_float(offset + RC(2, 3)),

      GameMemory::read_float(offset + RC(3, 0)),
      GameMemory::read_float(offset + RC(3, 1)),
      GameMemory::read_float(offset + RC(3, 2)),
      GameMemory::read_float(offset + RC(3, 3)),
  };

#undef RC
}

glm::mat4 MathUtils::readAsCTransform(const GameDefinitions::GameMember &member) {
  uint32_t offset = member["m0"].offset;

#define RC(r, c) (r + c * 4) * 4

  return glm::mat4{
      GameMemory::read_float(offset + RC(0, 0)),
      GameMemory::read_float(offset + RC(0, 1)),
      GameMemory::read_float(offset + RC(0, 2)),
      0.0f,

      GameMemory::read_float(offset + RC(1, 0)),
      GameMemory::read_float(offset + RC(1, 1)),
      GameMemory::read_float(offset + RC(1, 2)),
      0.0f,

      GameMemory::read_float(offset + RC(2, 0)),
      GameMemory::read_float(offset + RC(2, 1)),
      GameMemory::read_float(offset + RC(2, 2)),
      0.0f,

      GameMemory::read_float(offset + RC(3, 0)),
      GameMemory::read_float(offset + RC(3, 1)),
      GameMemory::read_float(offset + RC(3, 2)),
      1.0f,
  };

#undef RC
}
