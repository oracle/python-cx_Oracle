#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1900 - Module for testing LOB (CLOB and BLOB) variables
"""

import TestEnv

import cx_Oracle
import sys

class TestCase(TestEnv.BaseTestCase):

    def __GetTempLobs(self, sid):
        cursor = self.connection.cursor()
        cursor.execute("""
                select abstract_lobs
                from v$temporary_lobs
                where sid = :sid""", sid = sid)
        row = cursor.fetchone()
        if row is None:
            return 0
        return int(row[0])

    def __PerformTest(self, lobType, inputType):
        longString = ""
        directType = getattr(cx_Oracle, lobType)
        self.cursor.execute("truncate table Test%ss" % lobType)
        for i in range(0, 11):
            if i > 0:
                char = chr(ord('A') + i - 1)
                longString += char * 25000
            elif inputType != directType:
                continue
            self.cursor.setinputsizes(longString = inputType)
            if lobType == "BLOB" and sys.version_info[0] >= 3:
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
                    )""" % (lobType, lobType),
                    integerValue = i,
                    longString = bindValue)
        self.connection.commit()
        self.cursor.execute("""
                select *
                from Test%ss
                order by IntCol""" % lobType)
        self.__ValidateQuery(self.cursor, lobType)

    def __TestLobOperations(self, lobType):
        self.cursor.execute("truncate table Test%ss" % lobType)
        self.cursor.setinputsizes(longString = getattr(cx_Oracle, lobType))
        longString = "X" * 75000
        writeValue = "TEST"
        if lobType == "BLOB" and sys.version_info[0] >= 3:
            longString = longString.encode("ascii")
            writeValue = writeValue.encode("ascii")
        self.cursor.execute("""
                insert into Test%ss (
                  IntCol,
                  %sCol
                ) values (
                  :integerValue,
                  :longString
                )""" % (lobType, lobType),
                integerValue = 1,
                longString = longString)
        self.cursor.execute("""
                select %sCol
                from Test%ss
                where IntCol = 1""" % (lobType, lobType))
        lob, = self.cursor.fetchone()
        self.assertEqual(lob.isopen(), False)
        lob.open()
        self.assertEqual(lob.isopen(), True)
        lob.close()
        self.assertEqual(lob.isopen(), False)
        self.assertEqual(lob.size(), 75000)
        lob.write(writeValue, 75001)
        self.assertEqual(lob.size(), 75000 + len(writeValue))
        self.assertEqual(lob.read(), longString + writeValue)
        lob.write(writeValue, 1)
        self.assertEqual(lob.read(), writeValue + longString[4:] + writeValue)
        lob.trim(25000)
        self.assertEqual(lob.size(), 25000)
        lob.trim()
        self.assertEqual(lob.size(), 0)

    def __TestTemporaryLOB(self, lobType):
        self.cursor.execute("truncate table Test%ss" % lobType)
        value = "A test string value"
        if lobType == "BLOB" and sys.version_info[0] >= 3:
            value = value.encode("ascii")
        lobTypeObj = getattr(cx_Oracle, lobType)
        lob = self.connection.createlob(lobTypeObj)
        lob.write(value)
        self.cursor.execute("""
                insert into Test%ss (IntCol, %sCol)
                values (:intVal, :lobVal)""" % (lobType, lobType),
                intVal = 1,
                lobVal = lob)
        self.cursor.execute("select %sCol from Test%ss" % (lobType, lobType))
        lob, = self.cursor.fetchone()
        self.assertEqual(lob.read(), value)

    def __ValidateQuery(self, rows, lobType):
        longString = ""
        dbType = getattr(cx_Oracle, "DB_TYPE_" + lobType)
        for row in rows:
            integerValue, lob = row
            self.assertEqual(lob.type, dbType)
            if integerValue == 0:
                self.assertEqual(lob.size(), 0)
                expectedValue = ""
                if lobType == "BLOB" and sys.version_info[0] >= 3:
                    expectedValue = expectedValue.encode("ascii")
                self.assertEqual(lob.read(), expectedValue)
            else:
                char = chr(ord('A') + integerValue - 1)
                prevChar = chr(ord('A') + integerValue - 2)
                longString += char * 25000
                if lobType == "BLOB" and sys.version_info[0] >= 3:
                    actualValue = longString.encode("ascii")
                    char = char.encode("ascii")
                    prevChar = prevChar.encode("ascii")
                else:
                    actualValue = longString
                self.assertEqual(lob.size(), len(actualValue))
                self.assertEqual(lob.read(), actualValue)
                if lobType == "CLOB":
                    self.assertEqual(str(lob), actualValue)
                self.assertEqual(lob.read(len(actualValue)), char)
            if integerValue > 1:
                offset = (integerValue - 1) * 25000 - 4
                string = prevChar * 5 + char * 5
                self.assertEqual(lob.read(offset, 10), string)

    def test_1900_BindLobValue(self):
        "1900 - test binding a LOB value directly"
        self.cursor.execute("truncate table TestCLOBs")
        self.cursor.execute("insert into TestCLOBs values (1, 'Short value')")
        self.cursor.execute("select ClobCol from TestCLOBs")
        lob, = self.cursor.fetchone()
        self.cursor.execute("insert into TestCLOBs values (2, :value)",
                value = lob)

    def test_1901_BLOBCursorDescription(self):
        "1901 - test cursor description is accurate for BLOBs"
        self.cursor.execute("select * from TestBLOBs")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.DB_TYPE_NUMBER, 10, None, 9, 0, 0),
                  ('BLOBCOL', cx_Oracle.DB_TYPE_BLOB, None, None, None, None,
                        0) ])

    def test_1902_BLOBsDirect(self):
        "1902 - test binding and fetching BLOB data (directly)"
        self.__PerformTest("BLOB", cx_Oracle.DB_TYPE_BLOB)

    def test_1903_BLOBsIndirect(self):
        "1903 - test binding and fetching BLOB data (indirectly)"
        self.__PerformTest("BLOB", cx_Oracle.DB_TYPE_LONG_RAW)

    def test_1904_BLOBOperations(self):
        "1904 - test operations on BLOBs"
        self.__TestLobOperations("BLOB")

    def test_1905_CLOBCursorDescription(self):
        "1905 - test cursor description is accurate for CLOBs"
        self.cursor.execute("select * from TestCLOBs")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.DB_TYPE_NUMBER, 10, None, 9, 0, 0),
                  ('CLOBCOL', cx_Oracle.DB_TYPE_CLOB, None, None, None, None,
                        0) ])

    def test_1906_CLOBsDirect(self):
        "1906 - test binding and fetching CLOB data (directly)"
        self.__PerformTest("CLOB", cx_Oracle.DB_TYPE_CLOB)

    def test_1907_CLOBsIndirect(self):
        "1907 - test binding and fetching CLOB data (indirectly)"
        self.__PerformTest("CLOB", cx_Oracle.DB_TYPE_LONG)

    def test_1908_CLOBOperations(self):
        "1908 - test operations on CLOBs"
        self.__TestLobOperations("CLOB")

    def test_1909_CreateBlob(self):
        "1909 - test creating a temporary BLOB"
        self.__TestTemporaryLOB("BLOB")

    def test_1910_CreateClob(self):
        "1910 - test creating a temporary CLOB"
        self.__TestTemporaryLOB("CLOB")

    def test_1911_CreateNclob(self):
        "1911 - test creating a temporary NCLOB"
        self.__TestTemporaryLOB("NCLOB")

    def test_1912_MultipleFetch(self):
        "1912 - test retrieving data from a CLOB after multiple fetches"
        self.cursor.arraysize = 1
        self.cursor.execute("select * from TestCLOBS")
        rows = self.cursor.fetchall()
        self.__ValidateQuery(rows, "CLOB")

    def test_1913_NCLOBCursorDescription(self):
        "1913 - test cursor description is accurate for NCLOBs"
        self.cursor.execute("select * from TestNCLOBs")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.DB_TYPE_NUMBER, 10, None, 9, 0, 0),
                  ('NCLOBCOL', cx_Oracle.DB_TYPE_NCLOB, None, None, None, None,
                      0) ])

    def test_1914_NCLOBsDirect(self):
        "1914 - test binding and fetching NCLOB data (directly)"
        self.__PerformTest("NCLOB", cx_Oracle.DB_TYPE_NCLOB)

    def test_1915_NCLOBDifferentEncodings(self):
        "1915 - test binding and fetching NCLOB data (different encodings)"
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                encoding = "UTF-8", nencoding = "UTF-16")
        value = "\u03b4\u4e2a"
        cursor = connection.cursor()
        cursor.execute("truncate table TestNCLOBs")
        cursor.setinputsizes(val = cx_Oracle.DB_TYPE_NVARCHAR)
        cursor.execute("insert into TestNCLOBs values (1, :val)", val = value)
        cursor.execute("select NCLOBCol from TestNCLOBs")
        nclob, = cursor.fetchone()
        cursor.setinputsizes(val = cx_Oracle.DB_TYPE_NVARCHAR)
        cursor.execute("update TestNCLOBs set NCLOBCol = :val",
                val = nclob.read() + value)
        cursor.execute("select NCLOBCol from TestNCLOBs")
        nclob, = cursor.fetchone()
        self.assertEqual(nclob.read(), value + value)

    def test_1916_NCLOBsIndirect(self):
        "1916 - test binding and fetching NCLOB data (indirectly)"
        self.__PerformTest("NCLOB", cx_Oracle.DB_TYPE_LONG)

    def test_1917_NCLOBOperations(self):
        "1917 - test operations on NCLOBs"
        self.__TestLobOperations("NCLOB")

    def test_1918_TemporaryLobs(self):
        "1918 - test temporary LOBs"
        cursor = self.connection.cursor()
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select sys_context('USERENV', 'SID')
                from dual""")
        sid, = cursor.fetchone()
        tempLobs = self.__GetTempLobs(sid)
        self.assertEqual(tempLobs, 0)
        cursor.execute("""
                select extract(xmlcol, '/').getclobval()
                from TestXML""")
        for lob, in cursor:
            value = lob.read()
            del lob
        cursor.close()
        tempLobs = self.__GetTempLobs(sid)
        self.assertEqual(tempLobs, 0)

    def test_1919_AssignStringBeyondArraySize(self):
        "1919 - test assign string to NCLOB beyond array size"
        nclobVar = self.cursor.var(cx_Oracle.DB_TYPE_NCLOB)
        self.assertRaises(IndexError, nclobVar.setvalue, 1, "test char")

if __name__ == "__main__":
    TestEnv.RunTestCases()
