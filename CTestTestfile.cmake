# CMake generated Testfile for 
# Source directory: /home/vagrant/Desktop/spring2017-assignment6-Redavion
# Build directory: /home/vagrant/Desktop/spring2017-assignment6-Redavion
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(correctExecution "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests/test-correct-execution.sh")
set_tests_properties(correctExecution PROPERTIES  WORKING_DIRECTORY "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests")
add_test(parallelExecution "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests/test-parallel-execution.sh")
set_tests_properties(parallelExecution PROPERTIES  WORKING_DIRECTORY "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests")
add_test(incorrectProgram "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests/test-nonexistent-program.sh")
set_tests_properties(incorrectProgram PROPERTIES  WORKING_DIRECTORY "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests")
add_test(background "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests/test-background-execution.sh")
set_tests_properties(background PROPERTIES  WORKING_DIRECTORY "/home/vagrant/Desktop/spring2017-assignment6-Redavion/tests")
