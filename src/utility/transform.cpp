#include "transform.h"

namespace dae
{
    void transform::set_world_position(float const x, float const y)
    {
        set_world_position(glm::vec3{ x, y, world_position_.z });
    }

    void transform::set_world_position(float const x, float const y, float const z)
    {
        set_world_position(glm::vec3{ x, y, z });
    }

    void transform::set_world_position(glm::vec2 const &position)
    {
        set_world_position(glm::vec3{ position, world_position_.z });
    }

    void transform::set_world_position(glm::vec3 const &position)
    {
        world_position_ = position;
    }

    void transform::set_local_position(float const x, float const y)
    {
        set_local_position(glm::vec3{ x, y, local_position_.z });
    }

    void transform::set_local_position(float const x, float const y, float const z)
    {
        set_local_position(glm::vec3{ x, y, z });
    }

    void transform::set_local_position(glm::vec2 const &position)
    {
        set_local_position(glm::vec3{ position, local_position_.z });
    }

    void transform::set_local_position(glm::vec3 const &position)
    {
        local_position_ = position;
    }
}
