#------------------------------------------------------------------------------
# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
1500 - Module for testing comparisons with database types and API types,
including the synonyms retained for backwards compatibility. This module also
tests for pickling/unpickling of database types and API types.
"""

import TestEnv

import cx_Oracle
import pickle

class TestCase(TestEnv.BaseTestCase):

    def __testCompare(self, dbType, apiType):
        self.assertEqual(dbType, dbType)
        self.assertEqual(dbType, apiType)
        self.assertEqual(apiType, dbType)
        self.assertNotEqual(dbType, 5)
        self.assertNotEqual(dbType, cx_Oracle.DB_TYPE_OBJECT)

    def __testPickle(self, typ):
        self.assertIs(typ, pickle.loads(pickle.dumps(typ)))

    def test_1500_DB_TYPE_BFILE(self):
        "1500 - test cx_Oracle.DB_TYPE_BFILE comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_BFILE, cx_Oracle.BFILE)
        self.__testPickle(cx_Oracle.DB_TYPE_BFILE)

    def test_1501_DB_TYPE_BINARY_DOUBLE(self):
        "1501 - test cx_Oracle.DB_TYPE_BINARY_DOUBLE comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_DOUBLE, cx_Oracle.NUMBER)
        self.assertEqual(cx_Oracle.DB_TYPE_BINARY_DOUBLE,
                cx_Oracle.NATIVE_FLOAT)
        self.__testPickle(cx_Oracle.DB_TYPE_BINARY_DOUBLE)

    def test_1502_DB_TYPE_BINARY_FLOAT(self):
        "1502 - test cx_Oracle.DB_TYPE_BINARY_FLOAT comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_FLOAT, cx_Oracle.NUMBER)
        self.__testPickle(cx_Oracle.DB_TYPE_BINARY_FLOAT)

    def test_1503_DB_TYPE_BINARY_INTEGER(self):
        "1503 - test cx_Oracle.DB_TYPE_BINARY_INTEGER comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_BINARY_INTEGER, cx_Oracle.NUMBER)
        self.assertEqual(cx_Oracle.DB_TYPE_BINARY_INTEGER,
                cx_Oracle.NATIVE_INT)
        self.__testPickle(cx_Oracle.DB_TYPE_BINARY_INTEGER)

    def test_1504_DB_TYPE_BLOB(self):
        "1504 - test cx_Oracle.DB_TYPE_BLOB comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_BLOB, cx_Oracle.BLOB)
        self.__testPickle(cx_Oracle.DB_TYPE_BLOB)

    def test_1505_DB_TYPE_BOOLEAN(self):
        "1505 - test cx_Oracle.DB_TYPE_BOOLEAN comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_BOOLEAN, cx_Oracle.BOOLEAN)
        self.__testPickle(cx_Oracle.DB_TYPE_BOOLEAN)

    def test_1506_DB_TYPE_CHAR(self):
        "1506 - test cx_Oracle.DB_TYPE_CHAR comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_CHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_CHAR, cx_Oracle.FIXED_CHAR)
        self.__testPickle(cx_Oracle.DB_TYPE_CHAR)

    def test_1507_DB_TYPE_CLOB(self):
        "1507 - test cx_Oracle.DB_TYPE_CLOB comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_CLOB, cx_Oracle.CLOB)
        self.__testPickle(cx_Oracle.DB_TYPE_CLOB)

    def test_1508_DB_TYPE_CURSOR(self):
        "1508 - test cx_Oracle.DB_TYPE_CURSOR comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_CURSOR, cx_Oracle.CURSOR)
        self.__testPickle(cx_Oracle.DB_TYPE_CURSOR)

    def test_1509_DB_TYPE_DATE(self):
        "1509 - test cx_Oracle.DB_TYPE_DATE comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_DATE, cx_Oracle.DATETIME)
        self.__testPickle(cx_Oracle.DB_TYPE_DATE)

    def test_1510_DB_TYPE_INTERVAL_DS(self):
        "1510 - test cx_Oracle.DB_TYPE_INTERVAL_DS comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_INTERVAL_DS, cx_Oracle.INTERVAL)
        self.__testPickle(cx_Oracle.DB_TYPE_INTERVAL_DS)

    def test_1511_DB_TYPE_LONG(self):
        "1511 - test cx_Oracle.DB_TYPE_LONG comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_LONG, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_LONG, cx_Oracle.LONG_STRING)
        self.__testPickle(cx_Oracle.DB_TYPE_LONG)

    def test_1512_DB_TYPE_LONG_RAW(self):
        "1512 - test cx_Oracle.DB_TYPE_LONG_RAW comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_LONG_RAW, cx_Oracle.BINARY)
        self.assertEqual(cx_Oracle.DB_TYPE_LONG_RAW, cx_Oracle.LONG_BINARY)
        self.__testPickle(cx_Oracle.DB_TYPE_LONG_RAW)

    def test_1513_DB_TYPE_NCHAR(self):
        "1513 - test cx_Oracle.DB_TYPE_NCHAR comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_NCHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_NCHAR, cx_Oracle.FIXED_NCHAR)
        self.__testPickle(cx_Oracle.DB_TYPE_NCHAR)

    def test_1514_DB_TYPE_NCLOB(self):
        "1514 - test cx_Oracle.DB_TYPE_NCLOB comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_NCLOB, cx_Oracle.NCLOB)
        self.__testPickle(cx_Oracle.DB_TYPE_NCLOB)

    def test_1515_DB_TYPE_NUMBER(self):
        "1515 - test cx_Oracle.DB_TYPE_NUMBER comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_NUMBER, cx_Oracle.NUMBER)
        self.__testPickle(cx_Oracle.DB_TYPE_NUMBER)

    def test_1516_DB_TYPE_NVARCHAR(self):
        "1516 - test cx_Oracle.DB_TYPE_NVARCHAR comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_NVARCHAR, cx_Oracle.STRING)
        self.assertEqual(cx_Oracle.DB_TYPE_NVARCHAR, cx_Oracle.NCHAR)
        self.__testPickle(cx_Oracle.DB_TYPE_NVARCHAR)

    def test_1517_DB_TYPE_OBJECT(self):
        "1517 - test cx_Oracle.DB_TYPE_OBJECT comparisons and pickling"
        self.assertEqual(cx_Oracle.DB_TYPE_OBJECT, cx_Oracle.OBJECT)
        self.__testPickle(cx_Oracle.DB_TYPE_OBJECT)

    def test_1518_DB_TYPE_RAW(self):
        "1518 - test cx_Oracle.DB_TYPE_RAW comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_RAW, cx_Oracle.BINARY)
        self.__testPickle(cx_Oracle.DB_TYPE_RAW)

    def test_1519_DB_TYPE_ROWID(self):
        "1519 - test cx_Oracle.DB_TYPE_ROWID comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_ROWID, cx_Oracle.ROWID)
        self.__testPickle(cx_Oracle.DB_TYPE_ROWID)

    def test_1520_DB_TYPE_TIMESTAMP(self):
        "1520 - test cx_Oracle.DB_TYPE_TIMESTAMP comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP, cx_Oracle.DATETIME)
        self.assertEqual(cx_Oracle.DB_TYPE_TIMESTAMP, cx_Oracle.TIMESTAMP)
        self.__testPickle(cx_Oracle.DB_TYPE_TIMESTAMP)

    def test_1521_DB_TYPE_TIMESTAMP_LTZ(self):
        "1521 - test cx_Oracle.DB_TYPE_TIMESTAMP_LTZ comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP_LTZ, cx_Oracle.DATETIME)
        self.__testPickle(cx_Oracle.DB_TYPE_TIMESTAMP_LTZ)

    def test_1522_DB_TYPE_TIMESTAMP_TZ(self):
        "1522 - test cx_Oracle.DB_TYPE_TIMESTAMP_TZ comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_TIMESTAMP_TZ, cx_Oracle.DATETIME)
        self.__testPickle(cx_Oracle.DB_TYPE_TIMESTAMP_TZ)

    def test_1523_DB_TYPE_VARCHAR(self):
        "1523 - test cx_Oracle.DB_TYPE_VARCHAR comparisons and pickling"
        self.__testCompare(cx_Oracle.DB_TYPE_VARCHAR, cx_Oracle.STRING)
        self.__testPickle(cx_Oracle.DB_TYPE_VARCHAR)

    def test_1524_NUMBER(self):
        "1524 - test cx_Oracle.NUMBER pickling"
        self.__testPickle(cx_Oracle.NUMBER)

    def test_1525_STRING(self):
        "1525 - test cx_Oracle.STRING pickling"
        self.__testPickle(cx_Oracle.STRING)

    def test_1526_DATETIME(self):
        "1526 - test cx_Oracle.DATETIME pickling"
        self.__testPickle(cx_Oracle.DATETIME)

    def test_1527_BINARY(self):
        "1527 - test cx_Oracle.BINARY pickling"
        self.__testPickle(cx_Oracle.BINARY)

    def test_1528_ROWID(self):
        "1528 - test cx_Oracle.ROWID pickling"
        self.__testPickle(cx_Oracle.ROWID)

if __name__ == "__main__":
    TestEnv.RunTestCases()
