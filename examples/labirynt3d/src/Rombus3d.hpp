#pragma once

#include <vector>

#include <glm/glm.hpp>

/*           4
 *         / |
 *        0  |      3
 * 2      |  5
 *        | /
 *        1/
 */
const inline std::vector< glm::vec3 > rombus3d_vertices{
    glm::vec3(-0.5F, 0.5F, 0.0F),
    glm::vec3(-0.5F, -0.5F, 0.0F),

    glm::vec3(0.0F, 0.0F, 1.0F),
    glm::vec3(0.0F, 0.0F, -1.0F),

    glm::vec3(0.5F, 0.5F, 0.0F),
    glm::vec3(0.5F, -0.5F, 0.0F),
};

const inline std::vector< unsigned > rombus3d_indices{
    0,
    1,
    2,

    0,
    2,
    4,

    2,
    5,
    4,

    2,
    1,
    5,

    3,
    0,
    4,

    3,
    4,
    5,

    3,
    5,
    1,

    3,
    1,
    0,
};
