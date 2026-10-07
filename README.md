# Sniff

Sniff is a private resale intelligence tool for parcel-shippable professional electronics,
initially portable test-and-measurement equipment. Read [MVP.md](MVP.md) for product
requirements and [AGENTS.md](AGENTS.md) for engineering guidance. Both are living documents.

## Current State

This is a buildable project skeleton. The executable supports help and version output;
discovery, identification, valuation, persistence, and transaction workflows are pending.
No marketplace or AI provider has been selected.

## Current Stack

Use C++23 with GCC or Clang, CMake 3.25 or newer, Ninja, and vcpkg manifest mode.
The manifest includes SQLite, libcurl with TLS, nlohmann/json, CLI11, and spdlog.
Catch2 is enabled by the `tests` manifest feature in the supplied presets.
The vcpkg registry baseline is pinned; update it deliberately when changing dependencies.

AI clients will use direct HTTP/JSON adapters. Marketplace adapters should use C++ where
practical. A TypeScript/Playwright sidecar can be introduced if browser automation becomes
necessary. The stack and directory layout can evolve with project requirements.

## Setup and Build

Install GCC or Clang with C++23 support, CMake, Ninja, Git, and the prerequisites required
by vcpkg ports. On Linux, these commonly include a C/C++ toolchain, curl, zip/unzip, tar,
pkg-config, and Perl for OpenSSL. Initial dependency installation needs network access.

Use an existing vcpkg checkout by setting `VCPKG_ROOT`, or create a local ignored checkout:

```bash
git clone https://github.com/microsoft/vcpkg.git .tools/vcpkg
git -C .tools/vcpkg checkout 2cfff9c458d9dcf642e9fa09ba624f9931bb5358
.tools/vcpkg/bootstrap-vcpkg.sh -disableMetrics
export VCPKG_ROOT="$PWD/.tools/vcpkg"
```

Configure, build, and test with Ninja:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
./build/debug/sniff --help
./build/debug/sniff --version
```

The `release` configure/build/test presets use a separate `build/release` directory.
To choose Clang, set `CC=clang CXX=clang++` when configuring a fresh build directory.
For machine-specific paths, compilers, or triplets, use an ignored `CMakeUserPresets.json`
that inherits from the supplied presets. Do not change compilers in an existing build tree.

For a build without tests:

```bash
cmake --preset release -DBUILD_TESTING=OFF -DVCPKG_MANIFEST_FEATURES=
cmake --build --preset release
```

Microsoft documents [manifest mode](https://learn.microsoft.com/en-us/vcpkg/consume/manifest-mode)
and [CMake integration](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration).

## Directory Layout

```text
src/
  cli/          CLI parsing and presentation
  core/         Domain types and deterministic business logic, added as needed
  adapters/     Persistence, HTTP, AI, and marketplace integrations, added as needed
tests/          Offline Catch2 tests and CLI smoke checks
```

Core and adapter directories currently document intended boundaries. Add compiled targets
when they contain actual behavior. Keep identification independent of valuation, and
source-specific code independent of core rules. Add `workers/` only if a browser sidecar
is needed; no Node dependencies are installed now.

## Development Conventions

Use namespace `sniff`, descriptive `snake_case` file/function/variable names, and `PascalCase`
type names. Use four spaces and the checked-in `.clang-format` style. Keep headers
self-contained and use `#pragma once`. Build targets enable GCC/Clang warnings without
forcing warnings-as-errors on third-party dependencies.

Name test files `*_test.cpp`. Tests must run without live marketplaces or model APIs.
The initial suite checks CLI behavior, SQLite/JSON evidence round-tripping, and libcurl
initialization. These verify the scaffold, not an implemented sourcing workflow. As logic
is added, prioritize financial correctness, malformed input, uncertainty, and recovery.
Catch2 cases are registered with CTest using
[Catch2's CMake integration](https://catch2-temp.readthedocs.io/en/latest/cmake-integration.html).

Keep local databases and media under ignored `data/`, and credentials/session material
under ignored `secrets/` or in the environment. Review staged files before committing;
ignore rules do not replace careful handling of secrets.

```bash
git diff --check
git status --short
```
