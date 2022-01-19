
set(_target "zmpi_tests")

file(GLOB _srcs *.c)

add_executable(${_target} ${_srcs})

target_link_libraries(${_target} PRIVATE zmpi_reduce dblv)
