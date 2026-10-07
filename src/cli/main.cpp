#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"Sniff"};
    app.set_version_flag("--version", SNIFF_VERSION);

    try {
        app.parse(argc, argv);
        std::cout << app.help();
        return 0;
    } catch (const CLI::ParseError& error) {
        return app.exit(error);
    } catch (const std::exception& error) {
        spdlog::error("Startup failed: {}", error.what());
        return 1;
    }
}
