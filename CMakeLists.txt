cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (cs_signaling
	VERSION 0.1
	DESCRIPTION "Railroad signaling case study")

find_package(Threads REQUIRED)

include_directories(include)
set (SRC src/main.cpp
         src/system.cpp
         src/edge.cpp
         src/node.cpp
         src/rrsignal.cpp
         src/train.cpp)

# This project will output an executable file
add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(cs_signaling Threads::Threads)

# Create a simple configuration header
configure_file(config.h.in config.h)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
