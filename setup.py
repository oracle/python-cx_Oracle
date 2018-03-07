"""Distutils script for cx_Oracle.

Windows platforms:
    python setup.py build --compiler=mingw32 install

Unix platforms
    python setup.py build install

"""

import distutils.core
import os
import sys

# if setuptools is detected, use it to add support for eggs
try:
    from setuptools import setup, Extension
except:
    from distutils.core import setup
    from distutils.extension import Extension

# define build constants
BUILD_VERSION = "6.2.1"

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

class test(distutils.core.Command):
    description = "run the test suite for the extension"
    user_options = []

    def finalize_options(self):
        pass

    def initialize_options(self):
        pass

    def run(self):
        self.run_command("build")
        buildCommand = self.distribution.get_command_obj("build")
        sys.path.insert(0, os.path.abspath("test"))
        sys.path.insert(0, os.path.abspath(buildCommand.build_lib))
        fileName = os.path.join("test", "test.py")
        exec(open(fileName).read())

# define classifiers for the package index
classifiers = [
        "Development Status :: 6 - Mature",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: BSD License",
        "Natural Language :: English",
        "Operating System :: OS Independent",
        "Programming Language :: C",
        "Programming Language :: Python",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "Topic :: Database"
]

# define cx_Oracle sources
sourceDir = "src"
sources = [os.path.join(sourceDir, n) \
        for n in sorted(os.listdir(sourceDir)) if n.endswith(".c")]

# define ODPI-C sources
dpiSourceDir = os.path.join("odpi", "src")
dpiSources = [os.path.join(dpiSourceDir, n) \
        for n in sorted(os.listdir(dpiSourceDir)) if n.endswith(".c")]

# setup the extension
extension = Extension(
        name = "cx_Oracle",
        include_dirs = ["odpi/include", "odpi/src"],
        extra_compile_args = extraCompileArgs,
        define_macros = [("CXO_BUILD_VERSION", BUILD_VERSION)],
        extra_link_args = extraLinkArgs,
        sources = sources + dpiSources,
        depends = ["src/cxoModule.h", "odpi/include/dpi.h",
                "odpi/src/dpiImpl.h", "odpi/src/dpiErrorMessages.h"])

# perform the setup
setup(
        name = "cx_Oracle",
        version = BUILD_VERSION,
        description = "Python interface to Oracle",
        cmdclass = dict(test = test),
        data_files = [ ("cx_Oracle-doc", ["LICENSE.txt", "README.txt"]) ],
        long_description = \
            "Python interface to Oracle Database conforming to the Python DB "
            "API 2.0 specification.\n"
            "See http://www.python.org/topics/database/DatabaseAPI-2.0.html.",
        author = "Anthony Tuininga",
        author_email = "anthony.tuininga@gmail.com",
        url = "https://oracle.github.io/python-cx_Oracle",
        ext_modules = [extension],
        keywords = "Oracle",
        license = "BSD License",
        classifiers = classifiers)

