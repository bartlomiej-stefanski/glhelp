#pragma once

#include <vector>

#include <glm/glm.hpp>

/*    7-----4
 *   /|   / |
 *  3----0  |
 *  | 6--|--5
 *  |/   | /
 *  2----1/
 */
const inline std::vector< glm::vec3 > cube_vertices{
    glm::vec3(1, 1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(-1, -1, 1),
    glm::vec3(-1, 1, 1),

    glm::vec3(1, 1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, -1, -1),
    glm::vec3(-1, 1, -1),
};

const inline std::vector< unsigned > cube_indices{
    // front
    0,
    2,
    1,
    2,
    0,
    3,
    // back
    5,
    6,
    7,
    5,
    7,
    4,
    // right
    0,
    1,
    4,
    1,
    5,
    4,
    // left
    2,
    3,
    6,
    6,
    3,
    7,
    // top
    0,
    4,
    7,
    0,
    7,
    3,
    // bottom
    1,
    2,
    5,
    2,
    6,
    5,
};

const inline std::vector< unsigned > cube_indices_rev{
    // front
    0,
    1,
    2,
    2,
    3,
    0,
    // back
    5,
    7,
    6,
    5,
    4,
    7,
    // right
    0,
    4,
    1,
    1,
    4,
    5,
    // left
    2,
    6,
    3,
    6,
    7,
    3,
    // top
    0,
    7,
    4,
    0,
    3,
    7,
    // bottom
    1,
    5,
    2,
    2,
    5,
    6,
};
