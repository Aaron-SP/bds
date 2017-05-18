#include <iostream>
#include <tworldmesh.h>

int main()
{
    try
    {
        bool out = true;
        out = out && test_world_mesh();
        if (out)
        {
            std::cout << "Game tests passed!" << std::endl;
            return 0;
        }
    }
    catch (std::exception &ex)
    {
        std::cout << ex.what() << std::endl;
    }

    std::cout << "Game tests failed!" << std::endl;
    return -1;
}