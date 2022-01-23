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

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

target_link_libraries(${_target} PRIVATE Threads::Threads)
target_link_libraries(${_target} PRIVATE dblv)

set(
  ZMPIR_PUBLIC_HEADERS
  "zmpi_reduce.h"
  "mpi_reduce_rabenseifner.h"
  "mpi_reduce_gather.h"
  "mpi_reduce_pipe.h"
)

set_target_properties(
  ${_target}
  PROPERTIES PUBLIC_HEADER
  "${ZMPIR_PUBLIC_HEADERS}"
)

include(GNUInstallDirs)

install(
  TARGETS ${_target}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
