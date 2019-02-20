cmake_minimum_required(VERSION 3.12)

project(turn-based LANGUAGES CXX)

set(RENDERER console CACHE STRING "The rendering system to use")
set_property(CACHE RENDERER PROPERTY STRINGS console sfml)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(RENDERER STREQUAL "sfml")
    find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
endif()

set(SOURCE_FILES
    src/battle/action.h
    src/battle/battlesystem.cpp
    src/battle/battlesystem.h
    src/battle/battleview.h
    src/battle/controller.h
    src/battle/element.h
    src/battle/entity.cpp
    src/battle/entity.h
    src/battle/messages.h
    src/battle/npccontroller.cpp
    src/battle/npccontroller.h
    src/battle/playercontroller.cpp
    src/battle/playercontroller.h
    src/battle/skill.cpp
    src/battle/skill.h
    src/battle/skillhooks.h
    src/battle/skillref.h
    src/battle/stats.h
    src/battle/stats.cpp
    src/battle/statuseffect.h
    src/battle/statuseffect.cpp
    src/util.h
    src/random.h
)

if(RENDERER STREQUAL "console")
    list(APPEND SOURCE_FILES
        src/conmain.cpp
    )
else()
    list(APPEND SOURCE_FILES
        src/main.cpp
        src/game.cpp
        src/game.h
    )
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})


set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(MSVC)
    string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive-)
    target_compile_options(${PROJECT_NAME} PRIVATE /diagnostics:caret)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zi)
else()  # most likely Clang or GCC
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -pedantic
        -Wnon-virtual-dtor)  # TODO: add more warnings

    # TODO: replace with generator expressions?
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined,address)
        target_link_libraries(${PROJECT_NAME} asan ubsan)
    endif()
endif()


if(RENDERER STREQUAL "sfml")
    target_link_libraries(${PROJECT_NAME} sfml-graphics)
endif()
