"""
Script for creating all of the binaries that are released for the current
platform.
"""

import os
import sys

pythonVersions = [
        (2, 4),
        (2, 5),
        (2, 6),
        (3, 0)
]

currentVersion = sys.version_info[:2]
pythonVersions.remove(currentVersion)
pythonVersions.append(currentVersion)
oracleHomes = os.environ["ORACLE_HOMES"].split(",")

for majorVersion, minorVersion in pythonVersions:
    for oracleHome in oracleHomes:
        messageFragment = "for Python %s.%s in home %s" % \
                (majorVersion, minorVersion, oracleHome)
        sys.stdout.write("Creating release %s failed.\n" % messageFragment)
        os.environ["ORACLE_HOME"] = oracleHome
        if sys.platform == "win32":
            python = "python%s%s" % (majorVersion, minorVersion)
            if majorVersion == 2 and minorVersion == 4:
                subCommand = "bdist_wininst"
            else:
                subCommand = "bdist_msi"
            command = "%s setup.py %s" % (python, subCommand)
            sys.stdout.write("Running command %s\n" % command)
            if os.system(command) != 0:
                sys.exit("Stopping. Build %s failed.\n" % messageFragment)
        else:
            python = "python%s.%s" % (majorVersion, minorVersion)
            command = "%s setup.py bdist_rpm --no-autoreq --python %s" % \
                    (python, python)
            sys.stdout.write("Running command %s\n" % command)
            if os.system(command) != 0:
                sys.exit("Stopping. Build %s failed.\n" % messageFragment)

