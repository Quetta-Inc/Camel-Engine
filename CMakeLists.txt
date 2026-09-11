cmake_minimum_required(VERSION 3.10)

project(Camel-Engine CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_program(GLSLANG_VALIDATOR NAMES glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin" "D:/VulkanSDK/Bin" "D:/VulkanSDK/1.4.357.0/Bin")

if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found in PATH or VULKAN_SDK")
endif()

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shaders)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/frag.spv
    COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER_DIR}/triangle.vert -o ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/vert.spv
    COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER_DIR}/triangle.frag -o ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/frag.spv
    DEPENDS ${SHADER_DIR}/triangle.vert ${SHADER_DIR}/triangle.frag
    COMMENT "Compiling Vulkan triangle shaders"
    VERBATIM
)

add_custom_target(vulkan_shaders DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/vert.spv
    ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/frag.spv
)

add_executable(${PROJECT_NAME} main.cpp Source/vulkan.cpp)
add_dependencies(${PROJECT_NAME} vulkan_shaders)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/vert.spv
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/vert.spv
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders/frag.spv
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/frag.spv
    COMMENT "Copying shaders to executable directory"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    CAMEL_ENGINE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    CAMEL_ENGINE_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})