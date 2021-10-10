#include "OpenGLShader.hpp"

#include <iostream>
#include <glad/glad.h>


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

void OpenGLShader::use() {
  glUseProgram(program);
}
