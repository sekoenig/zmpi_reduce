cmake_minimum_required(VERSION 3.15)
project(zmpi_reduce)

set(USE_MPI TRUE)

if(USE_MPI)
  find_package(MPI REQUIRED)

  add_compile_definitions(USE_MPI)
  link_libraries(MPI::MPI_C)
endif()

add_subdirectory(dblv)
add_subdirectory(reduce)
add_subdirectory(tests)
