#include "adapters/persistence/listing_database.hpp"

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"Sniff"};
    app.set_version_flag("--version", SNIFF_VERSION);
    std::string database_path = "data/sniff.db";
    auto* init_db =
        app.add_subcommand("init-db", "Open a persistent database and initialize its schema");
    init_db->add_option("--path", database_path, "SQLite database file")->capture_default_str();
    init_db->callback([&] {
        sniff::persistence::ListingDatabase database{database_path};
        std::cout << "Database ready: " << database_path << '\n';
    });

    try {
        app.parse(argc, argv);
        if (app.get_subcommands().empty()) {
            std::cout << app.help();
        }
        return 0;
    } catch (const CLI::ParseError& error) {
        return app.exit(error);
    } catch (const std::exception& error) {
        spdlog::error("Command failed: {}", error.what());
        return 1;
    }
}
