"""Setup script for cx_Oracle.

Windows platforms:
    python setup.py build --compiler=mingw32 install

Unix platforms
    python setup.py build install

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
BUILD_VERSION = "8.1.0"

# setup extra link and compile args
extraLinkArgs = []
extraCompileArgs = []
if sys.platform == "aix4":
    extraCompileArgs.append("-qcpluscmt")
elif sys.platform == "aix5":
    extraCompileArgs.append("-DAIX5")
elif sys.platform == "cygwin":
    extraLinkArgs.append("-Wl,--enable-runtime-pseudo-reloc")
elif sys.platform == "darwin":
    extraLinkArgs.append("-shared-libgcc")

# define cx_Oracle sources
sourceDir = "src"
sources = [os.path.join(sourceDir, n) \
        for n in sorted(os.listdir(sourceDir)) if n.endswith(".c")]
depends = ["src/cxoModule.h"]

# define ODPI-C sources, libraries and include directories; if the environment
# variables ODPIC_INC_DIR and ODPIC_LIB_DIR are both set, assume these
# locations contain a compiled installation of ODPI-C; otherwise, use the
# source of ODPI-C found in the odpi subdirectory
dpiIncludeDir = os.environ.get("ODPIC_INC_DIR")
dpiLibDir = os.environ.get("ODPIC_LIB_DIR")
if dpiIncludeDir and dpiLibDir:
    dpiSources = []
    includeDirs = [dpiIncludeDir]
    libraries = ["odpic"]
    libraryDirs = [dpiLibDir]
else:
    includeDirs = ["odpi/include", "odpi/src"]
    dpiSourceDir = os.path.join("odpi", "src")
    dpiSources = [os.path.join(dpiSourceDir, n) \
            for n in sorted(os.listdir(dpiSourceDir)) if n.endswith(".c")]
    depends.extend(["odpi/include/dpi.h", "odpi/src/dpiImpl.h",
            "odpi/src/dpiErrorMessages.h"])
    libraries = []
    libraryDirs = []

# setup the extension
extension = setuptools.Extension(
        name = "cx_Oracle",
        include_dirs = includeDirs,
        extra_compile_args = extraCompileArgs,
        define_macros = [("CXO_BUILD_VERSION", BUILD_VERSION)],
        extra_link_args = extraLinkArgs,
        sources = sources + dpiSources,
        depends = depends,
        libraries = libraries,
        library_dirs = libraryDirs)

# perform the setup
setuptools.setup(
        version = BUILD_VERSION,
        data_files = [ ("cx_Oracle-doc", ["LICENSE.txt", "README.txt"]) ],
        ext_modules = [extension])
