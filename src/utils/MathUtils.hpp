#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "defs/GameDefinitions.hpp"

namespace MathUtils {
  glm::vec3 readAsCVector3f(const GameDefinitions::GameMember &member);
  glm::quat readAsCQuaternion(const GameDefinitions::GameMember &member);
  glm::mat4 readAsCMatrix4f(const GameDefinitions::GameMember &member);
  glm::mat4 readAsCTransform(const GameDefinitions::GameMember &member);
}


