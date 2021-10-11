#pragma once

#include <string>
#include <glm/glm.hpp>

class OpenGLShader {
public:
  OpenGLShader(const char *vert, const char *frag);
  ~OpenGLShader();

  OpenGLShader(const OpenGLShader &) = delete;
  OpenGLShader &operator=(const OpenGLShader &) = delete;

  void use() const;

  void setBool(const std::__cxx11::basic_string<char> &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec2(const std::string &name, const glm::vec2 &value) const;
  void setVec3(const std::string &name, const glm::vec3 &value) const;
  void setVec4(const std::string &name, const glm::vec4 &value) const;
  void setMat2(const std::string &name, const glm::mat2 &value) const;
  void setMat3(const std::string &name, const glm::mat3 &value) const;
  void setMat4(const std::string &name, const glm::mat4 &value) const;
private:
  unsigned int program{0};
};


