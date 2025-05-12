cmake_minimum_required(VERSION 2.8)

project(fsm_gen)

# Compiler and flags
set(CMAKE_CXX_STANDARD 11)  # Or a more recent standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g") # Add -Wall and -Wextra

set(GEN_SRC autogen/process_state.cpp)

# Source files
set(SRCS app/client.cpp)

include_directories(./include ./autogen)
add_executable(client_app ${SRCS} ${GEN_SRC})