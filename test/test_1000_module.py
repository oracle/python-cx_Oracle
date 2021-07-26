#------------------------------------------------------------------------------
# Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
1000 - Module for testing top-level module methods
"""

import datetime
import time

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):
    requires_connection = False

    def test_1000_date_from_ticks(self):
        "1000 - test DateFromTicks()"
        today = datetime.datetime.today()
        timestamp = time.mktime(today.timetuple())
        date = oracledb.DateFromTicks(timestamp)
        self.assertEqual(date, today.date())

    def test_1001_future_obj(self):
        "1001 - test management of __future__ object"
        self.assertEqual(oracledb.__future__.dummy, None)
        oracledb.__future__.dummy = "Unimportant"
        self.assertEqual(oracledb.__future__.dummy, None)

    def test_1002_timestamp_from_ticks(self):
        "1002 - test TimestampFromTicks()"
        timestamp = time.mktime(datetime.datetime.today().timetuple())
        today = datetime.datetime.fromtimestamp(timestamp)
        date = oracledb.TimestampFromTicks(timestamp)
        self.assertEqual(date, today)

    def test_1003_unsupported_functions(self):
        "1003 - test unsupported time functions"
        self.assertRaises(oracledb.NotSupportedError, oracledb.Time, 12, 0, 0)
        self.assertRaises(oracledb.NotSupportedError, oracledb.TimeFromTicks,
                          100)

if __name__ == "__main__":
    test_env.run_test_cases()
