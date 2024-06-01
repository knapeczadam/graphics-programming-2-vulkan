#pragma once

// Project includes
#include "src/utility/singleton.h"

// Standard includes
#include <string>

// JSON includes
#if defined(CMAKE_BUILD)
#include <single_include/nlohmann/json.hpp>
#else
#include "json.hpp"
#endif

namespace dae
{
    // Aliases
    using json = nlohmann::json;
    
    class scene_config_manager final : public singleton<scene_config_manager>
    {
    public:
        ~scene_config_manager() override = default;

        scene_config_manager(scene_config_manager const& other)            = delete;
        scene_config_manager(scene_config_manager &&other)                 = delete;
        scene_config_manager &operator=(scene_config_manager const &other) = delete;
        scene_config_manager &operator=(scene_config_manager &&other)      = delete;
        
        void load_scene_config(std::string const &file_path);
        [[nodiscard]] auto scene_config() const -> json const & { return scene_config_; }

    private:
        friend class singleton<scene_config_manager>;
        scene_config_manager() = default;

    private:
        json scene_config_;
    };
}
