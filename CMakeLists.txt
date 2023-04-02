cmake_minimum_required(VERSION 3.5)

project(FastCap2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FastCap2
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
  src/fastcap.cc
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

target_link_libraries(FastCap2 m)
target_compile_options(FastCap2 PRIVATE -Wall -pedantic -Wno-unused-function -Wno-unused-but-set-variable)
# Alternative solver options
# target_compile_definitions(FastCap2 PRIVATE DIRSOL=1 EXPGCR=1)
# With all diagnostic options:
# target_compile_definitions(FastCap2 PRIVATE RAWDAT=1 ITRDAT=1 TIMDAT=1 CFGDAT=1 MULDAT=1 DISSYN=1 DMTCNT=1 NAMDAT=1 DISQ2M=1 DISM2M=1 DISM2P=1 DISL2P=1 DISQ2P=1 DSQ2PD=1 DISQ2L=1 DISM2L=1 DISL2L=1 DALQ2M=1 DALM2P=1 DALL2P=1 DALQ2L=1 DUPVEC=0 DISFAC=1 DPSYSD=1 DILIST=1 DMPELE=1 DMPCHG=1 CKDLST=1 DMPREC=1 CKCLST=1 DUMPPS=1 DPCOMP=1 DPDDIF=1 CHKDUM=1 JACDBG=1)

install(TARGETS FastCap2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
