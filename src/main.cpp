
// Project includes
#include "engine/scene_config_manager.h"
#include "engine/scene_loader.h"
#include "src/engine/engine.h"
#include "src/utility/utils.h"

// Standard includes
#include <cstdlib>
#include <iostream>

void print_debug()
{
    auto const on_off = LEFT_PAR MAGENTA_TEXT("ON") SLASH MAGENTA_TEXT("OFF") RIGHT_PAR;
    std::cout << '\n' << YELLOW_TEXT("[Key Bindings]") << '\n';
    std::cout << ONE_TAB << YELLOW_TEXT("[1]") << ONE_TAB << GREEN_TEXT("Cycle Shading Mode") << TWO_TABS << LEFT_PAR << MAGENTA_TEXT("COMBINED") << SLASH << MAGENTA_TEXT("OBSERVED AREA") << SLASH << MAGENTA_TEXT("DIFFUSE") << SLASH << MAGENTA_TEXT("SPECULAR") << RIGHT_PAR << '\n';
    std::cout << ONE_TAB << YELLOW_TEXT("[2]") << ONE_TAB << GREEN_TEXT("Toggle NormalMap") << TWO_TABS << on_off << '\n';
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