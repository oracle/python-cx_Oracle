#------------------------------------------------------------------------------
# Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
1000 - Module for testing top-level module methods
"""

import TestEnv

import cx_Oracle
import datetime
import time

class TestCase(TestEnv.BaseTestCase):

    def test_1000_DateFromTicks(self):
        "1000 - test DateFromTicks()"
        today = datetime.datetime.today()
        timestamp = time.mktime(today.timetuple())
        date = cx_Oracle.DateFromTicks(timestamp)
        self.assertEqual(date, today.date())

    def test_1001_FutureObj(self):
        "1001 - test management of __future__ object"
        self.assertEqual(cx_Oracle.__future__.dummy, None)
        cx_Oracle.__future__.dummy = "Unimportant"
        self.assertEqual(cx_Oracle.__future__.dummy, None)

    def test_1002_TimestampFromTicks(self):
        "1002 - test TimestampFromTicks()"
        timestamp = time.mktime(datetime.datetime.today().timetuple())
        today = datetime.datetime.fromtimestamp(timestamp)
        date = cx_Oracle.TimestampFromTicks(timestamp)
        self.assertEqual(date, today)

    def test_1003_UnsupportedFunctions(self):
        "1003 - test unsupported time functions"
        self.assertRaises(cx_Oracle.NotSupportedError, cx_Oracle.Time,
                12, 0, 0)
        self.assertRaises(cx_Oracle.NotSupportedError, cx_Oracle.TimeFromTicks,
                100)

if __name__ == "__main__":
    TestEnv.RunTestCases()
