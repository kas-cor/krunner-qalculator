# Test CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(qalculator_runner_tests)

find_package(Qt6 REQUIRED COMPONENTS Core Test)
find_package(PkgConfig REQUIRED)

pkg_check_modules(QALCULATE REQUIRED IMPORTED_TARGET libqalculate)

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test executable — uses libqalculate C++ API directly
add_executable(test_qalculatorrunner
    test_qalculatorrunner.cpp
)

# Keep test binary in build/bin/ (consistent with main plugin build via KDECMakeSettings)
set_target_properties(test_qalculatorrunner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(test_qalculatorrunner
    Qt6::Core
    Qt6::Test
    PkgConfig::QALCULATE
)

# Enable automatic MOC
set_property(TARGET test_qalculatorrunner PROPERTY AUTOMOC ON)

# Register the test
add_test(NAME test_qalculatorrunner COMMAND test_qalculatorrunner)
