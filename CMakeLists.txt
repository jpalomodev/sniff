cmake_minimum_required(VERSION 3.25)

project(sniff VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_library(sniff_project_options INTERFACE)
target_compile_features(sniff_project_options INTERFACE cxx_std_23)
target_compile_options(sniff_project_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
)

add_executable(sniff src/cli/main.cpp)
target_link_libraries(sniff PRIVATE sniff_project_options CLI11::CLI11 spdlog::spdlog)
target_compile_definitions(sniff PRIVATE SNIFF_VERSION="${PROJECT_VERSION}")

if(BUILD_TESTING)
    find_package(Catch2 3 CONFIG REQUIRED)
    add_subdirectory(tests)
endif()
