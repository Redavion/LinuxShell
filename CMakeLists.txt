cmake_minimum_required(VERSION 2.8)
project('assignment-3')

enable_testing()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -g")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
add_executable (shell main.cpp app.cpp utilities.cpp)


# Check for correct execution
add_test(NAME correctExecution
COMMAND ${PROJECT_SOURCE_DIR}/tests/test-correct-execution.sh
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)

# Check for correct parallel execution
add_test(NAME parallelExecution
COMMAND ${PROJECT_SOURCE_DIR}/tests/test-parallel-execution.sh
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)

# Check that the program correctly reports that exec failed if the program does not exist
add_test(NAME incorrectProgram
COMMAND ${PROJECT_SOURCE_DIR}/tests/test-nonexistent-program.sh
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)


add_test(NAME background
COMMAND ${PROJECT_SOURCE_DIR}/tests/test-background-execution.sh
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)

