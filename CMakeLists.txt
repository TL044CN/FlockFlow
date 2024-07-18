cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

if(DEFINED PROJECT_NAME AND NOT "${PROJECT_NAME}" STREQUAL "")
    set(IS_SUBMODULE ON)
else()
    set(IS_SUBMODULE OFF)
endif()

# get the version from git
include(CMakeFiles/HandleGit.cmake)

project(FlockFlow
    VERSION ${SANITIZED_VERSION}
    DESCRIPTION "Threadpool Library"
    LANGUAGES CXX
)


# set the build options
option(${PROJECT_NAME}_BUILD_SHARED "Build Shared Library" OFF)
option(${PROJECT_NAME}_BUILD_TESTS "Build Tests" ON)
option(${PROJECT_NAME}_BUILD_DOCS "Build Documentation" ON)

# modify the build options if this is a submodule
if(IS_SUBMODULE)
    set(${PROJECT_NAME}_BUILD_TESTS OFF CACHE BOOL "Build Tests" FORCE)
    set(${PROJECT_NAME}_BUILD_DOCS OFF CACHE BOOL "Build Documentation" FORCE)
endif()

#print info about the build options
include(CMakeFiles/PrintInfo.cmake)

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


# build documentation
if(${PROJECT_NAME}_BUILD_DOCS)
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

# build tests
if(${PROJECT_NAME}_BUILD_TESTS)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    #find correct coverage system
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        find_program(GCOV gcov)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        find_program(GCOV llvm-cov)
    endif()
    
    #prepare for coverage report
    if(GCOV AND LCOV AND GENHTML)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        endif()
    else()
        if(NOT GCOV)
            message(WARNING "gcov not found. No coverage report will be generated.")
        endif()
        if(NOT LCOV)
            message(WARNING "lcov not found. No coverage report will be generated.")
        endif()
        if(NOT GENHTML)
            message(WARNING "genhtml not found. No coverage report will be generated.")
        endif()
    endif()
        
    # tests:
    enable_testing()
    add_subdirectory(tests)
endif()