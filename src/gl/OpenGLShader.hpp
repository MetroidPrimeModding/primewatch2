#pragma once


class OpenGLShader {
public:
  OpenGLShader(const char *vert, const char *frag);
  ~OpenGLShader();

  OpenGLShader(const OpenGLShader &) = delete;
  OpenGLShader &operator=(const OpenGLShader &) = delete;

  void use();
private:
  unsigned int program{0};
};


