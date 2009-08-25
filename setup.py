"""Distutils script for cx_Oracle.

Windows platforms:
    python setup.py build --compiler=mingw32 install

Unix platforms
    python setup.py build install

"""

import distutils.command
try:
    import distutils.command.bdist_msi
except ImportError:
    distutils.command.bdist_msi = None
try:
    import distutils.command.bdist_wininst
except ImportError:
    distutils.command.bdist_wininst = None
import distutils.command.bdist_rpm
import distutils.command.build
import distutils.dist
import distutils.util
import os
import struct
import sys

from distutils.errors import DistutilsSetupError

# if setuptools is detected, use it to add support for eggs
try:
    from setuptools import setup, Extension
except:
    from distutils.core import setup
    from distutils.extension import Extension

# define build constants
BUILD_VERSION = "5.0.2"

# define the list of files to be included as documentation for Windows
dataFiles = None
if sys.platform in ("win32", "cygwin"):
    baseName = "cx_Oracle-doc"
    dataFiles = [ (baseName,
            [ "BUILD.txt", "LICENSE.TXT", "README.TXT", "HISTORY.txt"]) ]
    for dir in ("html", "html/_static", "samples", "test"):
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
docFiles = "LICENSE.txt README.txt BUILD.txt HISTORY.txt html samples test"

# method for checking a potential Oracle home
def CheckOracleHome(directoryToCheck):
    global oracleHome, oracleVersion, oracleLibDir, struct
    if sys.platform in ("win32", "cygwin"):
        subDirs = ["bin"]
        filesToCheck = [
                ("11g", "oraocci11.dll"),
                ("10g", "oraocci10.dll"),
                ("9i", "oraclient9.dll")
        ]
    elif sys.platform == "darwin":
        subDirs = ["lib"]
        filesToCheck = [
                ("11g", "libclntsh.dylib.11.1"),
                ("10g", "libclntsh.dylib.10.1"),
                ("9i", "libclntsh.dylib.9.0")
        ]
    else:
        if struct.calcsize("P") == 4:
            subDirs = ["lib", "lib32"]
        else:
            subDirs = ["lib", "lib64"]
        filesToCheck = [
                ("11g", "libclntsh.so.11.1"),
                ("10g", "libclntsh.so.10.1"),
                ("9i", "libclntsh.so.9.0")
        ]
    for version, baseFileName in filesToCheck:
        fileName = os.path.join(directoryToCheck, baseFileName)
        if os.path.exists(fileName):
            if os.path.basename(directoryToCheck).lower() == "bin":
                oracleHome = os.path.dirname(directoryToCheck)
            else:
                oracleHome = directoryToCheck
            oracleLibDir = directoryToCheck
            oracleVersion = version
            return True
        for subDir in subDirs:
            fileName = os.path.join(directoryToCheck, subDir, baseFileName)
            if os.path.exists(fileName):
                oracleHome = directoryToCheck
                oracleLibDir = os.path.join(directoryToCheck, subDir)
                oracleVersion = version
                return True
            dirName = os.path.dirname(directoryToCheck)
            fileName = os.path.join(dirName, subDir, baseFileName)
            if os.path.exists(fileName):
                oracleHome = dirName
                oracleLibDir = os.path.join(dirName, subDir)
                oracleVersion = version
                return True
    oracleHome = oracleVersion = oracleLibDir = None
    return False

# try to determine the Oracle home
userOracleHome = os.environ.get("ORACLE_HOME")
if userOracleHome is not None:
    if not CheckOracleHome(userOracleHome):
        messageFormat = "Oracle home (%s) does not refer to an " \
                "9i, 10g or 11g installation."
        raise DistutilsSetupError(messageFormat % userOracleHome)
else:
    for path in os.environ["PATH"].split(os.pathsep):
        if CheckOracleHome(path):
            break
    if oracleHome is None:
        raise DistutilsSetupError("cannot locate an Oracle software " \
                "installation")

# define some variables
if sys.platform == "win32":
    libDirs = [os.path.join(oracleHome, "bin"), oracleHome,
            os.path.join(oracleHome, "oci", "lib", "msvc"),
            os.path.join(oracleHome, "sdk", "lib", "msvc")]
    possibleIncludeDirs = ["oci/include", "rdbms/demo", "sdk/include"]
    includeDirs = []
    for dir in possibleIncludeDirs:
        path = os.path.normpath(os.path.join(oracleHome, dir))
        if os.path.isdir(path):
            includeDirs.append(path)
    if not includeDirs:
        raise DistutilsSetupError("cannot locate Oracle include files")
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
    libPath = os.path.join(oracleHome, "lib")
    if struct.calcsize("P") == 4:
        alternatePath = os.path.join(oracleHome, "lib32")
    else:
        alternatePath = os.path.join(oracleHome, "lib64")
    if os.path.exists(alternatePath):
        libPath = alternatePath
    libDirs = [libPath, oracleHome]
    libs = ["clntsh"]
    possibleIncludeDirs = ["rdbms/demo", "rdbms/public", "network/public",
            "sdk/include"]
    if sys.platform == "darwin":
        possibleIncludeDirs.append("plsql/public")
    includeDirs = []
    for dir in possibleIncludeDirs:
        path = os.path.join(oracleHome, dir)
        if os.path.isdir(path):
            includeDirs.append(path)
    if not includeDirs:
        path = os.path.join(os.path.dirname(libPath), "include")
        if os.path.isdir(path):
            includeDirs.append(path)
    if not includeDirs:
        raise DistutilsSetupError("cannot locate Oracle include files")

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
    extraLinkArgs.append("-Wl,--enable-runtime-pseudo-reloc")
