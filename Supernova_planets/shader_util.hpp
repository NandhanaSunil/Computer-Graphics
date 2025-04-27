#ifndef _SHADER_UTIL_HPP_
#define _SHADER_UTIL_HPP_

#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "common.hpp"

namespace glFramework
{
  GLuint LoadShaderGL(GLenum eShaderType, const std::string &strFilename);
  GLuint CreateShaderGL(GLenum eShaderType, const std::string &strShaderFile);
  GLuint CreateProgramGL(const std::vector<GLuint> &shaderList);
};

#endif