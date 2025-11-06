cmake_minimum_required(VERSION 3.20)
project(ImGuiXmlOop)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# TinyXML2 sources
set(TINYXML2_DIR ${CMAKE_SOURCE_DIR}/thirdparty/tinyxml2)
set(TINYXML2_SOURCES ${TINYXML2_DIR}/tinyxml2.cpp)

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/thirdparty/imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# Yoga sources
set(YOGA_DIR ${CMAKE_SOURCE_DIR}/thirdparty/yoga)
file(GLOB_RECURSE YOGA_SOURCES 
    ${YOGA_DIR}/yoga/*.cpp
)
list(FILTER YOGA_SOURCES EXCLUDE REGEX ".*test.*")

set(CORE_SOURCES
    Widget.cpp
    Panel.cpp
)

# Create executable
add_executable(imgui_oop_app
    main.cpp
    XmlParser.cpp
    ${CORE_SOURCES}
    ${IMGUI_SOURCES}
    ${YOGA_SOURCES}
    ${TINYXML2_SOURCES}
)

# Include directories
target_include_directories(imgui_oop_app PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${YOGA_DIR}
    ${TINYXML2_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(imgui_oop_app 
    ${SDL2_LIBRARIES}
)

# Compiler flags
target_compile_options(imgui_oop_app PRIVATE ${SDL2_CFLAGS_OTHER})

add_executable(imgui_builder
    builder_main.cpp
    CityDataPanelBuilder.cpp
    ${CORE_SOURCES}
    ${IMGUI_SOURCES}
    ${YOGA_SOURCES}
)

target_include_directories(imgui_builder PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${YOGA_DIR}
    ${TINYXML2_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(imgui_builder
    ${SDL2_LIBRARIES}
)

target_compile_options(imgui_builder PRIVATE ${SDL2_CFLAGS_OTHER})
