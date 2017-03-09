#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing long and long raw variables."""

import sys

class TestLongVar(BaseTestCase):

    def __PerformTest(self, a_Type, a_InputType):
        self.cursor.execute("truncate table Test%ss" % a_Type)
        longString = ""
        for i in range(1, 11):
            char = chr(ord('A') + i - 1)
            longString += char * 25000
            self.cursor.setinputsizes(longString = a_InputType)
            if a_Type == "LongRaw" and sys.version_info[0] >= 3:
                bindValue = longString.encode("ascii")
            else:
                bindValue = longString
            self.cursor.execute("""
                    insert into Test%ss (
                      IntCol,
                      %sCol
                    ) values (
                      :integerValue,
                      :longString
                    )""" % (a_Type, a_Type),
                    integerValue = i,
                    longString = bindValue)
        self.connection.commit()
        self.cursor.execute("""
                select *
                from Test%ss
                order by IntCol""" % a_Type)
        longString = ""
        while 1:
            row = self.cursor.fetchone()
            if row is None:
                break
            integerValue, fetchedValue = row
            char = chr(ord('A') + integerValue - 1)
            longString += char * 25000
            if a_Type == "LongRaw" and sys.version_info[0] >= 3:
                actualValue = longString.encode("ascii")
            else:
                actualValue = longString
            self.assertEqual(len(fetchedValue), integerValue * 25000)
            self.assertEqual(fetchedValue, actualValue)

    def testLongs(self):
        "test binding and fetching long data"
        self.__PerformTest("Long", cx_Oracle.LONG_STRING)

    def testLongRaws(self):
        "test binding and fetching long raw data"
        self.__PerformTest("LongRaw", cx_Oracle.LONG_BINARY)

    def testLongCursorDescription(self):
        "test cursor description is accurate for longs"
        self.cursor.execute("select * from TestLongs")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.NUMBER, 10, None, 9, 0, 0),
                  ('LONGCOL', cx_Oracle.LONG_STRING, None, None, None, None,
                        0) ])

    def testLongRawCursorDescription(self):
        "test cursor description is accurate for long raws"
        self.cursor.execute("select * from TestLongRaws")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.NUMBER, 10, None, 9, 0, 0),
                  ('LONGRAWCOL', cx_Oracle.LONG_BINARY, None, None, None, None,
                        0) ])

    def testSetOutputSizesAll(self):
        "test setoutputsizes is valid (all)"
        self.cursor.setoutputsize(25000)
        self.cursor.execute("select * from TestLongRaws")
        longVar = self.cursor.fetchvars[1]
        self.assertEqual(longVar.size, 25000)

    def testSetOutputSizesWrongColumn(self):
        "test setoutputsizes is valid (wrong column)"
        self.cursor.setoutputsize(25000, 1)
        self.cursor.execute("select * from TestLongs")
        longVar = self.cursor.fetchvars[1]
        self.assertEqual(longVar.size, 131072)

    def testSetOutputSizesRightColumn(self):
        "test setoutputsizes is valid (right column)"
        self.cursor.setoutputsize(35000, 2)
        self.cursor.execute("select * from TestLongRaws")
        longVar = self.cursor.fetchvars[1]
        self.assertEqual(longVar.size, 35000)

    def testArraySizeTooLarge(self):
        "test array size too large generates an exception"
        self.cursor.arraysize = 268435456
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.execute,
                "select * from TestLongRaws")

