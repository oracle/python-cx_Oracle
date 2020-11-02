#------------------------------------------------------------------------------
# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
1500 - Module for testing comparisons with database types and API types. This
also contains tests for comparisons with database types and variable types, for
backwards compatibility.
"""

import TestEnv

import cx_Oracle

class TestCase(TestEnv.BaseTestCase):

    def __testCompare(self, dbType, apiType):
        self.assertEqual(dbType, dbType)
        self.assertEqual(dbType, apiType)
        self.assertEqual(apiType, dbType)
        self.assertNotEqual(dbType, 5)
        self.assertNotEqual(dbType, cx_Oracle.DB_TYPE_OBJECT)

    def test_1500_Bfile(self):
        "1500 - test cx_Oracle.DB_TYPE_BFILE comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_BFILE, cx_Oracle.BFILE)

    def test_1501_BinaryDouble(self):
        "1501 - test cx_Oracle.DB_TYPE_BINARY_DOUBLE comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_DOUBLE, cx_Oracle.NUMBER)
        self.assertEqual(cx_Oracle.DB_TYPE_BINARY_DOUBLE,
                cx_Oracle.NATIVE_FLOAT)

    def test_1502_BinaryFloat(self):
        "1502 - test cx_Oracle.DB_TYPE_BINARY_FLOAT comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_FLOAT, cx_Oracle.NUMBER)

    def test_1503_BinaryInteger(self):
        "1503 - test cx_Oracle.DB_TYPE_BINARY_INTEGER comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_INTEGER, cx_Oracle.NUMBER)
        self.assertEqual(cx_Oracle.DB_TYPE_BINARY_INTEGER,
                cx_Oracle.NATIVE_INT)

    def test_1504_Blob(self):
        "1504 - test cx_Oracle.DB_TYPE_BLOB comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_BLOB, cx_Oracle.BLOB)

    def test_1505_Boolean(self):
        "1505 - test cx_Oracle.DB_TYPE_BOOLEAN comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_BOOLEAN, cx_Oracle.BOOLEAN)

    def test_1506_Char(self):
        "1506 - test cx_Oracle.DB_TYPE_CHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_CHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_CHAR, cx_Oracle.FIXED_CHAR)

    def test_1507_Clob(self):
        "1507 - test cx_Oracle.DB_TYPE_CLOB comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_CLOB, cx_Oracle.CLOB)

    def test_1508_Cursor(self):
        "1508 - test cx_Oracle.DB_TYPE_CURSOR comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_CURSOR, cx_Oracle.CURSOR)

    def test_1509_Date(self):
        "1509 - test cx_Oracle.DB_TYPE_DATE comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_DATE, cx_Oracle.DATETIME)

    def test_1510_IntervalDS(self):
        "1510 - test cx_Oracle.DB_TYPE_INTERVAL_DS comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_INTERVAL_DS, cx_Oracle.INTERVAL)

    def test_1511_Long(self):
        "1511 - test cx_Oracle.DB_TYPE_LONG comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_LONG, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_LONG, cx_Oracle.LONG_STRING)

    def test_1512_LongRaw(self):
        "1512 - test cx_Oracle.DB_TYPE_LONG_RAW comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_LONG_RAW, cx_Oracle.BINARY)
        self.assertEqual(cx_Oracle.DB_TYPE_LONG_RAW, cx_Oracle.LONG_BINARY)

    def test_1513_Nchar(self):
        "1513 - test cx_Oracle.DB_TYPE_NCHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_NCHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_NCHAR, cx_Oracle.FIXED_NCHAR)

    def test_1514_Nclob(self):
        "1514 - test cx_Oracle.DB_TYPE_NCLOB comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_NCLOB, cx_Oracle.NCLOB)

    def test_1515_Number(self):
        "1515 - test cx_Oracle.DB_TYPE_NUMBER comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_NUMBER, cx_Oracle.NUMBER)

    def test_1516_Nvarchar(self):
        "1516 - test cx_Oracle.DB_TYPE_NVARCHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_NVARCHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_NVARCHAR, cx_Oracle.NCHAR)

    def test_1517_Object(self):
        "1517 - test cx_Oracle.DB_TYPE_OBJECT comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_OBJECT, cx_Oracle.OBJECT)

    def test_1518_Raw(self):
        "1518 - test cx_Oracle.DB_TYPE_RAW comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_RAW, cx_Oracle.BINARY)

    def test_1519_Rowid(self):
        "1519 - test cx_Oracle.DB_TYPE_ROWID comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_ROWID, cx_Oracle.ROWID)

    def test_1520_Timestamp(self):
        "1520 - test cx_Oracle.DB_TYPE_TIMESTAMP comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP, cx_Oracle.DATETIME)
        self.assertEqual(cx_Oracle.DB_TYPE_TIMESTAMP, cx_Oracle.TIMESTAMP)

    def test_1521_TimestampLTZ(self):
        "1521 - test cx_Oracle.DB_TYPE_TIMESTAMP_LTZ comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP_LTZ, cx_Oracle.DATETIME)

    def test_1522_TimestampTZ(self):
        "1522 - test cx_Oracle.DB_TYPE_TIMESTAMP_TZ comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP_TZ, cx_Oracle.DATETIME)

    def test_1523_Varchar(self):
        "1523 - test cx_Oracle.DB_TYPE_VARCHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_VARCHAR, cx_Oracle.STRING)


if __name__ == "__main__":
    TestEnv.RunTestCases()
