#------------------------------------------------------------------------------
# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
Module for testing comparisons with database types and API types. This also
contains tests for comparisons with database types and variable types, for
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

    def testBfile(self):
        "test cx_Oracle.DB_TYPE_BFILE comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_BFILE, cx_Oracle.BFILE)

    def testBinaryDouble(self):
        "test cx_Oracle.DB_TYPE_BINARY_DOUBLE comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_DOUBLE, cx_Oracle.NUMBER)
        self.assertEqual(cx_Oracle.DB_TYPE_BINARY_DOUBLE,
                cx_Oracle.NATIVE_FLOAT)

    def testBinaryFloat(self):
        "test cx_Oracle.DB_TYPE_BINARY_FLOAT comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_FLOAT, cx_Oracle.NUMBER)

    def testBinaryInteger(self):
        "test cx_Oracle.DB_TYPE_BINARY_INTEGER comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_INTEGER, cx_Oracle.NUMBER)
        self.assertEqual(cx_Oracle.DB_TYPE_BINARY_INTEGER,
                cx_Oracle.NATIVE_INT)

    def testBlob(self):
        "test cx_Oracle.DB_TYPE_BLOB comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_BLOB, cx_Oracle.BLOB)

    def testBoolean(self):
        "test cx_Oracle.DB_TYPE_BOOLEAN comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_BOOLEAN, cx_Oracle.BOOLEAN)

    def testChar(self):
        "test cx_Oracle.DB_TYPE_CHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_CHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_CHAR, cx_Oracle.FIXED_CHAR)

    def testClob(self):
        "test cx_Oracle.DB_TYPE_CLOB comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_CLOB, cx_Oracle.CLOB)

    def testCursor(self):
        "test cx_Oracle.DB_TYPE_CURSOR comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_CURSOR, cx_Oracle.CURSOR)

    def testDate(self):
        "test cx_Oracle.DB_TYPE_DATE comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_DATE, cx_Oracle.DATETIME)

    def testIntervalDS(self):
        "test cx_Oracle.DB_TYPE_INTERVAL_DS comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_INTERVAL_DS, cx_Oracle.INTERVAL)

    def testLong(self):
        "test cx_Oracle.DB_TYPE_LONG comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_LONG, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_LONG, cx_Oracle.LONG_STRING)

    def testLongRaw(self):
        "test cx_Oracle.DB_TYPE_LONG_RAW comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_LONG_RAW, cx_Oracle.BINARY)
        self.assertEqual(cx_Oracle.DB_TYPE_LONG_RAW, cx_Oracle.LONG_BINARY)

    def testNchar(self):
        "test cx_Oracle.DB_TYPE_NCHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_NCHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_NCHAR, cx_Oracle.FIXED_NCHAR)

    def testNclob(self):
        "test cx_Oracle.DB_TYPE_NCLOB comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_NCLOB, cx_Oracle.NCLOB)

    def testNumber(self):
        "test cx_Oracle.DB_TYPE_NUMBER comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_NUMBER, cx_Oracle.NUMBER)

    def testNvarchar(self):
        "test cx_Oracle.DB_TYPE_NVARCHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_NVARCHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_NVARCHAR, cx_Oracle.NCHAR)

    def testObject(self):
        "test cx_Oracle.DB_TYPE_OBJECT comparisons"
        self.assertEqual(cx_Oracle.DB_TYPE_OBJECT, cx_Oracle.OBJECT)

    def testRaw(self):
        "test cx_Oracle.DB_TYPE_RAW comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_RAW, cx_Oracle.BINARY)

    def testRowid(self):
        "test cx_Oracle.DB_TYPE_ROWID comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_ROWID, cx_Oracle.ROWID)

    def testTimestamp(self):
        "test cx_Oracle.DB_TYPE_TIMESTAMP comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP, cx_Oracle.DATETIME)
        self.assertEqual(cx_Oracle.DB_TYPE_TIMESTAMP, cx_Oracle.TIMESTAMP)

    def testTimestampLTZ(self):
        "test cx_Oracle.DB_TYPE_TIMESTAMP_LTZ comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP_LTZ, cx_Oracle.DATETIME)

    def testTimestampTZ(self):
        "test cx_Oracle.DB_TYPE_TIMESTAMP_TZ comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP_TZ, cx_Oracle.DATETIME)

    def testVarchar(self):
        "test cx_Oracle.DB_TYPE_VARCHAR comparisons"
        self.__testCompare(cx_Oracle.DB_TYPE_VARCHAR, cx_Oracle.STRING)


if __name__ == "__main__":
    TestEnv.RunTestCases()
