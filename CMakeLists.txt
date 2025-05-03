cmake_minimum_required(VERSION 3.10...3.24)
project(QuantumMeltdownSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy config.json into build dir
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.json
  ${CMAKE_CURRENT_BINARY_DIR}/config.json
  COPYONLY
)

# Find OpenMP for parallel tasks
find_package(OpenMP REQUIRED)

# Fetch third-party libs
include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

FetchContent_MakeAvailable(json googletest)

# Build core simulator library
add_library(quantum_simulator STATIC
    src/Particle.cpp
    src/ContainmentField.cpp
    src/ThreadManager.cpp
    src/Simulation.cpp       # ← Make sure Simulation.cpp is here!
)

target_include_directories(quantum_simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${json_SOURCE_DIR}/include
)

target_link_libraries(quantum_simulator PUBLIC OpenMP::OpenMP_CXX)

# Build the main application
add_executable(quantum_simulator_app
    src/main.cpp
)

target_link_libraries(quantum_simulator_app PRIVATE quantum_simulator)

# (Optional) Tests
enable_testing()
add_executable(quantum_tests
    tests/ParticleTests.cpp
    tests/SimulationTests.cpp   # ← if you have tests for Simulation
)
target_link_libraries(quantum_tests PRIVATE quantum_simulator gtest_main)
add_test(NAME ParticleTests COMMAND quantum_tests)
