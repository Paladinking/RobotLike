cmake_minimum_required(VERSION 3.20)

project(ProcAsm)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

set(ENGINE_DIR ${PROJECT_SOURCE_DIR}/src/engine)

set(ENGINGE_SRC 
    ${ENGINE_DIR}/engine.cpp
    ${ENGINE_DIR}/game.cpp
    ${ENGINE_DIR}/input.cpp
    ${ENGINE_DIR}/random.cpp
    ${ENGINE_DIR}/texture.cpp
    ${ENGINE_DIR}/ui.cpp
    ${ENGINE_DIR}/menu.cpp
    ${ENGINE_DIR}/events.cpp
)

add_executable(embed tools/embed.c)

set(FONT ${CMAKE_SOURCE_DIR}/flexi-ibm-vga-true.regular.ttf)
set(FONT_OBJ ${CMAKE_CURRENT_BINARY_DIR}/font${CMAKE_C_OUTPUT_EXTENSION})

add_executable(main src/main.cpp src/game.cpp src/editbox.cpp
               src/editlines.cpp src/maze.cpp src/language.cpp
               src/parser.cpp src/slime.cpp src/equipment.cpp
               src/player.cpp src/utils.cpp
               ${ENGINGE_SRC} ${FONT_OBJ})

add_custom_command(OUTPUT ${FONT_OBJ} ${PROJECT_SOURCE_DIR}/tools/font.h
        COMMAND embed ARGS ${FONT} -H ${PROJECT_SOURCE_DIR}/tools/font.h -o ${FONT_OBJ} -smono_font
        DEPENDS ${FONT}
)

set(LIBRARIES 
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

target_link_libraries(main PUBLIC ${LIBRARIES})

target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/tools)