elif sys.platform == "darwin":
    extraLinkArgs.append("-shared-libgcc")
if "WITH_UNICODE" in os.environ or sys.version_info[0] >= 3:
    extraCompileArgs.append("-DWITH_UNICODE")

# force the inclusion of an RPATH linker directive if desired; this will
# eliminate the need for setting LD_LIBRARY_PATH but it also means that this
# location will be the only location searched for the Oracle client library
if "FORCE_RPATH" in os.environ:
    extraLinkArgs.append("-Wl,-rpath,%s" % oracleLibDir)

# tweak distribution full name to include the Oracle version
class Distribution(distutils.dist.Distribution):

    def get_fullname_with_oracle_version(self):
        name = self.metadata.get_fullname()
        return "%s-%s" % (name, oracleVersion)


# tweak the RPM build command to include the Python and Oracle version
class bdist_rpm(distutils.command.bdist_rpm.bdist_rpm):

    def run(self):
        distutils.command.bdist_rpm.bdist_rpm.run(self)
        specFile = os.path.join(self.rpm_base, "SPECS",
                "%s.spec" % self.distribution.get_name())
        queryFormat = "%{name}-%{version}-%{release}.%{arch}.rpm"
        command = "rpm -q --qf '%s' --specfile %s" % (queryFormat, specFile)
        origFileName = os.popen(command).read()
        parts = origFileName.split("-")
        parts.insert(2, oracleVersion)
        parts.insert(3, "py%s%s" % sys.version_info[:2])
        newFileName = "-".join(parts)
        self.move_file(os.path.join("dist", origFileName),
        os.path.join("dist", newFileName))


# tweak the build directories to include the Oracle version
class build(distutils.command.build.build):

    def finalize_options(self):
        global distutils
        global os
        global sys
        platSpecifier = ".%s-%s-%s" % \
                (distutils.util.get_platform(), sys.version[0:3],
                 oracleVersion)
        if self.build_platlib is None:
            self.build_platlib = os.path.join(self.build_base,
                    "lib%s" % platSpecifier)
        if self.build_temp is None:
            self.build_temp = os.path.join(self.build_base,
                    "temp%s" % platSpecifier)
        distutils.command.build.build.finalize_options(self)

commandClasses = dict(build = build, bdist_rpm = bdist_rpm)

# tweak the Windows installer names to include the Oracle version
if distutils.command.bdist_msi is not None:

    class bdist_msi(distutils.command.bdist_msi.bdist_msi):

        def run(self):
            origMethod = self.distribution.get_fullname
            self.distribution.get_fullname = \
                    self.distribution.get_fullname_with_oracle_version
            distutils.command.bdist_msi.bdist_msi.run(self)
            self.distribution.get_fullname = origMethod

    commandClasses["bdist_msi"] = bdist_msi

if distutils.command.bdist_wininst is not None:

    class bdist_wininst(distutils.command.bdist_wininst.bdist_wininst):

        def run(self):
            origMethod = self.distribution.get_fullname
            self.distribution.get_fullname = \
                    self.distribution.get_fullname_with_oracle_version
            distutils.command.bdist_wininst.bdist_wininst.run(self)
            self.distribution.get_fullname = origMethod

    commandClasses["bdist_wininst"] = bdist_wininst

# define classifiers for the package index
classifiers = [
        "Development Status :: 6 - Mature",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Python Software Foundation License",
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
        include_dirs = includeDirs,
        libraries = libs,
        library_dirs = libDirs,
        extra_compile_args = extraCompileArgs,
        extra_link_args = extraLinkArgs,
        sources = ["cx_Oracle.c"],
        depends = ["Callback.c", "Connection.c", "Cursor.c", "CursorVar.c",
                "DateTimeVar.c", "Environment.c", "Error.c",
                "ExternalLobVar.c", "ExternalObjectVar.c", "IntervalVar.c",
                "LobVar.c", "LongVar.c", "NumberVar.c", "ObjectType.c",
                "ObjectVar.c", "SessionPool.c", "StringUtils.c", "StringVar.c",
                "Subscription.c", "TimestampVar.c", "Transforms.c",
                "Variable.c"])

# perform the setup
setup(
        name = "cx_Oracle",
        version = BUILD_VERSION,
        distclass = Distribution,
        description = "Python interface to Oracle",
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
        ext_modules = [extension],
        keywords = "Oracle",
        license = "Python Software Foundation License",
        classifiers = classifiers)

