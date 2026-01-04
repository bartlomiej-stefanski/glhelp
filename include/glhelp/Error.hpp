#pragma once

#include <stdexcept>
#include <string>

namespace glhelp {

struct OpenglError : public std::runtime_error {
  OpenglError(const std::string& where) : std::runtime_error(where) {}
};

void check_gl(const std::string& where);

#define CHECK_GL(where) ::glhelp::check_gl(where)

} // namespace glhelp
