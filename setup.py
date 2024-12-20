#!/usr/bin/env python

from setuptools import setup, Extension

core_src=[
  "fastcap2/src/fastcap2.cc",
  "fastcap2/src/problem.cc",
  "fastcap2/src/surface.cc",
  "src/blkDirect.cc",
  "src/calcp.cc",
  "src/capsolve.cc",
  "src/counters.cc",
  "src/direct.cc",
  "src/electric.cc",
  "src/fastcap_solve.cc",
  "src/heap.cc",
  "src/input.cc",
  "src/mulDisplay.cc",
  "src/mulDo.cc",
  "src/mulGlobal.cc",
  "src/mulLocal.cc",
  "src/mulMats.cc",
  "src/mulMulti.cc",
  "src/mulSetup.cc",
  "src/mulStruct.cc",
  "src/psMatDisplay.cc",
  "src/quickif.cc",
  "src/patran.cc",
  "src/savemat_mod.cc",
  "src/zbuf2fastcap.cc",
  "src/zbufInOut.cc",
  "src/zbufProj.cc",
  "src/zbufSort.cc"
]

fastcap2_core_module = Extension(
  'fastcap2_core', 
  core_src, 
  include_dirs=['src'],
  define_macros=[('BUILD_FASTCAP2_PYMOD',None)]
)

setup(name='FastCap2',
  version='2.1',
  description='FastCap2 Python port',
  long_description='FastCap2 is a venerable state-of-the art tool to compute the capacitance matrix from an arrangement of (ideal) conductors and dielectric surfaces in the quasi-static case. This module provides a Python API for this tool.\n\nFor more details see https://klayoutmatthias.github.io/FastCap2/introduction.html',
  long_description_content_type='text/x-rst',
  author='Matthias Koefferlein',
  url='https://klayoutmatthias.github.io/FastCap2/',
  packages=['fastcap2'],
  ext_modules=[fastcap2_core_module],
  package_dir={'fastcap2': 'fastcap2/fastcap2'}
)

