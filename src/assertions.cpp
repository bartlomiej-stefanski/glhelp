#include <glhelp/primitives/PlayerController.hpp>
#include <glhelp/primitives/PositionFollower.hpp>
#include <glhelp/primitives/PositionProvider.hpp>
#include <glhelp/primitives/SimplePosition.hpp>
#include <glhelp/primitives/mesh2d.hpp>

namespace glhelp {

static_assert(PositionProvider< PlayerController >, "PlayerController should implement PositionProvider interface");
static_assert(PositionProvider< SimplePosition >, "SimplePosition should implement PositionProvider interface");
static_assert(PositionProvider< PositionFollower< SimplePosition > >, "PositionFollower should implement PositionProvider interface");

} // namespace glhelp
