cmake_minimum_required(VERSION 3.16)

project(krunner_qalculator_root)

# Set project version
set(PROJECT_VERSION "2.1.0")

# Add options
option(BUILD_TESTING "Build the testing tree." ON)
option(BUILD_ONLY_TESTS "Build only the test target (skip main plugin, no KF6 dependency)" OFF)
option(ENABLE_DEBUG "Enable debug" OFF)

if(BUILD_ONLY_TESTS)
    # CI / test-only mode: skip the main plugin (needs KF6), build only tests.
    # Tests only need Qt6::Test and libqalculate.
    set(BUILD_TESTING ON)
    enable_testing()
    add_subdirectory(tests)
else()
    # Normal mode: build the full plugin + tests
    # src/CMakeLists.txt already handles tests via add_subdirectory(../tests ../tests)
    add_subdirectory(src)
    if(BUILD_TESTING)
        enable_testing()
    endif()
endif()
