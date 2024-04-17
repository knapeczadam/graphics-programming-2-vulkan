
// Project includes
#include "lve_app.h"

// Standard includes
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    dae::lve_app app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}