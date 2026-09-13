cmake_minimum_required(VERSION 3.10)

project(Camel-Engine CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME} main.cpp Source/vulkan.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

add_test(
    NAME vulkan_init_smoke
    COMMAND $<TARGET_FILE:${PROJECT_NAME}> --smoke-test
)

set_tests_properties(vulkan_init_smoke PROPERTIES
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    TIMEOUT 30
)
