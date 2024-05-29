#pragma once

// GLM includes
#include <glm/glm.hpp>

namespace dae
{
    class transform final
    {
    public:
        [[nodiscard]] auto world_position() const -> const glm::vec3 & { return world_position_; }
        [[nodiscard]] auto local_position() const -> const glm::vec3 & { return local_position_; }
        
        void set_world_position(float const x, float const y);
        void set_world_position(float const x, float const y, float const z);
        void set_world_position(glm::vec2 const &position);
        void set_world_position(glm::vec3 const &position);

        void set_local_position(float const x, float const y);
        void set_local_position(float const x, float const y, float const z);
        void set_local_position(glm::vec2 const &position);
        void set_local_position(glm::vec3 const &position);

    private:
        glm::vec3 world_position_ = {};
        glm::vec3 local_position_ = {};
    };
}
