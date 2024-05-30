#pragma once

// Project includes
#include "src/core/game_object.h"

// Standard includes
#include <memory>
#include <string>
#include <vector>

namespace dae
{
    // Forward declarations
    class game_object;
    class scene_manager;
    class i_system;

    class descriptor_set_layout;

    class scene final
    {
        friend class scene_manager;

    public:
        ~scene();
        
        scene(scene const &other)            = delete;
        scene(scene &&other)                 = delete;
        scene &operator=(scene const &other) = delete;
        scene &operator=(scene &&other)      = delete;

        void update();
        void render() const;

        [[nodiscard]] auto name() const -> std::string const & { return name_; }

        auto create_game_object(std::string const &name = "new_game_object") -> game_object *;
        [[nodiscard]] auto objects() const -> std::vector<game_object*>;

    private:
        scene();
        explicit scene(std::string name, std::unique_ptr<i_system> system);

        std::string name_;
        std::vector<std::unique_ptr<game_object>> objects_{};
        std::unique_ptr<i_system> system_{};
    };
}
