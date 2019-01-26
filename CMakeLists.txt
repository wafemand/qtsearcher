cmake_minimum_required(VERSION 3.12)
set(TARGET_NAME qtsearcher)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall --std=c++17 -g -pthread)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        tests/gtest
)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Source files
set(SOURCES
        application/main.cpp
        indexer/index.h
        indexer/indexer.h
        finder/finder.h
        ui/MainWindow.h finder/prefix_function.h)

# User interface files
set(FORMS
        ui/MainWindow.ui)


# Shared libraries
set(LIBRARIES
        Qt5::Widgets
        pthread
        )

# Generate additional sources with MOC and UIC
qt5_wrap_ui(UIC_SOURCES ${FORMS})

# Set target
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS} ${UIC_SOURCES})

# Link with libraries
target_link_libraries(${TARGET_NAME} ${LIBRARIES})

# testing
add_executable(testing
        indexer/index.h
        tests/test_index.cpp
        tests/gtest/gtest-all.cc
        tests/gtest/gtest_main.cc)
target_link_libraries(testing ${LIBRARIES})
