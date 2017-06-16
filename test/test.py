#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Runs all defined unit tests."""

from __future__ import print_function

import cx_Oracle
import imp
import os
import sys
import unittest

inSetup = (os.path.basename(sys.argv[0]).lower() == "setup.py")

print("Running tests for cx_Oracle version", cx_Oracle.version,
        cx_Oracle.buildtime)
print("File:", cx_Oracle.__file__)
sys.stdout.flush()

import TestEnv

if len(sys.argv) > 1 and not inSetup:
    moduleNames = [os.path.splitext(v)[0] for v in sys.argv[1:]]
else:
    moduleNames = [
            "Connection",
            "Cursor",
            "CursorVar",
            "DateTimeVar",
            "DMLReturning",
            "Error",
            "IntervalVar",
            "LobVar",
            "LongVar",
            "NCharVar",
            "NumberVar",
            "ObjectVar",
            "SessionPool",
            "StringVar",
            "TimestampVar"
    ]
    if sys.version_info[0] < 3:
        moduleNames.extend([
                "uConnection",
                "uCursor",
                "uCursorVar",
                "uDateTimeVar",
                "uIntervalVar",
                "uLobVar",
                "uLongVar",
                "uNumberVar",
                "uObjectVar",
                "uSessionPool",
                "uStringVar",
                "uTimestampVar"
        ])
    clientVersion = cx_Oracle.clientversion()
    if clientVersion[:2] >= (12, 1):
        moduleNames.append("BooleanVar")
        moduleNames.append("Features12_1")

class BaseTestCase(unittest.TestCase):

    def setUp(self):
        import cx_Oracle
        import TestEnv
        self.connection = cx_Oracle.connect(TestEnv.USERNAME,
                TestEnv.PASSWORD, TestEnv.TNSENTRY,
                encoding = TestEnv.ENCODING, nencoding = TestEnv.NENCODING)
        self.cursor = self.connection.cursor()
        self.cursor.arraysize = TestEnv.ARRAY_SIZE

    def tearDown(self):
        del self.cursor
        del self.connection


# determine character set ratio in use in order to determine the buffer size
# that will be reported in cursor.description; this depends on the database
# character set and the client character set
connection = cx_Oracle.connect(TestEnv.USERNAME, TestEnv.PASSWORD,
        TestEnv.TNSENTRY, encoding = TestEnv.ENCODING,
        nencoding = TestEnv.NENCODING)
cursor = connection.cursor()
cursor.execute("select 'X' from dual")
col, = cursor.description
csratio = col[3]

loader = unittest.TestLoader()
runner = unittest.TextTestRunner(verbosity = 2)
failures = []
for name in moduleNames:
    fileName = name + ".py"
    print()
    print("Running tests in", fileName)
    if inSetup:
        fileName = os.path.join("test", fileName)
    module = imp.new_module(name)
    setattr(module, "USERNAME", TestEnv.USERNAME)
    setattr(module, "PASSWORD", TestEnv.PASSWORD)
    setattr(module, "TNSENTRY", TestEnv.TNSENTRY)
    setattr(module, "ENCODING", TestEnv.ENCODING)
    setattr(module, "NENCODING", TestEnv.NENCODING)
    setattr(module, "ARRAY_SIZE", TestEnv.ARRAY_SIZE)
    setattr(module, "CS_RATIO", csratio)
    setattr(module, "TestCase", unittest.TestCase)
    setattr(module, "BaseTestCase", BaseTestCase)
    setattr(module, "cx_Oracle", cx_Oracle)
    if sys.version_info[0] >= 3:
        exec(open(fileName, encoding = "UTF-8").read(), module.__dict__)
    else:
        execfile(fileName, module.__dict__)
    tests = loader.loadTestsFromModule(module)
    result = runner.run(tests)
    if not result.wasSuccessful():
        failures.append(name)
if failures:
    print("***** Some tests in the following modules failed. *****")
    for name in failures:
        print("      %s" % name)
    sys.exit(1)

