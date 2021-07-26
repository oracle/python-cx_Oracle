#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1400 - Module for testing date/time variables
"""

import datetime
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
            time_in_ticks = time.mktime(time_tuple) + i * 86400 + i * 8640
            date_col = oracledb.TimestampFromTicks(int(time_in_ticks))
            if i % 2:
                time_in_ticks = time.mktime(time_tuple) + i * 86400 * 2 + \
                        i * 12960
                nullable_col = oracledb.TimestampFromTicks(int(time_in_ticks))
            else:
                nullable_col = None
            tuple = (i, date_col, nullable_col)
            self.raw_data.append(tuple)
            self.data_by_key[i] = tuple

    def test_1400_bind_date(self):
        "1400 - test binding in a date"
        self.cursor.execute("""
                select * from TestDates
                where DateCol = :value""",
                value = oracledb.Timestamp(2002, 12, 13, 9, 36, 0))
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[4]])

    def test_1401_bind_datetime(self):
        "1401 - test binding in a datetime.datetime value"
        self.cursor.execute("""
                select * from TestDates
                where DateCol = :value""",
                value=datetime.datetime(2002, 12, 13, 9, 36, 0))
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[4]])

    def test_1402_bind_date_in_datetime_var(self):
        "1402 - test binding date in a datetime variable"
        var = self.cursor.var(oracledb.DATETIME)
        date_val = datetime.date.today()
        var.setvalue(0, date_val)
        self.cursor.execute("select :1 from dual", [var])
        result, = self.cursor.fetchone()
        self.assertEqual(result.date(), date_val)

    def test_1403_bind_date_after_string(self):
        "1403 - test binding in a date after setting input sizes to a string"
        self.cursor.setinputsizes(value=15)
        self.cursor.execute("""
                select * from TestDates
                where DateCol = :value""",
                value = oracledb.Timestamp(2002, 12, 14, 12, 0, 0))
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[5]])

    def test_1404_bind_null(self):
        "1404 - test binding in a null"
        self.cursor.setinputsizes(value=oracledb.DATETIME)
        self.cursor.execute("""
                select * from TestDates
                where DateCol = :value""",
                value = None)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_1405_bind_date_array_direct(self):
        "1405 - test binding in a date array"
        return_value = self.cursor.var(oracledb.NUMBER)
        array = [r[1] for r in self.raw_data]
        statement = """
                begin
                  :return_value := pkg_TestDateArrays.TestInArrays(
                      :start_value, :base_date, :array);
                end;"""
        self.cursor.execute(statement, return_value=return_value,
                            start_value=5,
                            base_date=oracledb.Date(2002, 12, 12), array=array)
        self.assertEqual(return_value.getvalue(), 35.5)
        array = array + array[:5]
        self.cursor.execute(statement, start_value=7,
                            base_date=oracledb.Date(2002, 12, 13), array=array)
        self.assertEqual(return_value.getvalue(), 24.0)

    def test_1406_bind_date_array_by_sizes(self):
        "1406 - test binding in a date array (with setinputsizes)"
        return_value = self.cursor.var(oracledb.NUMBER)
        self.cursor.setinputsizes(array=[oracledb.DATETIME, 10])
        array = [r[1] for r in self.raw_data]
        self.cursor.execute("""
                begin
                  :return_value := pkg_TestDateArrays.TestInArrays(
                      :start_value, :base_date, :array);
                end;""",
                return_value=return_value,
                start_value=6,
                base_date=oracledb.Date(2002, 12, 13),
                array=array)
        self.assertEqual(return_value.getvalue(), 26.5)

    def test_1407_bind_date_array_by_var(self):
        "1407 - test binding in a date array (with arrayvar)"
        return_value = self.cursor.var(oracledb.NUMBER)
        array = self.cursor.arrayvar(oracledb.DATETIME, 10, 20)
        array.setvalue(0, [r[1] for r in self.raw_data])
        self.cursor.execute("""
                begin
                  :return_value := pkg_TestDateArrays.TestInArrays(
                      :start_value, :base_date, :array);
                end;""",
                return_value=return_value,
                start_value=7,
                base_date=oracledb.Date(2002, 12, 14),
                array=array)
        self.assertEqual(return_value.getvalue(), 17.5)

    def test_1408_bind_in_out_date_array_by_var(self):
        "1408 - test binding in/out a date array (with arrayvar)"
        array = self.cursor.arrayvar(oracledb.DATETIME, 10, 100)
        original_data = [r[1] for r in self.raw_data]
        array.setvalue(0, original_data)
        self.cursor.execute("""
                begin
                  pkg_TestDateArrays.TestInOutArrays(:num_elems, :array);
                end;""",
                num_elems=5,
                array=array)
        self.assertEqual(array.getvalue(),
                [ oracledb.Timestamp(2002, 12, 17, 2, 24, 0),
                  oracledb.Timestamp(2002, 12, 18, 4, 48, 0),
                  oracledb.Timestamp(2002, 12, 19, 7, 12, 0),
                  oracledb.Timestamp(2002, 12, 20, 9, 36, 0),
                  oracledb.Timestamp(2002, 12, 21, 12, 0, 0) ] + \
                original_data[5:])

    def test_1409_bind_out_date_array_by_var(self):
        "1409 - test binding out a date array (with arrayvar)"
        array = self.cursor.arrayvar(oracledb.DATETIME, 6, 100)
        self.cursor.execute("""
                begin
                  pkg_TestDateArrays.TestOutArrays(:num_elems, :array);
                end;""",
                num_elems=6,
                array=array)
        self.assertEqual(array.getvalue(),
                [ oracledb.Timestamp(2002, 12, 13, 4, 48, 0),
                  oracledb.Timestamp(2002, 12, 14, 9, 36, 0),
                  oracledb.Timestamp(2002, 12, 15, 14, 24, 0),
                  oracledb.Timestamp(2002, 12, 16, 19, 12, 0),
                  oracledb.Timestamp(2002, 12, 18, 0, 0, 0),
                  oracledb.Timestamp(2002, 12, 19, 4, 48, 0) ])

    def test_1410_bind_out_set_input_sizes(self):
        "1410 - test binding out with set input sizes defined"
        bind_vars = self.cursor.setinputsizes(value=oracledb.DATETIME)
        self.cursor.execute("""
                begin
                  :value := to_date(20021209, 'YYYYMMDD');
                end;""")
        self.assertEqual(bind_vars["value"].getvalue(),
               oracledb.Timestamp(2002, 12, 9))

    def test_1411_bind_in_out_set_input_sizes(self):
        "1411 - test binding in/out with set input sizes defined"
        bind_vars = self.cursor.setinputsizes(value=oracledb.DATETIME)
        self.cursor.execute("""
                begin
                  :value := :value + 5.25;
                end;""",
                value=oracledb.Timestamp(2002, 12, 12, 10, 0, 0))
        self.assertEqual(bind_vars["value"].getvalue(),
                oracledb.Timestamp(2002, 12, 17, 16, 0, 0))

    def test_1412_bind_out_var(self):
        "1412 - test binding out with cursor.var() method"
        var = self.cursor.var(oracledb.DATETIME)
        self.cursor.execute("""
                begin
                  :value := to_date('20021231 12:31:00',
                      'YYYYMMDD HH24:MI:SS');
                end;""",
                value=var)
        self.assertEqual(var.getvalue(),
               oracledb.Timestamp(2002, 12, 31, 12, 31, 0))

    def test_1413_bind_in_out_var_direct_set(self):
        "1413 - test binding in/out with cursor.var() method"
        var = self.cursor.var(oracledb.DATETIME)
        var.setvalue(0, oracledb.Timestamp(2002, 12, 9, 6, 0, 0))
        self.cursor.execute("""
                begin
                  :value := :value + 5.25;
                end;""",
                value=var)
        self.assertEqual(var.getvalue(),
                         oracledb.Timestamp(2002, 12, 14, 12, 0, 0))

    def test_1414_cursor_description(self):
        "1414 - test cursor description is accurate"
        self.cursor.execute("select * from TestDates")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('DATECOL', oracledb.DB_TYPE_DATE, 23, None, None, None, False),
            ('NULLABLECOL', oracledb.DB_TYPE_DATE, 23, None, None, None, True)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_1415_fetchall(self):
        "1415 - test that fetching all of the data returns the correct results"
        self.cursor.execute("select * From TestDates order by IntCol")
        self.assertEqual(self.cursor.fetchall(), self.raw_data)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_1416_fetchmany(self):
        "1416 - test that fetching data in chunks returns the correct results"
        self.cursor.execute("select * From TestDates order by IntCol")
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[0:3])
        self.assertEqual(self.cursor.fetchmany(2), self.raw_data[3:5])
        self.assertEqual(self.cursor.fetchmany(4), self.raw_data[5:9])
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[9:])
        self.assertEqual(self.cursor.fetchmany(3), [])

    def test_1417_fetchone(self):
        "1417 - test that fetching a single row returns the correct results"
        self.cursor.execute("""
                select *
                from TestDates
                where IntCol in (3, 4)
                order by IntCol""")
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[3])
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[4])
        self.assertEqual(self.cursor.fetchone(), None)

if __name__ == "__main__":
    test_env.run_test_cases()
