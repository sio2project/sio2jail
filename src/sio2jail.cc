#include "common/Exception.h"
#include "s2japp/Application.h"

#include <iostream>
#include <sys/wait.h>

int main(int argc, const char* argv[]) {
    try {
        int ret = s2j::app::Application(argc, argv).main();
        while (wait(NULL)>0){}
        return ret;
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception occurred: " << ex.what() << std::endl;
        return s2j::app::Application::ExitCode::FATAL_ERROR;
    }
}
