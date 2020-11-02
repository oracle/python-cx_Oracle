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

import TestEnv

import cx_Oracle
import time

class TestCase(TestEnv.BaseTestCase):

    def setUp(self):
        TestEnv.BaseTestCase.setUp(self)
        self.rawData = []
        self.dataByKey = {}
        for i in range(1, 11):
            timeTuple = (2002, 12, 9, 0, 0, 0, 0, 0, -1)
            timeInTicks = time.mktime(timeTuple) + i * 86400
            dateValue = cx_Oracle.TimestampFromTicks(int(timeInTicks))
            strValue = str(i * 50)
            fsecond = int(strValue + "0" * (6 - len(strValue)))
            dateCol = cx_Oracle.Timestamp(dateValue.year, dateValue.month,
                    dateValue.day, dateValue.hour, dateValue.minute,
                    i * 2, fsecond)
            if i % 2:
                timeInTicks = time.mktime(timeTuple) + i * 86400 + 86400
                dateValue = cx_Oracle.TimestampFromTicks(int(timeInTicks))
                strValue = str(i * 125)
                fsecond = int(strValue + "0" * (6 - len(strValue)))
                nullableCol = cx_Oracle.Timestamp(dateValue.year,
                        dateValue.month, dateValue.day, dateValue.hour,
                        dateValue.minute, i * 3, fsecond)
            else:
                nullableCol = None
            tuple = (i, dateCol, nullableCol)
            self.rawData.append(tuple)
            self.dataByKey[i] = tuple

    def test_2600_BindTimestamp(self):
        "2600 - test binding in a timestamp"
        self.cursor.setinputsizes(value = cx_Oracle.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                select * from TestTimestamps
                where TimestampCol = :value""",
                value = cx_Oracle.Timestamp(2002, 12, 14, 0, 0, 10, 250000))
        self.assertEqual(self.cursor.fetchall(), [self.dataByKey[5]])

    def test_2601_BindNull(self):
        "2601 - test binding in a null"
        self.cursor.setinputsizes(value = cx_Oracle.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                select * from TestTimestamps
                where TimestampCol = :value""",
                value = None)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_2602_BindOutSetInputSizes(self):
        "2602 - test binding out with set input sizes defined"
        vars = self.cursor.setinputsizes(value = cx_Oracle.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                begin
                  :value := to_timestamp('20021209', 'YYYYMMDD');
                end;""")
        self.assertEqual(vars["value"].getvalue(),
               cx_Oracle.Timestamp(2002, 12, 9))

    def test_2603_BindInOutSetInputSizes(self):
        "2603 - test binding in/out with set input sizes defined"
        vars = self.cursor.setinputsizes(value = cx_Oracle.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                begin
                  :value := :value + 5.25;
                end;""",
                value = cx_Oracle.Timestamp(2002, 12, 12, 10, 0, 0))
        self.assertEqual(vars["value"].getvalue(),
                cx_Oracle.Timestamp(2002, 12, 17, 16, 0, 0))

    def test_2604_BindOutVar(self):
        "2604 - test binding out with cursor.var() method"
        var = self.cursor.var(cx_Oracle.DB_TYPE_TIMESTAMP)
        self.cursor.execute("""
                begin
                  :value := to_date('20021231 12:31:00',
                      'YYYYMMDD HH24:MI:SS');
                end;""",
                value = var)
        self.assertEqual(var.getvalue(),
               cx_Oracle.Timestamp(2002, 12, 31, 12, 31, 0))

    def test_2605_BindInOutVarDirectSet(self):
        "2605 - test binding in/out with cursor.var() method"
        var = self.cursor.var(cx_Oracle.DB_TYPE_TIMESTAMP)
        var.setvalue(0, cx_Oracle.Timestamp(2002, 12, 9, 6, 0, 0))
        self.cursor.execute("""
                begin
                  :value := :value + 5.25;
                end;""",
                value = var)
        self.assertEqual(var.getvalue(),
                cx_Oracle.Timestamp(2002, 12, 14, 12, 0, 0))

    def test_2606_CursorDescription(self):
        "2606 - test cursor description is accurate"
        self.cursor.execute("select * from TestTimestamps")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.DB_TYPE_NUMBER, 10, None, 9, 0, 0),
                  ('TIMESTAMPCOL', cx_Oracle.DB_TYPE_TIMESTAMP, 23, None, 0, 6,
                      0),
                  ('NULLABLECOL', cx_Oracle.DB_TYPE_TIMESTAMP, 23, None, 0, 6,
                      1) ])

    def test_2607_FetchAll(self):
        "2607 - test that fetching all of the data returns the correct results"
        self.cursor.execute("select * From TestTimestamps order by IntCol")
        self.assertEqual(self.cursor.fetchall(), self.rawData)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_2608_FetchMany(self):
        "2608 - test that fetching data in chunks returns the correct results"
        self.cursor.execute("select * From TestTimestamps order by IntCol")
        self.assertEqual(self.cursor.fetchmany(3), self.rawData[0:3])
        self.assertEqual(self.cursor.fetchmany(2), self.rawData[3:5])
        self.assertEqual(self.cursor.fetchmany(4), self.rawData[5:9])
        self.assertEqual(self.cursor.fetchmany(3), self.rawData[9:])
        self.assertEqual(self.cursor.fetchmany(3), [])

    def test_2609_FetchOne(self):
        "2609 - test that fetching a single row returns the correct results"
        self.cursor.execute("""
                select *
                from TestTimestamps
                where IntCol in (3, 4)
                order by IntCol""")
        self.assertEqual(self.cursor.fetchone(), self.dataByKey[3])
        self.assertEqual(self.cursor.fetchone(), self.dataByKey[4])
        self.assertEqual(self.cursor.fetchone(), None)

if __name__ == "__main__":
    TestEnv.RunTestCases()
