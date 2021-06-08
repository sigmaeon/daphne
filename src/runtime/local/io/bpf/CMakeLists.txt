cmake_minimum_required(VERSION 3.16)

project(daphne-bpf)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-target bpf -g -c")

file(GLOB BPFS "*.c")
add_library(BPFS OBJECT ${BPFS})


foreach(BPF ${BPFS})
  string(REPLACE ".c" "" NAME ${BPF})
  string(REPLACE ${CMAKE_SOURCE_DIR}/ "" NAME ${NAME})
  string(REPLACE "/" "_" TARGET ${NAME})

  add_custom_target(BPF_${TARGET}
    ALL
    DEPENDS BPFS
    COMMAND cp "${CMAKE_SOURCE_DIR}/CMakeFiles/BPFS.dir/${NAME}.c.o" ${NAME}.o
    BYPRODUCTS ${NAME}.o
  )
endforeach()
