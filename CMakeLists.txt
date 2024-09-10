cmake_minimum_required(VERSION 3.30)

include(FetchContent)
include(FindVulkan)

project(KhronosTutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GLFW_BUILD_WAYLAND ON)
set(GLFW_BUILD_X11 OFF)
# option(TRACY_ENABLE "" ON)
# option(TRACY_ON_DEMAND "" ON)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

# FetchContent_Declare (
#   tracy
#   GIT_REPOSITORY https://github.com/wolfpld/tracy.git
#   GIT_TAG v0.11.0
#   GIT_SHALLOW TRUE
#   GIT_PROGRESS TRUE
# )

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG release
)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

FetchContent_MakeAvailable(glm glfw tinyobjloader)
# FetchContent_MakeAvailable(glm glfw tracy)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

file(GLOB SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

add_definitions(-DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE -DGLFW_INCLUDE_VULKAN)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm ${Vulkan_LIBRARIES} tinyobjloader)
target_compile_definitions(${PROJECT_NAME} PRIVATE TINYOBJLOADER_IMPLEMENTATION)
# target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm ${Vulkan_LIBRARIES} TracyClient)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/compiled
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.sh
    DEPENDS ${SHADERS}
    COMMENT "Compiling shaders..."
)

add_custom_target(compile_shaders ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders/compiled)
add_dependencies(${PROJECT_NAME} compile_shaders)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILES_MATCHING PATTERN "*.spv")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})