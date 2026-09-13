cmake_minimum_required(VERSION 3.10)

foreach(shader IN ITEMS VERT_SHADER FRAG_SHADER)
    if(NOT DEFINED ${shader})
        message(FATAL_ERROR "${shader} was not provided")
    endif()

    if(NOT EXISTS "${${shader}}")
        message(FATAL_ERROR "Shader file does not exist: ${${shader}}")
    endif()

    file(SIZE "${${shader}}" shader_size)
    if(shader_size LESS 4)
        message(FATAL_ERROR "Shader file is empty or truncated: ${${shader}}")
    endif()

    file(READ "${${shader}}" shader_magic LIMIT 4 HEX)
    string(TOLOWER "${shader_magic}" shader_magic)
    if(NOT shader_magic STREQUAL "03022307")
        message(FATAL_ERROR "Invalid SPIR-V magic number in ${${shader}}: ${shader_magic}")
    endif()

    message(STATUS "Valid SPIR-V shader: ${${shader}} (${shader_size} bytes)")
endforeach()
