"""Distutils script for cx_Oracle.

Windows platforms:
    python setup.py build --compiler=mingw32 install

Unix platforms
    python setup.py build install

"""

import sys

if sys.platform == "win32":
    import distutils.command.bdist_msi
    import distutils.command.bdist_wininst
import distutils.command.build
import distutils.dist
import distutils.util
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

# define the list of files to be included as documentation for bdist_rpm
docFiles = "LICENSE.txt README.txt HISTORY.txt html test"

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
    if sys.maxint == 2 ** 31 - 1:
        alternatePath = os.path.join(oracleHome, "lib32")
    else:
        alternatePath = os.path.join(oracleHome, "lib64")
    if os.path.exists(alternatePath):
        libPath = alternatePath
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

# tweak distribution full name to include the Oracle version
class Distribution(distutils.dist.Distribution):

    def __init__(self, attrs):
        global oracleHome
        distutils.dist.Distribution.__init__(self, attrs)
        if sys.platform == "win32":
            subDir = "bin"
            filesToCheck = [
                    ("11g", "oraclient11.dll"),
                    ("10g", "oraclient10.dll"),
                    ("9i", "oraclient9.dll"),
                    ("8i", "oraclient8.dll")
            ]
        else:
            subDir = "lib"
            filesToCheck = [
                    ("11g", "libclient11.a"),
                    ("10g", "libclient10.a"),
                    ("9i", "libclient9.a"),
                    ("8i", "libclient8.a")
            ]
        self.oracleVersion = None
        for version, baseFileName in filesToCheck:
            fileName = os.path.join(oracleHome, subDir, baseFileName)
            if os.path.exists(fileName):
                self.oracleVersion = version
                break
        if self.oracleVersion is None:
            raise DistutilsSetupError, "Oracle home does not refer to an " \
                    "8i, 9i, 10g or 11g installation"

    def get_fullname_with_oracle_version(self):
        name = self.metadata.get_fullname()
        return "%s-%s" % (name, self.oracleVersion)


# tweak the build directories to include the Oracle version
class build(distutils.command.build.build):

    def finalize_options(self):
        global distutils
        global os
        global sys
        platSpecifier = ".%s-%s-%s" % \
                (distutils.util.get_platform(), sys.version[0:3],
                 self.distribution.oracleVersion)
        if self.build_platlib is None:
            self.build_platlib = os.path.join(self.build_base,
                    "lib%s" % platSpecifier)
        if self.build_temp is None:
            self.build_temp = os.path.join(self.build_base,
                    "temp%s" % platSpecifier)
        distutils.command.build.build.finalize_options(self)

commandClasses = dict(build = build)

# tweak the Windows installer names to include the Oracle version
if sys.platform == "win32":

    class bdist_msi(distutils.command.bdist_msi.bdist_msi):

        def run(self):
            origMethod = self.distribution.get_fullname
            self.distribution.get_fullname = \
                    self.distribution.get_fullname_with_oracle_version
            distutils.command.bdist_msi.bdist_msi.run(self)
            self.distribution.get_fullname = origMethod

    commandClasses["bdist_msi"] = bdist_msi

    class bdist_wininst(distutils.command.bdist_wininst.bdist_wininst):

        def run(self):
            origMethod = self.distribution.get_fullname
            self.distribution.get_fullname = \
                    self.distribution.get_fullname_with_oracle_version
            distutils.command.bdist_wininst.bdist_wininst.run(self)
            self.distribution.get_fullname = origMethod

    commandClasses["bdist_wininst"] = bdist_wininst


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
        distclass = Distribution,
        description = "Python interface to Oracle",
        license = "See LICENSE.txt",
        data_files = dataFiles,
        cmdclass = commandClasses,
        options = dict(bdist_rpm = dict(doc_files = docFiles)),
        long_description = \
            "Python interface to Oracle conforming to the Python DB API 2.0 "
            "specification.\n"
            "See http://www.python.org/topics/database/DatabaseAPI-2.0.html.",
        author = "Anthony Tuininga",
        author_email = "anthony.tuininga@gmail.com",
        url = "http://cx-oracle.sourceforge.net",
        ext_modules = [extension])

