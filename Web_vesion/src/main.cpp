#include <exception>
#include <iostream>

#include "Application.h"

int main() {
    try {
        cosmosim::Application application;
        if (!application.initialize()) {
            std::cerr << "CosmoSim 3D could not start. See the messages above.\n";
            return 1;
        }
        return application.run();
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 2;
    } catch (...) {
        std::cerr << "Fatal unknown error.\n";
        return 3;
    }
}
