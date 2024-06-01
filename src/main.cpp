
// Project includes
#include "engine/scene_config_manager.h"
#include "engine/scene_loader.h"
#include "src/engine/engine.h"

// Standard includes
#include <cstdlib>
#include <iostream>

#define ONE_TAB "\t"
#define TWO_TABS "\t\t"

#define YELLOW_TEXT(text) "\033[1;33m" text "\033[0m"

void print_debug()
{
    const auto onOff = YELLOW_TEXT("(ON/OFF)");
    std::cout << '\n' << YELLOW_TEXT("[Key Bindings]") << '\n';
    std::cout << ONE_TAB << YELLOW_TEXT("[1]") << ONE_TAB << YELLOW_TEXT("Cycle Shading Mode") << TWO_TABS << YELLOW_TEXT("(COMBINED/OBSERVED AREA/DIFFUSE/SPECULAR)") << '\n';
    std::cout << ONE_TAB << YELLOW_TEXT("[2]") << ONE_TAB << YELLOW_TEXT("Toggle NormalMap") << TWO_TABS << onOff << '\n';
}

void load()
{
    dae::scene_config_manager::instance().load_scene_config("configs/scene_config.json");
    dae::scene_loader::instance().load_scenes();
    print_debug();
}

int main()
{
    try
    {
        dae::engine engine{"data/"};
        engine.run(load);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}