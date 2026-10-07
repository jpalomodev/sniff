add_executable(sniff_tests dependency_smoke_test.cpp listing_test.cpp listing_database_test.cpp)
target_link_libraries(sniff_tests PRIVATE
    sniff_project_options
    sniff_persistence
    Catch2::Catch2WithMain
    CURL::libcurl
    nlohmann_json::nlohmann_json
    unofficial::sqlite3::sqlite3
)

include(Catch)
catch_discover_tests(sniff_tests)

add_test(NAME cli_help COMMAND sniff --help)
set_tests_properties(cli_help PROPERTIES PASS_REGULAR_EXPRESSION "--help")
add_test(NAME cli_version COMMAND sniff --version)
set_tests_properties(cli_version PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")
add_test(NAME cli_invalid_option COMMAND sniff --unknown-option)
set_tests_properties(cli_invalid_option PROPERTIES WILL_FAIL TRUE)
