#include "common/Exception.h"
#include "s2japp/Application.h"

#include <iostream>

int main(int argc, const char* argv[]) {
    try {
        return s2j::app::Application(argc, argv).main();
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception occurred: " << ex.what() << std::endl;
        return s2j::app::Application::ExitCode::FATAL_ERROR;
    }
}
