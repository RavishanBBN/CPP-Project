cmake_minimum_required(VERSION 3.10)
project(QuantumMeltdownSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- copy config.json into build dir ---------------------------------
configure_file(config.json config.json COPYONLY)

# --- OpenMP -----------------------------------------------------------
find_package(OpenMP REQUIRED)

# --- third‑party: nlohmann/json + googletest --------------------------
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

# --- simulator static library ----------------------------------------
add_library(quantum_simulator STATIC
    src/Particle.cpp
    src/ContainmentField.cpp
    src/ThreadManager.cpp
    src/Simulation.cpp             # ← ←  keep this!
)

target_include_directories(quantum_simulator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${json_SOURCE_DIR}/include
)

target_link_libraries(quantum_simulator
    PUBLIC OpenMP::OpenMP_CXX       # OpenMP visible to dependents
)

# --- main application -------------------------------------------------
add_executable(quantum_simulator_app
    src/main.cpp
)

target_link_libraries(quantum_simulator_app
    PRIVATE quantum_simulator       # ← ←  link the library!
)

# --- (optional) tests -------------------------------------------------
enable_testing()
add_executable(quantum_tests
    tests/ParticleTests.cpp
)
target_link_libraries(quantum_tests
    PRIVATE quantum_simulator gtest_main
)
add_test(NAME ParticleTests COMMAND quantum_tests)
