
// Project includes
#include "core/app.h"

// Standard includes
#include <cstdlib>
#include <iostream>

int main()
{
    dae::app app{};

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