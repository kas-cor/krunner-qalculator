project(krunner-qalculator)
cmake_minimum_required(VERSION 2.8.12)
set(PROJECT_VERSION "1.0.0")
set(PROJECT_SOVERSION 0)

add_definitions("-DPROJECT_VERSION=\"${PROJECT_VERSION}\"")

add_definitions(-std=c++11)
option(ENABLE_DEBUG "Enable debug" OFF)
if(ENABLE_DEBUG)
    add_definitions(-g -DDEBUG)
endif()

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS Runner I18n)

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMPackageConfigHelpers)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(krunner_qalculator_SRCS
    qalculatorrunner.cpp
)

add_library(krunner_qalculator MODULE ${krunner_qalculator_SRCS})
target_link_libraries(
    krunner_qalculator
    Qt5::Widgets
    KF5::Runner
    KF5::I18n
)

install(TARGETS krunner_qalculator DESTINATION ${KDE_INSTALL_PLUGINDIR})
