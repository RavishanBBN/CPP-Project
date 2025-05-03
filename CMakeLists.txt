cmake_minimum_required(VERSION 3.10...3.24)
project(QuantumMeltdownSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy config.json to build dir
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.json
  ${CMAKE_CURRENT_BINARY_DIR}/config.json
  COPYONLY
)

# (Optional) OpenMP
find_package(OpenMP REQUIRED)

# FetchContent for nlohmann/json & gtest
include(FetchContent)
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(json googletest)

# Build core library
add_library(quantum_simulator
    src/Particle.cpp
    src/ContainmentField.cpp
    src/Simulation.cpp
    src/ThreadManager.cpp      # ← include your new file
)

target_include_directories(quantum_simulator PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${json_SOURCE_DIR}/include
)

target_link_libraries(quantum_simulator PUBLIC OpenMP::OpenMP_CXX)

# Build the app
add_executable(quantum_simulator_app src/main.cpp)
target_link_libraries(quantum_simulator_app PRIVATE quantum_simulator)
