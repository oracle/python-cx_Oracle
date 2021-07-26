#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
2600 - Module for testing timestamp variables
"""

import time

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def setUp(self):
        super().setUp()
        self.raw_data = []
        self.data_by_key = {}
        for i in range(1, 11):
            time_tuple = (2002, 12, 9, 0, 0, 0, 0, 0, -1)
            time_in_ticks = time.mktime(time_tuple) + i * 86400
            date_value = oracledb.TimestampFromTicks(int(time_in_ticks))
            str_value = str(i * 50)
            fsecond = int(str_value + "0" * (6 - len(str_value)))
            date_col = oracledb.Timestamp(date_value.year, date_value.month,
                                          date_value.day, date_value.hour,
                                          date_value.minute, i * 2, fsecond)
            if i % 2:
                time_in_ticks = time.mktime(time_tuple) + i * 86400 + 86400
                date_value = oracledb.TimestampFromTicks(int(time_in_ticks))
                str_value = str(i * 125)
                fsecond = int(str_value + "0" * (6 - len(str_value)))
                nullable_col = oracledb.Timestamp(date_value.year,
                                                 date_value.month,
                                                 date_value.day,
                                                 date_value.hour,
                                                 date_value.minute, i * 3,
                                                 fsecond)
            else:
                nullable_col = None
            data_tuple = (i, date_col, nullable_col)
            self.raw_data.append(data_tuple)
            self.data_by_key[i] = data_tuple

    def test_2600_bind_timestamp(self):
        "2600 - test binding in a timestamp"
        self.cursor.setinputsizes(value=oracledb.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                select * from TestTimestamps
                where TimestampCol = :value""",
                value=oracledb.Timestamp(2002, 12, 14, 0, 0, 10, 250000))
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[5]])

    def test_2601_bind_null(self):
        "2601 - test binding in a null"
        self.cursor.setinputsizes(value=oracledb.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                select * from TestTimestamps
                where TimestampCol = :value""",
                value=None)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_2602_bind_out_set_input_sizes(self):
        "2602 - test binding out with set input sizes defined"
        bind_vars = self.cursor.setinputsizes(value=oracledb.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                begin
                  :value := to_timestamp('20021209', 'YYYYMMDD');
                end;""")
        self.assertEqual(bind_vars["value"].getvalue(),
                         oracledb.Timestamp(2002, 12, 9))

    def test_2603_bind_in_out_set_input_sizes(self):
        "2603 - test binding in/out with set input sizes defined"
        bind_vars = self.cursor.setinputsizes(value=oracledb.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                begin
                  :value := :value + 5.25;
                end;""",
                value = oracledb.Timestamp(2002, 12, 12, 10, 0, 0))
        self.assertEqual(bind_vars["value"].getvalue(),
                         oracledb.Timestamp(2002, 12, 17, 16, 0, 0))

    def test_2604_bind_out_var(self):
        "2604 - test binding out with cursor.var() method"
        var = self.cursor.var(oracledb.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                begin
                  :value := to_date('20021231 12:31:00',
                      'YYYYMMDD HH24:MI:SS');
                end;""",
                value=var)
        self.assertEqual(var.getvalue(),
                         oracledb.Timestamp(2002, 12, 31, 12, 31, 0))

    def test_2605_bind_in_out_var_direct_set(self):
        "2605 - test binding in/out with cursor.var() method"
        var = self.cursor.var(oracledb.DB_TYPE_TIMESTAMP)
        var.setvalue(0, oracledb.Timestamp(2002, 12, 9, 6, 0, 0))
        self.cursor.execute("""
                begin
                  :value := :value + 5.25;
                end;""",
                value = var)
        self.assertEqual(var.getvalue(),
                         oracledb.Timestamp(2002, 12, 14, 12, 0, 0))

    def test_2606_cursor_description(self):
        "2606 - test cursor description is accurate"
        self.cursor.execute("select * from TestTimestamps")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('TIMESTAMPCOL', oracledb.DB_TYPE_TIMESTAMP, 23, None, 0, 6,
                    False),
            ('NULLABLECOL', oracledb.DB_TYPE_TIMESTAMP, 23, None, 0, 6, True)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_2607_fetchall(self):
        "2607 - test that fetching all of the data returns the correct results"
        self.cursor.execute("select * From TestTimestamps order by IntCol")
        self.assertEqual(self.cursor.fetchall(), self.raw_data)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_2608_fetchmany(self):
        "2608 - test that fetching data in chunks returns the correct results"
        self.cursor.execute("select * From TestTimestamps order by IntCol")
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[0:3])
        self.assertEqual(self.cursor.fetchmany(2), self.raw_data[3:5])
        self.assertEqual(self.cursor.fetchmany(4), self.raw_data[5:9])
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[9:])
        self.assertEqual(self.cursor.fetchmany(3), [])

    def test_2609_fetchone(self):
        "2609 - test that fetching a single row returns the correct results"
        self.cursor.execute("""
                select *
                from TestTimestamps
                where IntCol in (3, 4)
                order by IntCol""")
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[3])
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[4])
        self.assertEqual(self.cursor.fetchone(), None)

if __name__ == "__main__":
    test_env.run_test_cases()
