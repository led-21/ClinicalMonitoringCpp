cmake_minimum_required(VERSION 3.20)

project(ClinicalMonitoringCpp
    VERSION 1.0.0
    DESCRIPTION "ICU Clinical Patient Monitoring and NEWS2 Calculation System in Modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable folder grouping for IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------
# Compiler Warnings and Flags
# ---------------------------------------------------------------------------
add_library(project_warnings INTERFACE)
if(MSVC)
    target_compile_options(project_warnings INTERFACE
        /W4
        /permissive-
        /utf-8
    )
else()
    target_compile_options(project_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnon-virtual-dtor
    )
endif()

# ---------------------------------------------------------------------------
# Core Clinical Library
# ---------------------------------------------------------------------------
set(CLINICAL_CORE_SOURCES
    src/NewsCalculator.cpp
    src/ICU.cpp
    src/ScoreUpdateNotifier.cpp
    src/Dashboard.cpp
    src/LedIndicator.cpp
)

set(CLINICAL_CORE_HEADERS
    include/clinical/Models.h
    include/clinical/Patient.h
    include/clinical/NewsCalculator.h
    include/clinical/ICU.h
    include/clinical/ScoreObserver.h
    include/clinical/ScoreUpdateNotifier.h
    include/clinical/Dashboard.h
    include/clinical/LedIndicator.h
)

add_library(clinical_core STATIC
    ${CLINICAL_CORE_SOURCES}
    ${CLINICAL_CORE_HEADERS}
)

target_include_directories(clinical_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(clinical_core
    PRIVATE
        project_warnings
)

# ---------------------------------------------------------------------------
# Console Application
# ---------------------------------------------------------------------------
add_executable(icu_monitor_app
    app/main.cpp
    app/ConsoleUI.h
    app/ConsoleUI.cpp
)

target_include_directories(icu_monitor_app
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app
)

target_link_libraries(icu_monitor_app
    PRIVATE
        clinical_core
        project_warnings
)

# ---------------------------------------------------------------------------
# Automated Testing Suite (CTest)
# ---------------------------------------------------------------------------
enable_testing()

add_executable(unit_tests
    tests/main.cpp
    tests/TestFramework.h
    tests/NewsCalculatorTests.h
    tests/NewsCalculatorTests.cpp
    tests/IcuBedManagementTests.h
    tests/IcuBedManagementTests.cpp
    tests/ObserverNotificationTests.h
    tests/ObserverNotificationTests.cpp
)

target_include_directories(unit_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

target_link_libraries(unit_tests
    PRIVATE
        clinical_core
        project_warnings
)

add_test(NAME ClinicalMonitoringTestSuite COMMAND unit_tests)
