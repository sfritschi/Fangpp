#include <iostream>
#include <exception>

#include <fangpp/graphics.hpp>

int main(void)
{
    try {
        Graphics graphics;
        graphics.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
