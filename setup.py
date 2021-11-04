"""
Setup script for cx_Oracle.
"""

import os
import pkg_resources
import setuptools
import sys

# check minimum supported Python version
if sys.version_info[:2] < (3, 6):
    raise Exception("Python 3.6 or higher is required. " +
            "For python 2, use 'pip install cx_Oracle==7.3'")

# check minimum supported version of setuptools
pkg_resources.require("setuptools>=40.6.0")

# define build constants
BUILD_VERSION = "8.3.0"

# setup extra link and compile args
extra_link_args = []
extra_compile_args = []
if sys.platform == "aix4":
    extra_compile_args.append("-qcpluscmt")
elif sys.platform == "aix5":
    extra_compile_args.append("-DAIX5")
elif sys.platform == "cygwin":
    extra_link_args.append("-Wl,--enable-runtime-pseudo-reloc")
elif sys.platform == "darwin":
    extra_link_args.append("-shared-libgcc")

# define cx_Oracle sources
source_dir = "src"
sources = [os.path.join(source_dir, n) \
           for n in sorted(os.listdir(source_dir)) if n.endswith(".c")]
depends = ["src/cxoModule.h"]

# define ODPI-C sources, libraries and include directories; if the environment
# variables ODPIC_INC_DIR and ODPIC_LIB_DIR are both set, assume these
# locations contain a compiled installation of ODPI-C; otherwise, use the
# source of ODPI-C found in the odpi subdirectory
dpi_include_dir = os.environ.get("ODPIC_INC_DIR")
dpi_lib_dir = os.environ.get("ODPIC_LIB_DIR")
if dpi_include_dir and dpi_lib_dir:
    dpi_sources = []
    include_dirs = [dpi_include_dir]
    libraries = ["odpic"]
    library_dirs = [dpi_lib_dir]
else:
    include_dirs = ["odpi/include", "odpi/src"]
    dpi_source_dir = os.path.join("odpi", "src")
    dpi_sources = [os.path.join(dpi_source_dir, n) \
            for n in sorted(os.listdir(dpi_source_dir)) if n.endswith(".c")]
    depends.extend(["odpi/include/dpi.h", "odpi/src/dpiImpl.h",
            "odpi/src/dpiErrorMessages.h"])
    libraries = []
    library_dirs = []

# setup the extension
extension = setuptools.Extension(
        name="cx_Oracle",
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        define_macros=[("CXO_BUILD_VERSION", BUILD_VERSION)],
        extra_link_args=extra_link_args,
        sources=sources + dpi_sources,
        depends=depends,
        libraries=libraries,
        library_dirs=library_dirs)

# perform the setup
setuptools.setup(
        version=BUILD_VERSION,
        data_files=[ ("cx_Oracle-doc", ["LICENSE.txt", "README.txt"]) ],
        ext_modules=[extension])
