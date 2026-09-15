cmake_minimum_required(VERSION 3.20)

project(Camel-Engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

find_program(GLSLANG_VALIDATOR
    NAMES glslangValidator
    HINTS "$ENV{VULKAN_SDK}/Bin" "D:/VulkanSDK/Bin" "D:/VulkanSDK/1.4.357.0/Bin"
)

if(NOT GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found in PATH or VULKAN_SDK")
endif()

set(CAMEL_ENGINE_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders")
set(CAMEL_ENGINE_SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated_shaders")
set(CAMEL_ENGINE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
file(MAKE_DIRECTORY "${CAMEL_ENGINE_SHADER_BINARY_DIR}")

add_custom_command(
    OUTPUT
        "${CAMEL_ENGINE_SHADER_BINARY_DIR}/vert.spv"
        "${CAMEL_ENGINE_SHADER_BINARY_DIR}/frag.spv"
    COMMAND "${GLSLANG_VALIDATOR}" -V "${CAMEL_ENGINE_SHADER_SOURCE_DIR}/triangle.vert"
            -o "${CAMEL_ENGINE_SHADER_BINARY_DIR}/vert.spv"
    COMMAND "${GLSLANG_VALIDATOR}" -V "${CAMEL_ENGINE_SHADER_SOURCE_DIR}/triangle.frag"
            -o "${CAMEL_ENGINE_SHADER_BINARY_DIR}/frag.spv"
    DEPENDS
        "${CAMEL_ENGINE_SHADER_SOURCE_DIR}/triangle.vert"
        "${CAMEL_ENGINE_SHADER_SOURCE_DIR}/triangle.frag"
    COMMENT "Compiling Vulkan triangle shaders"
    VERBATIM
)

add_custom_target(camel_engine_shaders DEPENDS
    "${CAMEL_ENGINE_SHADER_BINARY_DIR}/vert.spv"
    "${CAMEL_ENGINE_SHADER_BINARY_DIR}/frag.spv"
)

add_library(CamelEngine STATIC
    src/Logger.cpp
    src/Primitive.cpp
    src/VulkanBuffer.cpp
    src/VulkanCommandExecutor.cpp
    src/VulkanDevice.cpp
    src/VulkanDescriptors.cpp
    src/VulkanDepthBuffer.cpp
    src/VulkanFramebuffers.cpp
    src/VulkanImageViews.cpp
    src/VulkanInstance.cpp
    src/VulkanPipeline.cpp
    src/VulkanPhysicalDevice.cpp
    src/VulkanRenderPass.cpp
    src/VulkanRenderer.cpp
    src/VulkanSurface.cpp
    src/VulkanSwapchain.cpp
    src/VulkanTexture.cpp
)

add_dependencies(CamelEngine camel_engine_shaders)

target_include_directories(CamelEngine
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external/stb" ${Vulkan_INCLUDE_DIR}
)

target_compile_definitions(CamelEngine PRIVATE
    CAMEL_ENGINE_SHADER_DIR="${CAMEL_ENGINE_SHADER_BINARY_DIR}"
    CAMEL_ENGINE_ASSET_DIR="${CAMEL_ENGINE_ASSET_DIR}"
)

target_link_libraries(CamelEngine
    PUBLIC SDL3::SDL3 glm::glm Vulkan::Vulkan
)

add_executable(Camel-Engine src/main.cpp)
target_link_libraries(Camel-Engine PRIVATE CamelEngine)
add_dependencies(Camel-Engine camel_engine_shaders)
target_compile_definitions(Camel-Engine PRIVATE
    CAMEL_ENGINE_SHADER_DIR="${CAMEL_ENGINE_SHADER_BINARY_DIR}"
    CAMEL_ENGINE_ASSET_DIR="${CAMEL_ENGINE_ASSET_DIR}"
)
set_target_properties(Camel-Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Camel-Engine>"
)

enable_testing()
add_test(NAME vulkan_triangle_smoke COMMAND Camel-Engine --smoke-test)
set_tests_properties(vulkan_triangle_smoke PROPERTIES TIMEOUT 30)
