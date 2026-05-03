cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR iOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(GlobalEditor VERSION 1.0.0)
add_definitions(-DWIN32_LEAN_AND_MEAN)


# --- ЗАВАНТАЖЕННЯ МЕРЕЖЕВОЇ БІБЛІОТЕКИ ---
include(FetchContent)
FetchContent_Declare(
    asio
    URL "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-28-0.zip"
)
FetchContent_MakeAvailable(asio)
# -----------------------------------------


set(GEODE_CLI "D:/MultiplayerEditor/my-collab/geode.exe")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Вказуємо шлях до завантаженої asio
target_include_directories(${PROJECT_NAME} PUBLIC ${asio_SOURCE_DIR}/asio/include)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})
