#include "litecomp/repl.hpp"

int main() {
    try {
        std::string line;

        launchRepl();
    }
    catch (const std::exception &) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}