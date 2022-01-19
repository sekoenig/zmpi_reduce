
set(_target "dblv")

file(GLOB _srcs *.c)

add_library(${_target} STATIC ${_srcs})

target_include_directories(${_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(USE_MPI)
  find_package(MPI REQUIRED)

  target_compile_definitions(${_target} PUBLIC USE_MPI)
  target_link_libraries(${_target} PRIVATE MPI::MPI_C)
endif()
