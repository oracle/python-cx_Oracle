"""
Script for creating all of the binaries that are released for the current
platform.
"""

import os
import sys

oracleHomes = os.environ["CX_ORACLE_HOMES"].split(",")
pythonVersions = os.environ["CX_ORACLE_PYTHON_VERSIONS"].split(",")
pythonFormat = os.environ["CX_ORACLE_PYTHON_FORMAT"]

for withUnicode in (False, True):
    if withUnicode:
        os.environ["WITH_UNICODE"] = "1"
    for version in pythonVersions:
        majorVersion, minorVersion = [int(s) for s in version.split(".")]
        if withUnicode and majorVersion >= 3:
            continue
        for oracleHome in oracleHomes:
            messageFragment = "for Python %s.%s in home %s" % \
                    (majorVersion, minorVersion, oracleHome)
            sys.stdout.write("Creating release %s.\n" % messageFragment)
            os.environ["ORACLE_HOME"] = oracleHome
            python = pythonFormat % (majorVersion, minorVersion)
            if sys.platform == "win32":
                if majorVersion == 2 and minorVersion == 4:
                    subCommand = "bdist_wininst"
                else:
                    subCommand = "bdist_msi"
                command = "%s setup.py %s" % (python, subCommand)
            else:
                command = "%s setup.py bdist_rpm --no-autoreq --python %s" % \
                        (python, python)
            sys.stdout.write("Running command %s\n" % command)
            if os.system(command) != 0:
                sys.exit("Stopping. Build %s failed.\n" % messageFragment)

