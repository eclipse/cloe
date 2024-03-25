#include <iostream>

#include <esminiLib.hpp>

int main(int argv, const char** argc) {
    if (argv != 2) {
        std::cerr << "Error: expect argument to path of XOSC file" << std::endl;
        return 2;
    }
    SE_SetLogFilePath("/tmp/esmini_test_package.log");
    SE_ClearPaths();
    SE_Init(argc[1], 0, 0, 0, 0);
    int n_Objects = SE_GetNumberOfObjects();
    SE_Close();
    if (n_Objects != 3) {
        std::cerr << "Error: expected #objects = 3, got " << n_Objects << std::endl;;
        return 1;
    }
    return 0;
}
