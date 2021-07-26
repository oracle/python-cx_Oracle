#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1800 - Module for testing interval variables
"""

import datetime

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def setUp(self):
        super().setUp()
        self.raw_data = []
        self.data_by_key = {}
        for i in range(1, 11):
            delta = datetime.timedelta(days=i, hours=i, minutes=i * 2,
                                       seconds=i * 3)
            if i % 2 == 0:
                nullable_delta = None
            else:
                nullable_delta = datetime.timedelta(days=i + 5, hours=i + 2,
                                                    minutes=i * 2 + 5,
                                                    seconds=i * 3 + 5)
            data_tuple = (i, delta, nullable_delta)
            self.raw_data.append(data_tuple)
            self.data_by_key[i] = data_tuple

    def test_1800_bind_interval(self):
        "1800 - test binding in an interval"
        self.cursor.setinputsizes(value=oracledb.DB_TYPE_INTERVAL_DS)
        value = datetime.timedelta(days=5, hours=5, minutes=10, seconds=15)
        self.cursor.execute("""
                select * from TestIntervals
                where IntervalCol = :value""",
                value=value)
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[5]])

    def test_1801_bind_null(self):
        "1801 - test binding in a null"
        self.cursor.setinputsizes(value=oracledb.DB_TYPE_INTERVAL_DS)
        self.cursor.execute("""
                select * from TestIntervals
                where IntervalCol = :value""",
                value=None)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_1802_bind_out_set_input_sizes(self):
        "1802 - test binding out with set input sizes defined"
        bind_vars = \
                self.cursor.setinputsizes(value=oracledb.DB_TYPE_INTERVAL_DS)
        self.cursor.execute("""
                begin
                  :value := to_dsinterval('8 09:24:18.123789');
                end;""")
        expected_value = datetime.timedelta(days=8, hours=9, minutes=24,
                                            seconds=18, microseconds=123789)
        self.assertEqual(bind_vars["value"].getvalue(), expected_value)

    def test_1803_bind_in_out_set_input_sizes(self):
        "1803 - test binding in/out with set input sizes defined"
        bind_vars = \
                self.cursor.setinputsizes(value=oracledb.DB_TYPE_INTERVAL_DS)
        self.cursor.execute("""
                begin
                  :value := :value + to_dsinterval('5 08:30:00');
                end;""",
                value=datetime.timedelta(days=5, hours=2, minutes=15))
        expected_value = datetime.timedelta(days=10, hours=10, minutes=45)
        self.assertEqual(bind_vars["value"].getvalue(), expected_value)

    def test_1804_bind_in_out_fractional_second(self):
        "1804 - test binding in/out with set input sizes defined"
        bind_vars = \
                self.cursor.setinputsizes(value=oracledb.DB_TYPE_INTERVAL_DS)
        self.cursor.execute("""
                begin
                  :value := :value + to_dsinterval('5 08:30:00');
                end;""",
                value=datetime.timedelta(days=5, seconds=12.123789))
        expected_value = datetime.timedelta(days=10, hours=8, minutes=30,
                                            seconds=12, microseconds=123789)
        self.assertEqual(bind_vars["value"].getvalue(), expected_value)

    def test_1805_bind_out_var(self):
        "1805 - test binding out with cursor.var() method"
        var = self.cursor.var(oracledb.DB_TYPE_INTERVAL_DS)
        self.cursor.execute("""
                begin
                  :value := to_dsinterval('15 18:35:45.586');
                end;""",
                value=var)
        expected_value = datetime.timedelta(days=15, hours=18, minutes=35,
                                            seconds=45, milliseconds=586)
        self.assertEqual(var.getvalue(), expected_value)

    def test_1806_bind_in_out_var_direct_set(self):
        "1806 - test binding in/out with cursor.var() method"
        var = self.cursor.var(oracledb.DB_TYPE_INTERVAL_DS)
        var.setvalue(0, datetime.timedelta(days=1, minutes=50))
        self.cursor.execute("""
                begin
                  :value := :value + to_dsinterval('8 05:15:00');
                end;""",
                value=var)
        expected_value = datetime.timedelta(days=9, hours=6, minutes=5)
        self.assertEqual(var.getvalue(), expected_value)

    def test_1807_cursor_description(self):
        "1807 - test cursor description is accurate"
        self.cursor.execute("select * from TestIntervals")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('INTERVALCOL', oracledb.DB_TYPE_INTERVAL_DS, None, None, 2, 6,
                    False),
            ('NULLABLECOL', oracledb.DB_TYPE_INTERVAL_DS, None, None, 2, 6,
                    True)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_1808_fetchall(self):
        "1808 - test that fetching all of the data returns the correct results"
        self.cursor.execute("select * From TestIntervals order by IntCol")
        self.assertEqual(self.cursor.fetchall(), self.raw_data)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_1809_fetchmany(self):
        "1809 - test that fetching data in chunks returns the correct results"
        self.cursor.execute("select * From TestIntervals order by IntCol")
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[0:3])
        self.assertEqual(self.cursor.fetchmany(2), self.raw_data[3:5])
        self.assertEqual(self.cursor.fetchmany(4), self.raw_data[5:9])
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[9:])
        self.assertEqual(self.cursor.fetchmany(3), [])

    def test_1810_fetchone(self):
        "1810 - test that fetching a single row returns the correct results"
        self.cursor.execute("""
                select *
                from TestIntervals
                where IntCol in (3, 4)
                order by IntCol""")
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[3])
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[4])
        self.assertEqual(self.cursor.fetchone(), None)

    def test_1811_bind_and_fetch_negative_interval(self):
        "1811 - test binding and fetching a negative interval"
        value = datetime.timedelta(days=-1, seconds=86314, microseconds=431152)
        self.cursor.execute("select :1 from dual", [value])
        result, = self.cursor.fetchone()
        self.assertEqual(result, value)

if __name__ == "__main__":
    test_env.run_test_cases()
