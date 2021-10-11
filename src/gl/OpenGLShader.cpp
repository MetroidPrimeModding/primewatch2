#include "OpenGLShader.hpp"

#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>


OpenGLShader::OpenGLShader(const char *vertSrc, const char *fragSrc) {
  int success;
  char infoLog[512];

  unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, &vertSrc, nullptr);
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
    std::cout << "Vert shader failed" << std::endl << infoLog << std::endl;
    exit(1);
  }

  unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, &fragSrc, nullptr);
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
    std::cout << "Frag shader failed" << std::endl << infoLog << std::endl;
    exit(1);
  }

  program = glCreateProgram();
  glAttachShader(program, fragShader);
  glAttachShader(program, vertShader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, nullptr, infoLog);
    std::cout << "Shader link failed" << std::endl << infoLog << std::endl;
    exit(1);
  }

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
}

OpenGLShader::~OpenGLShader() {
  if (program != 0) {
    glDeleteProgram(program);
  }
}

void OpenGLShader::use() const {
  glUseProgram(program);
}

void OpenGLShader::setBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(program, name.c_str()), (int) value);
}

void OpenGLShader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}

void OpenGLShader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}

void OpenGLShader::setVec2(const std::string &name, const glm::vec2 &value) const {
  glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value));
}

void OpenGLShader::setVec3(const std::string &name, const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value));
}

void OpenGLShader::setVec4(const std::string &name, const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(value));
}

void OpenGLShader::setMat2(const std::string &name, const glm::mat2 &value) const {
  glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLShader::setMat3(const std::string &name, const glm::mat3 &value) const {
  glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void OpenGLShader::setMat4(const std::string &name, const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
