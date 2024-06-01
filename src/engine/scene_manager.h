#pragma once

// Project includes
#include "src/system/i_system.h"
#include "src/utility/singleton.h"

// Standard includes
#include <memory>
#include <string>
#include <vector>

namespace dae
{
    // Forward declarations
    class game_object;
    class scene;
    
    class descriptor_set_layout;
    
    class scene_manager final : public singleton<scene_manager>
    {
    public:
        ~scene_manager() override;

        scene_manager(scene_manager const &other)            = delete;
        scene_manager(scene_manager &&other)                 = delete;
        scene_manager &operator=(scene_manager const &other) = delete;
        scene_manager &operator=(scene_manager &&other)      = delete;

        void update();
        void render();

        [[nodiscard]] auto find(std::string const &name) -> scene *;

        auto create_scene(std::string const &name, std::unique_ptr<i_system> system) -> scene *;

    private:
        friend class singleton<scene_manager>;
        scene_manager();
        
        std::vector<std::unique_ptr<scene>> scenes_;
    };
}
