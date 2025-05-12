cmake_minimum_required(VERSION 2.8)

project(fsm_gen)

# Compiler and flags
set(CMAKE_CXX_STANDARD 11)  # Or a more recent standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g") # Add -Wall and -Wextra


# Source files
set(SRCS parser.cpp)

# Output binary (if you want to create an executable as well)
set(BINARY fsm_gen)

include_directories(/usr/include/libxml2)

# Create the executable
add_executable(${BINARY} ${SRCS})
target_link_libraries(${BINARY} xml2)
