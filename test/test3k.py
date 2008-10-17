"""Runs all defined unit tests."""

import cx_Oracle
import imp
import os
import sys
import unittest

print("Running tests for cx_Oracle version", cx_Oracle.version)

import TestEnv

if len(sys.argv) > 1:
    moduleNames = [os.path.splitext(v)[0] for v in sys.argv[1:]]
else:
    moduleNames = [
            "Connection",
            "Cursor",
            "CursorVar",
            "DateTimeVar",
            "LobVar",
            "LongVar",
            "3kNumberVar",
            "ObjectVar",
            "SessionPool",
            "3kStringVar",
            "TimestampVar"
    ]

class BaseTestCase(unittest.TestCase):

    def setUp(self):
        self.connection = cx_Oracle.connect(TestEnv.USERNAME,
                TestEnv.PASSWORD, TestEnv.TNSENTRY)
        self.cursor = self.connection.cursor()
        self.cursor.arraysize = TestEnv.ARRAY_SIZE

    def tearDown(self):
        del self.cursor
        del self.connection


loader = unittest.TestLoader()
runner = unittest.TextTestRunner(verbosity = 2)
failures = []
for name in moduleNames:
    fileName = name + ".py"
    print()
    print("Running tests in", fileName)
    module = imp.new_module(name)
    setattr(module, "USERNAME", TestEnv.USERNAME)
    setattr(module, "PASSWORD", TestEnv.PASSWORD)
    setattr(module, "TNSENTRY", TestEnv.TNSENTRY)
    setattr(module, "ARRAY_SIZE", TestEnv.ARRAY_SIZE)
    setattr(module, "TestCase", unittest.TestCase)
    setattr(module, "BaseTestCase", BaseTestCase)
    setattr(module, "cx_Oracle", cx_Oracle)
    exec(open(fileName).read(), module.__dict__)
    tests = loader.loadTestsFromModule(module)
    result = runner.run(tests)
    if not result.wasSuccessful():
        failures.append(name)
if failures:
    print("***** Some tests in the following modules failed. *****")
    for name in failures:
        print("      %s" % name)

