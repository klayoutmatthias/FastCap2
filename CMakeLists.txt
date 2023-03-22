cmake_minimum_required(VERSION 3.5)

project(FastCap2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FastCap2
  src/mulGlobal.h src/input.h src/zbuf2fastcap.h src/mulMulti.h src/mulMats.h
  src/mulSetup.h src/calcp.h src/capsolve.h src/mulDisplay.h src/direct.h
  src/mulDo.h src/zbufInOut.h src/zbufProj.h src/zbufSort.h src/patran.h src/patran_f.h
  src/quickif.h src/mulLocal.h
  src/fastcap.cc
  src/mulGlobal.cc
  src/mulSetup.cc src/mulMats.cc src/mulDo.cc
  src/mulDisplay.cc src/mulMulti.cc
  src/mulLocal.cc src/direct.cc src/blkDirect.cc src/calcp.cc
  src/uglyalloc.cc
  src/patran.cc src/quickif.cc src/electric.cc
  src/input.cc src/psMatDisplay.cc src/capsolve.cc
  src/zbufInOut.cc src/zbuf2fastcap.cc src/zbufProj.cc src/zbufSort.cc
)

target_link_libraries(FastCap2 m)
target_compile_options(FastCap2 PRIVATE -Wall -pedantic)
target_compile_definitions(FastCap2 PRIVATE DIRSOL=1 EXPGCR=1)

install(TARGETS FastCap2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
