/// Prelude with all position types
#include <glhelp/position/CachingSimplePosition.hpp>
#include <glhelp/position/FPSPlayerController.hpp>
#include <glhelp/position/FPSSimplePosition.hpp>
#include <glhelp/position/InteractiveController.hpp>
#include <glhelp/position/PlayerController.hpp>
#include <glhelp/position/PositionFollower.hpp>
#include <glhelp/position/PositionProvider.hpp>
#include <glhelp/position/SimplePosition.hpp>

namespace glhelp {

static_assert(PositionProvider< SimplePosition >, "SimplePosition should implement PositionProvider interface");
static_assert(PositionProvider< FPSSimplePosition >, "FPSSimplePosition should implement PositionProvider interface");
static_assert(PositionProvider< PositionFollower< SimplePosition > >, "PositionFollower should implement PositionProvider interface");
static_assert(CachingPositionProvider< CachingSimplePosition >, "SimplePosition should implement PositionProvider interface");
static_assert(PositionController< PlayerController >, "PlayerController should implement PositionProvider interface");
static_assert(PositionController< FPSPlayerController >, "PlayerController should implement PositionProvider interface");
static_assert(PositionController< InteractiveController< PlayerController > >, "PlayerController should implement PositionProvider interface");

} // namespace glhelp
