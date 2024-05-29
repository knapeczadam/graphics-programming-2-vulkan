#include "scene_config_manager.h"

// Standard includes
#include <fstream>
#include <stdexcept>

namespace dae
{
    void scene_config_manager::load_scene_config(std::string const &file_path)
    {
        std::ifstream file(file_path);
        if (file)
        {
            scene_config_ = json::parse(file);
        }
    }
}
