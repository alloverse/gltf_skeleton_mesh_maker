cmake_minimum_required(VERSION 3.10)

# Set the project name
project(gltf_to_skeletonbox)

include_directories(SYSTEM
    "${PROJECT_SOURCE_DIR}/tinygltf"
)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source file
add_executable(gltf_to_skeletonbox 
main.cpp
octree.cpp
octree.h
point.h
)



# Add the library
add_subdirectory(tinygltf)

target_link_libraries(gltf_to_skeletonbox tinygltf)
