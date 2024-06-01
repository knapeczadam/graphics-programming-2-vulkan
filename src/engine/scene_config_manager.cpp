#include "scene_config_manager.h"

// Project includes
#include "engine.h"

// Standard includes
#include <fstream>
#include <stdexcept>

#if defined(CMAKE_BUILD)
#ifndef ENGINE_DIR
#define ENGINE_DIR "../../../"
#endif
#else
#ifndef ENGINE_DIR
#define ENGINE_DIR ""
#endif
#endif

namespace dae
{
    void scene_config_manager::load_scene_config(std::string const &file_path)
    {
        std::ifstream file(ENGINE_DIR + engine::data_path + file_path);
        if (file)
        {
            scene_config_ = json::parse(file);
        }
    }
}
