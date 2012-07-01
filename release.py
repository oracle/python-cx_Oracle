"""
Script for creating all of the binaries that are released for the current
platform.
"""

import os
import sys

testMode = "--test" in sys.argv
oracleHomes = os.environ["CX_ORACLE_HOMES"].split(",")
pythonVersions = os.environ["CX_ORACLE_PYTHON_VERSIONS"].split(",")
pythonFormat = os.environ["CX_ORACLE_PYTHON_FORMAT"]
origPath = os.environ["PATH"]

for version in pythonVersions:
    majorVersion, minorVersion = [int(s) for s in version.split(".")]
    python = pythonFormat % (majorVersion, minorVersion)
    if sys.platform != "win32":
        linkNames = ["python"]
        if majorVersion == 3:
            linkNames.append("python3")
        for linkName in linkNames:
            basePython = os.path.join(os.path.dirname(python), linkName)
            if os.path.exists(basePython):
                os.unlink(basePython)
            os.link(python, basePython)
    for oracleHome in oracleHomes:
        if sys.platform == "win32":
            os.environ["PATH"] = oracleHome + os.pathsep + origPath
        else:
            os.environ["ORACLE_HOME"] = oracleHome
        if testMode:
            subCommand = "test"
            subCommandArgs = ""
        elif sys.platform == "win32":
            subCommandArgs = ""
            if majorVersion == 2 and minorVersion == 4:
                subCommand = "bdist_wininst"
            else:
                subCommand = "bdist_msi"
        else:
            subCommand = "bdist_rpm"
            subCommandArgs = "--no-autoreq"
        command = "%s setup.py %s %s" % \
                (python, subCommand, subCommandArgs)
        messageFragment = "%s for Python %s.%s in home %s" % \
                (subCommand, majorVersion, minorVersion, oracleHome)
        sys.stdout.write("Executing %s.\n" % messageFragment)
        sys.stdout.write("Running command %s\n" % command)
        if os.system(command) != 0:
            msg = "Stopping. execution of %s failed.\n" % messageFragment
            sys.exit(msg)

