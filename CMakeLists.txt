cmake_minimum_required(VERSION 3.20)

project(Camel-Engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_library(CamelEngine STATIC
    src/Logger.cpp
    src/VulkanDevice.cpp
    src/VulkanInstance.cpp
    src/VulkanPhysicalDevice.cpp
    src/VulkanSurface.cpp
)

target_include_directories(CamelEngine
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(CamelEngine
    PUBLIC SDL3::SDL3 Vulkan::Vulkan
)

add_executable(Camel-Engine src/main.cpp)
target_link_libraries(Camel-Engine PRIVATE CamelEngine)
set_target_properties(Camel-Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Camel-Engine>"
)

enable_testing()
add_test(NAME vulkan_init_smoke COMMAND Camel-Engine --smoke-test)
set_tests_properties(vulkan_init_smoke PROPERTIES TIMEOUT 30)
