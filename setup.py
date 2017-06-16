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
BUILD_VERSION = "6.0rc1"

# define the list of files to be included as documentation for Windows
dataFiles = None
if sys.platform in ("win32", "cygwin"):
    baseName = "cx_Oracle-doc"
    dataFiles = [ (baseName, [ "LICENSE.txt", "README.txt"]) ]
    for dir in ("samples", "test"):
        files = []
        fullDirName = "%s/%s" % (baseName, dir)
        for name in os.listdir(dir):
            if name.startswith("."):
                continue
            if os.path.isdir(os.path.join(dir, name)):
                continue
            fullName = "%s/%s" % (dir, name)
            files.append(fullName)
        dataFiles.append((fullDirName, files))

# define the list of files to be included as documentation for bdist_rpm
docFiles = "README.txt LICENSE.txt samples test"

# setup extra link and compile args
extraLinkArgs = []
extraCompileArgs = ["-DBUILD_VERSION=%s" % BUILD_VERSION]
if sys.platform == "aix4":
    extraCompileArgs.append("-qcpluscmt")
elif sys.platform == "aix5":
    extraCompileArgs.append("-DAIX5")
elif sys.platform == "cygwin":
    extraLinkArgs.append("-Wl,--enable-runtime-pseudo-reloc")
elif sys.platform == "darwin":
    extraLinkArgs.append("-shared-libgcc")

# setup ODPI-C debugging, if desirable
defines = []
debugLevelName = "DPI_DEBUG_LEVEL"
if debugLevelName in os.environ:
    defines = [(debugLevelName, os.environ[debugLevelName])]

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

# setup the extension
extension = Extension(
        name = "cx_Oracle",
        include_dirs = ["odpi/include", "odpi/src"],
        extra_compile_args = extraCompileArgs,
        extra_link_args = extraLinkArgs,
        define_macros = defines,
        sources = ["src/cx_Oracle.c"],
        depends = ["src/BooleanVar.c", "src/Buffer.c", "src/Connection.c",
                "src/Cursor.c", "src/CursorVar.c", "src/DateTimeVar.c",
                "src/DeqOptions.c", "src/EnqOptions.c", "src/Error.c",
                "src/IntervalVar.c", "src/LOB.c", "src/LobVar.c",
                "src/LongVar.c", "src/MsgProps.c", "src/NumberVar.c",
                "src/Object.c", "src/ObjectType.c", "src/ObjectVar.c",
                "src/SessionPool.c", "src/StringVar.c", "src/Subscription.c",
                "src/Variable.c", "odpi/include/dpi.h", "odpi/src/dpiImpl.h",
                "odpi/src/dpiConn.c", "odpi/src/dpiContext.c",
                "odpi/src/dpiData.c", "odpi/src/dpiDeqOptions.c",
                "odpi/src/dpiEnqOptions.c", "odpi/src/dpiEnv.c",
                "odpi/src/dpiError.c", "odpi/src/dpiGen.c",
                "odpi/src/dpiGlobal.c", "odpi/src/dpiLob.c",
                "odpi/src/dpiMsgProps.c", "odpi/src/dpiObject.c",
                "odpi/src/dpiObjectAttr.c", "odpi/src/dpiObjectType.c",
                "odpi/src/dpiOci.c", "odpi/src/dpiOracleType.c",
                "odpi/src/dpiPool.c", "odpi/src/dpiRowid.c",
                "odpi/src/dpiStmt.c", "odpi/src/dpiSubscr.c",
                "odpi/src/dpiUtils.c", "odpi/src/dpiVar.c"])

# perform the setup
setup(
        name = "cx_Oracle",
        version = BUILD_VERSION,
        description = "Python interface to Oracle",
        data_files = dataFiles,
        cmdclass = dict(test = test),
        options = dict(bdist_rpm = dict(doc_files = docFiles)),
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

