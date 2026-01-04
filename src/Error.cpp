#include <glad/gl.h>

#include <glhelp/Error.hpp>

namespace glhelp {

void check_gl(const std::string& where)
{
  GLenum error{glGetError()};
  if (error != GL_NO_ERROR)
    throw OpenglError(where + ": OpenGL error code " + std::to_string(error));
}

} // namespace glhelp
