cmake_minimum_required(VERSION 3.10)
project(AVR_Emulator CXX)

# C++ Standard und Warnungen global definieren
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall)
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address -fsanitize=undefined")
    
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

add_library(avr_lib STATIC
    "source/avr.cpp"
    "source/instr.cpp"
    "source/io.cpp"
    "source/filter.cpp"
    "source/atmegaXX8.cpp"
    "source/atmega8.cpp"
    "source/attinyX5.cpp"
    "source/attinyX4.cpp"
    "source/atxmegaAU.cpp"
    "source/atmega2560.cpp"
)

add_executable(AVRemu "source/main.cpp" "source/execute.cpp")
target_link_libraries(AVRemu PRIVATE "avr_lib")

add_executable(AVRmatch "source/match.cpp")
target_link_libraries(AVRmatch PRIVATE "avr_lib")

add_executable(AVRtest "source/test.cpp")
target_link_libraries(AVRtest PRIVATE "avr_lib")

find_program(CTAGS_PATH ctags)
if (CTAGS_PATH)
    add_custom_target(tags
        COMMAND ${CTAGS_PATH} -R ${CMAKE_CURRENT_SOURCE_DIR}/source/*.h ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
