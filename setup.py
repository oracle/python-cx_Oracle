"""Distutils script for cx_Oracle.

Windows platforms:
    python setup.py build --compiler=mingw32 install

Unix platforms
    python setup.py build install

"""

import os
import sys

from distutils.core import setup
from distutils.errors import DistutilsSetupError
from distutils.extension import Extension

# define build constants
BUILD_VERSION = "4.4a1"

# define the list of files to be included as documentation for Windows
dataFiles = None
if sys.platform in ("win32", "cygwin"):
    baseName = "cx_Oracle-doc"
    dataFiles = [ (baseName, [ "LICENSE.TXT", "README.TXT", "HISTORY.txt"]) ]
    allFiles = []
    for fileName in open("MANIFEST").readlines():
        allFiles.append(fileName.strip())
    for dir in ("html", "test"):
        files = []
        for name in allFiles:
            if name.startswith(dir):
                files.append(name)
        dataFiles.append( ("%s/%s" % (baseName, dir), files) )

# try to determine the ORACLE_HOME
oracleHome = os.environ.get("ORACLE_HOME")
if oracleHome is None:
    if sys.platform == "win32":
        fileNameToFind = "oci.dll"
    else:
        fileNameToFind = "oracle"
    for path in os.environ["PATH"].split(os.pathsep):
        if os.path.exists(os.path.join(path, fileNameToFind)):
            oracleHome = os.path.dirname(path)
            break
if oracleHome is None:
    raise DistutilsSetupError, "cannot locate an Oracle software installation"

# define some variables
if sys.platform == "win32":
    libDirs = [os.path.join(oracleHome, "bin")]
    includeDirs = [os.path.join(oracleHome, "oci", "include"), \
            os.path.join(oracleHome, "rdbms", "demo")]
    libs = ["oci"]
elif sys.platform == "cygwin":
    includeDirs = ["/usr/include", "rdbms/demo", "rdbms/public", \
            "network/public", "oci/include"]
    libDirs = ["bin", "lib"]
    for i in range(len(includeDirs)):
        includeDirs[i] = os.path.join(oracleHome, includeDirs[i])
    for i in range(len(libDirs)):
        libDirs[i] = os.path.join(oracleHome, libDirs[i])
    libs = ["oci"]
else:
    includeDirs = ["rdbms/demo", "rdbms/public", "network/public",
            "sdk/include"]
    if sys.platform == "darwin":
        includeDirs.append("plsql/public")
    for i in range(len(includeDirs)):
        includeDirs[i] = os.path.join(oracleHome, includeDirs[i])
    libPath = os.path.join(oracleHome, "lib")
    if sys.platform == "sunos5" and sys.maxint > 2147483647:
        libPath = os.path.join(oracleHome, "lib64")
    libDirs = [libPath, oracleHome]
    libs = ["clntsh"]

# NOTE: on HP-UX Itanium with Oracle 10g you need to add the library "ttsh10"
# to the list of libraries along with "clntsh"; since I am unable to test, I'll
# leave this as a comment until someone can verify when this is required
# without making other cases where sys.platform == "hp-ux11" stop working

# setup extra link and compile args
extraCompileArgs = ["-DBUILD_VERSION=%s" % BUILD_VERSION]
extraLinkArgs = []
if sys.platform == "aix4":
    extraCompileArgs.append("-qcpluscmt")
elif sys.platform == "cygwin":
    extraCompileArgs.append("-mno-cygwin")
elif sys.platform == "darwin":
    extraLinkArgs = None

# force the inclusion of an RPATH linker directive if desired; this will
# eliminate the need for setting LD_LIBRARY_PATH but it also means that this
# location will be the only location searched for the Oracle client library
if "FORCE_RPATH" in os.environ:
    extraLinkArgs.append("-Wl,-rpath,%s/lib" % oracleHome)

# setup the extension
extension = Extension(
        name = "cx_Oracle",
        include_dirs = includeDirs,
        libraries = libs,
        library_dirs = libDirs,
        extra_compile_args = extraCompileArgs,
        extra_link_args = extraLinkArgs,
        sources = ["cx_Oracle.c"],
        depends = ["Callback.c", "Connection.c", "Cursor.c", "CursorVar.c",
                "DateTimeVar.c", "Environment.c", "Error.c",
                "ExternalDateTimeVar.c", "ExternalLobVar.c",
                "ExternalObjectVar.c", "LobVar.c", "LongVar.c", "NumberVar.c",
                "ObjectType.c", "ObjectVar.c", "SessionPool.c", "StringVar.c",
                "TimestampVar.c", "Transforms.c", "Variable.c"])

# perform the setup
setup(
        name = "cx_Oracle",
        version = BUILD_VERSION,
        description = "Python interface to Oracle",
        license = "See LICENSE.txt",
        data_files = dataFiles,
        long_description = \
            "Python interface to Oracle conforming to the Python DB API 2.0 "
            "specification.\n"
            "See http://www.python.org/topics/database/DatabaseAPI-2.0.html.",
        author = "Anthony Tuininga",
        author_email = "anthony.tuininga@gmail.com",
        url = "http://cx-oracle.sourceforge.net",
        ext_modules = [extension])

