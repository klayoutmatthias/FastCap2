cmake_minimum_required(VERSION 3.5)

project(FastCap2 LANGUAGES C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FastCap2
  src/mulGlobal.h src/input.h src/zbuf2fastcap.h src/mulMulti.h src/mulMats.h
  src/mulSetup.h src/calcp.h src/capsolve.h src/mulDisplay.h src/direct.h
  src/mulDo.h src/zbufInOut.h src/zbufProj.h src/zbufSort.h
  src/fastcap.c
  src/mulGlobal.c
  src/mulSetup.c src/mulMats.c src/mulDo.c
  src/mulDisplay.c src/mulMulti.c
  src/mulLocal.c src/direct.c src/calcp.c src/fastcap.c
  src/blkDirect.c src/uglyalloc.c
  src/patran.c src/quickif.c src/electric.c
  src/input.c src/psMatDisplay.c src/capsolve.c src/savemat_mod.c
  src/zbufInOut.c src/zbuf2fastcap.c src/zbufProj.c src/zbufSort.c
)

target_link_libraries(FastCap2 m)

install(TARGETS FastCap2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
