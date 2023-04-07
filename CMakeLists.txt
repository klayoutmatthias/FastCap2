cmake_minimum_required(VERSION 3.5)

set(COMPILE_OPTIONS -Wall -pedantic -fPIC)

# --------------------------------------------------------------
# corelib

add_library(corelib STATIC
  src/blkDirect.h
  src/calcp.h
  src/capsolve.h
  src/counters.h
  src/direct.h
  src/heap.h
  src/input.h
  src/mulDisplay.h
  src/mulDo.h
  src/mulGlobal.h
  src/mulLocal.h
  src/mulMats.h
  src/mulMulti.h
  src/mulSetup.h
  src/mulStruct.h
  src/patran_f.h
  src/patran.h
  src/psMatDisplay.h
  src/quickif.h
  src/savemat_mod.h
  src/zbuf2fastcap.h
  src/zbufInOut.h
  src/zbufProj.h
  src/zbufSort.h
  src/blkDirect.cc
  src/calcp.cc
  src/capsolve.cc
  src/counters.cc
  src/direct.cc
  src/electric.cc
  src/heap.cc
  src/input.cc
  src/mulDisplay.cc
  src/mulDo.cc
  src/mulGlobal.cc
  src/mulLocal.cc
  src/mulMats.cc
  src/mulMulti.cc
  src/mulSetup.cc
  src/mulStruct.cc
  src/patran.cc
  src/psMatDisplay.cc
  src/quickif.cc
  src/savemat_mod.cc
  src/zbuf2fastcap.cc
  src/zbufInOut.cc
  src/zbufProj.cc
  src/zbufSort.cc
)

target_compile_options(corelib PRIVATE ${COMPILE_OPTIONS})

# --------------------------------------------------------------
# cubegen

project(FastCap2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fastcap
  src/fastcap.cc
)

target_link_libraries(fastcap m corelib)
target_compile_options(fastcap PRIVATE ${COMPILE_OPTIONS})

install(TARGETS fastcap
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# --------------------------------------------------------------
# genlib

add_library(genlib STATIC
  src/epsilon.h
  src/disrect.h
  src/distri.h
  src/epsilon.cc
  src/disrect.cc
  src/distri.cc
)

target_compile_options(genlib PRIVATE ${COMPILE_OPTIONS})

# --------------------------------------------------------------
# cubegen

add_executable(cubegen
  src/cubegen.cc
)

target_link_libraries(cubegen m genlib)
target_compile_options(cubegen PRIVATE ${COMPILE_OPTIONS})

install(TARGETS cubegen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# --------------------------------------------------------------
# pipedgen

add_executable(pipedgen
  src/pipedgen.cc
)

target_link_libraries(pipedgen m genlib)
target_compile_options(pipedgen PRIVATE ${COMPILE_OPTIONS})

install(TARGETS pipedgen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# --------------------------------------------------------------
# teragen

add_executable(teragen
  src/teragen.cc
)

target_link_libraries(teragen m genlib)
target_compile_options(teragen PRIVATE -Wall ${COMPILE_OPTIONS})

install(TARGETS teragen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# --------------------------------------------------------------
# busgen

add_executable(busgen
  src/busgen.cc
)

target_link_libraries(busgen m genlib)
target_compile_options(busgen PRIVATE -Wall ${COMPILE_OPTIONS})

install(TARGETS busgen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# --------------------------------------------------------------
# pltcapgen

add_executable(pltcapgen
  src/pltcapgen.cc
)

target_link_libraries(pltcapgen m genlib)
target_compile_options(pltcapgen PRIVATE ${COMPILE_OPTIONS})

install(TARGETS pltcapgen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# --------------------------------------------------------------
# Python module

# This is a stub made for the project.
# Use setup.py to build the real module.

find_package(PythonLibs REQUIRED)

add_library(fastcap2_core SHARED
  fastcap2/src/fastcap2.cc
  fastcap2/src/problem.cc
  # ...
)

target_link_libraries(fastcap2_core m corelib)
target_include_directories (fastcap2_core PRIVATE src ${PYTHON_INCLUDE_DIRS})
