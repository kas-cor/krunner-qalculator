cmake_minimum_required(VERSION 3.16)

project(krunner-qalculator)

set(PROJECT_VERSION "2.0.1")
set(PROJECT_SOVERSION 0)
set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/kas-cor/krunner-qalculator")
set(CMAKE_PROJECT_AUTHOR "kas-cor")
set(CMAKE_PROJECT_AUTHOR_EMAIL "kascorp@gmail.com")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(ECM 5.240.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets)
find_package(KF6 REQUIRED COMPONENTS Runner I18n CoreAddons Config)
find_package(PkgConfig REQUIRED)

pkg_check_modules(QALCULATE REQUIRED IMPORTED_TARGET libqalculate)

add_definitions("-DPROJECT_VERSION=\"${PROJECT_VERSION}\"")

option(ENABLE_DEBUG "Enable debug" OFF)
if(ENABLE_DEBUG)
    add_definitions(-g -DDEBUG)
endif()

kcoreaddons_add_plugin(krunner_qalculator SOURCES qalculatorrunner.cpp INSTALL_NAMESPACE "kf6/krunner")

target_link_libraries(krunner_qalculator
    Qt6::Core
    Qt6::Widgets
    KF6::Runner
    KF6::I18n
    KF6::CoreAddons
    KF6::ConfigCore
    PkgConfig::QALCULATE
)

# Enable testing if not disabled
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(../tests ../tests)
endif()
