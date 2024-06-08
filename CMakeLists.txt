cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

if(DEFINED PROJECT_NAME AND NOT "${PROJECT_NAME}" STREQUAL "")
    set(IS_SUBMODULE ON)
else()
    set(IS_SUBMODULE OFF)
endif()

find_package(Git)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMODULE_RESULT
        )
        if(NOT GIT_SUBMODULE_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMODULE_RESULT}")
        endif()
    endif()

else()
    set(GIT_TAG_VERSION "1.0.0")
endif()

project(FlockFlow
    VERSION ${GIT_TAG_VERSION}
    DESCRIPTION "Threadpool Library"
    LANGUAGES CXX
)
if(IS_SUBMODULE)
    message("Building ${PROJECT_NAME} version ${PROJECT_VERSION} as Submodule")
else()
    message("Building ${PROJECT_NAME} version ${PROJECT_VERSION}")
endif()

find_program(LCOV lcov)
find_program(GENHTML genhtml)

#find correct coverage system
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    find_program(GCOV gcov)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    find_program(GCOV llvm-cov)
endif()

if(NOT IS_SUBMODULE)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_custom_target(documentation
            COMMAND "doxygen"
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            COMMENT "Generating Doxygen Documentation"
            VERBARIM
        )
    else()
        message("Doxygen required to build Doxygen Documentation")
    endif()
endif()

set(${PROJECT_NAME}_SOURCES
    source/ThreadPool.cpp
)

if(${PROJECT_NAME}_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER include/FlockFlow.hpp
)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    PUBLIC_HEADER DESTINATION include
)

install(
    DIRECTORY include/
    DESTINATION include
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)


#prepare for coverage report
if(NOT ${PROJECT_NAME}_IS_SUBMODULE)
    if(GCOV AND LCOV AND GENHTML)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        endif()
    else()
        if(NOT GCOV)
            message("gcov not found. No coverage report will be generated.")
        endif()
        if(NOT LCOV)
            message("lcov not found. No coverage report will be generated.")
        endif()
        if(NOT GENHTML)
            message("genhtml not found. No coverage report will be generated.")
        endif()
    endif()
endif()

set(${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# tests:
if(NOT IS_SUBMODULE)
    enable_testing()
    add_subdirectory(tests)
endif()
