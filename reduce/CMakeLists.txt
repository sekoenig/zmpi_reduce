
set(_target "zmpi_reduce")

file(GLOB _srcs *.c)

add_library(${_target} STATIC ${_srcs})

target_include_directories(${_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(${_target} PUBLIC USE_DBLV)

if(USE_MPI)
  find_package(MPI REQUIRED)

  target_compile_definitions(${_target} PUBLIC USE_MPI)
  target_link_libraries(${_target} PRIVATE MPI::MPI_C)
endif()

target_link_libraries(${_target} PRIVATE dblv)
