# print a message when a condition is true
function(print_conditional_status CONDITION MSG_ON MSG_OFF)
    if(${CONDITION})
        message(STATUS ${MSG_ON})
    else()
        message(STATUS ${MSG_OFF})
    endif()
endfunction()

# print the project information
message(STATUS "==========|${PROJECT_NAME}|=======>>")
message(STATUS "Version:      ${PROJECT_VERSION}")
message(STATUS "Git Tag:      ${GIT_TAG_VERSION}")
print_conditional_status(IS_SUBMODULE "Is Submodule: Yes" "Is Submodule: No")
print_conditional_status(${PROJECT_NAME}_BUILD_SHARED
    "Library Type: Shared"
    "Library Type: Static"
)
print_conditional_status(${PROJECT_NAME}_BUILD_TESTS
    "Build Tests:  Yes" 
    "Build Tests:  No"
)
print_conditional_status(${PROJECT_NAME}_BUILD_DOCS
    "Build Docs:   Yes"
    "Build Docs:   No"
)
message(STATUS "<<========|${PROJECT_NAME}|=========")