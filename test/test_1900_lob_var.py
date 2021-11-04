#------------------------------------------------------------------------------
# Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1900 - Module for testing LOB (CLOB and BLOB) variables
"""

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def __get_temp_lobs(self, sid):
        cursor = self.connection.cursor()
        cursor.execute("""
                select cache_lobs + nocache_lobs + abstract_lobs
                from v$temporary_lobs
                where sid = :sid""", sid = sid)
        row = cursor.fetchone()
        if row is None:
            return 0
        return int(row[0])

    def __perform_test(self, lob_type, input_type):
        long_string = ""
        db_type = getattr(oracledb, "DB_TYPE_" + lob_type)
        self.cursor.execute("truncate table Test%ss" % lob_type)
        for i in range(0, 11):
            if i > 0:
                char = chr(ord('A') + i - 1)
                long_string += char * 25000
            elif input_type is not db_type:
                continue
            self.cursor.setinputsizes(long_string=input_type)
            if lob_type == "BLOB":
                bind_value = long_string.encode()
            else:
                bind_value = long_string
            self.cursor.execute("""
                    insert into Test%ss (
                      IntCol,
                      %sCol
                    ) values (
                      :integer_value,
                      :long_string
                    )""" % (lob_type, lob_type),
                    integer_value=i,
                    long_string=bind_value)
        self.connection.commit()
        self.cursor.execute("""
                select *
                from Test%ss
                order by IntCol""" % lob_type)
        self.__validate_query(self.cursor, lob_type)

    def __test_lob_operations(self, lob_type):
        self.cursor.execute("truncate table Test%ss" % lob_type)
        self.cursor.setinputsizes(long_string=getattr(oracledb, lob_type))
        long_string = "X" * 75000
        write_value = "TEST"
        if lob_type == "BLOB":
            long_string = long_string.encode("ascii")
            write_value = write_value.encode("ascii")
        self.cursor.execute("""
                insert into Test%ss (
                  IntCol,
                  %sCol
                ) values (
                  :integer_value,
                  :long_string
                )""" % (lob_type, lob_type),
                integer_value=1,
                long_string=long_string)
        self.cursor.execute("""
                select %sCol
                from Test%ss
                where IntCol = 1""" % (lob_type, lob_type))
        lob, = self.cursor.fetchone()
        self.assertEqual(lob.isopen(), False)
        lob.open()
        self.assertEqual(lob.isopen(), True)
        lob.close()
        self.assertEqual(lob.isopen(), False)
        self.assertEqual(lob.size(), 75000)
        lob.write(write_value, 75001)
        self.assertEqual(lob.size(), 75000 + len(write_value))
        self.assertEqual(lob.read(), long_string + write_value)
        lob.write(write_value, 1)
        self.assertEqual(lob.read(),
                         write_value + long_string[4:] + write_value)
        lob.trim(25000)
        self.assertEqual(lob.size(), 25000)
        lob.trim()
        self.assertEqual(lob.size(), 0)

    def __test_temporary_lob(self, lob_type):
        self.cursor.execute("truncate table Test%ss" % lob_type)
        value = "A test string value"
        if lob_type == "BLOB":
            value = value.encode("ascii")
        db_type = getattr(oracledb, "DB_TYPE_" + lob_type)
        lob = self.connection.createlob(db_type)
        lob.write(value)
        self.cursor.execute("""
                insert into Test%ss (IntCol, %sCol)
                values (:int_val, :lob_val)""" % (lob_type, lob_type),
                int_val=1,
                lob_val=lob)
        self.cursor.execute("select %sCol from Test%ss" % (lob_type, lob_type))
        lob, = self.cursor.fetchone()
        self.assertEqual(lob.read(), value)

    def __validate_query(self, rows, lob_type):
        long_string = ""
        db_type = getattr(oracledb, "DB_TYPE_" + lob_type)
        for row in rows:
            integer_value, lob = row
            self.assertEqual(lob.type, db_type)
            if integer_value == 0:
                self.assertEqual(lob.size(), 0)
                expected_value = ""
                if lob_type == "BLOB":
                    expected_value = expected_value.encode()
                self.assertEqual(lob.read(), expected_value)
            else:
                char = chr(ord('A') + integer_value - 1)
                prev_char = chr(ord('A') + integer_value - 2)
                long_string += char * 25000
                if lob_type == "BLOB":
                    expected_value = long_string.encode("ascii")
                    char = char.encode("ascii")
                    prev_char = prev_char.encode("ascii")
                else:
                    expected_value = long_string
                self.assertEqual(lob.size(), len(expected_value))
                self.assertEqual(lob.read(), expected_value)
                if lob_type == "CLOB":
                    self.assertEqual(str(lob), expected_value)
                self.assertEqual(lob.read(len(expected_value)), char)
            if integer_value > 1:
                offset = (integer_value - 1) * 25000 - 4
                string = prev_char * 5 + char * 5
                self.assertEqual(lob.read(offset, 10), string)

    def test_1900_bind_lob_value(self):
        "1900 - test binding a LOB value directly"
        self.cursor.execute("truncate table TestCLOBs")
        self.cursor.execute("insert into TestCLOBs values (1, 'Short value')")
        self.cursor.execute("select ClobCol from TestCLOBs")
        lob, = self.cursor.fetchone()
        self.cursor.execute("insert into TestCLOBs values (2, :value)",
                            value=lob)

    def test_1901_blob_cursor_description(self):
        "1901 - test cursor description is accurate for BLOBs"
        self.cursor.execute("select * from TestBLOBs")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, 0),
            ('BLOBCOL', oracledb.DB_TYPE_BLOB, None, None, None, None, 0)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_1902_blob_direct(self):
        "1902 - test binding and fetching BLOB data (directly)"
        self.__perform_test("BLOB", oracledb.DB_TYPE_BLOB)

    def test_1903_blob_indirect(self):
        "1903 - test binding and fetching BLOB data (indirectly)"
        self.__perform_test("BLOB", oracledb.DB_TYPE_LONG_RAW)

    def test_1904_blob_operations(self):
        "1904 - test operations on BLOBs"
        self.__test_lob_operations("BLOB")

    def test_1905_clob_cursor_description(self):
        "1905 - test cursor description is accurate for CLOBs"
        self.cursor.execute("select * from TestCLOBs")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('CLOBCOL', oracledb.DB_TYPE_CLOB, None, None, None, None, False)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_1906_clob_direct(self):
        "1906 - test binding and fetching CLOB data (directly)"
        self.__perform_test("CLOB", oracledb.DB_TYPE_CLOB)

    def test_1907_clob_indirect(self):
        "1907 - test binding and fetching CLOB data (indirectly)"
        self.__perform_test("CLOB", oracledb.DB_TYPE_LONG)

    def test_1908_clob_operations(self):
        "1908 - test operations on CLOBs"
        self.__test_lob_operations("CLOB")

    def test_1909_create_temp_blob(self):
        "1909 - test creating a temporary BLOB"
        self.__test_temporary_lob("BLOB")

    def test_1910_create_temp_clob(self):
        "1910 - test creating a temporary CLOB"
        self.__test_temporary_lob("CLOB")

    def test_1911_create_temp_nclob(self):
        "1911 - test creating a temporary NCLOB"
        self.__test_temporary_lob("NCLOB")

    def test_1912_multiple_fetch(self):
        "1912 - test retrieving data from a CLOB after multiple fetches"
        self.cursor.arraysize = 1
        self.cursor.execute("select * from TestCLOBS")
        rows = self.cursor.fetchall()
        self.__validate_query(rows, "CLOB")

    def test_1913_nclob_cursor_description(self):
        "1913 - test cursor description is accurate for NCLOBs"
        self.cursor.execute("select * from TestNCLOBs")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, 0),
            ('NCLOBCOL', oracledb.DB_TYPE_NCLOB, None, None, None, None, 0)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_1914_nclob_direct(self):
        "1914 - test binding and fetching NCLOB data (directly)"
        self.__perform_test("NCLOB", oracledb.DB_TYPE_NCLOB)

    def test_1915_nclob_non_ascii_chars(self):
        "1915 - test binding and fetching NCLOB data (with non-ASCII chars)"
        value = "\u03b4\u4e2a"
        self.cursor.execute("truncate table TestNCLOBs")
        self.cursor.setinputsizes(val=oracledb.DB_TYPE_NVARCHAR)
        self.cursor.execute("insert into TestNCLOBs values (1, :val)",
                            val=value)
        self.cursor.execute("select NCLOBCol from TestNCLOBs")
        nclob, = self.cursor.fetchone()
        self.cursor.setinputsizes(val=oracledb.DB_TYPE_NVARCHAR)
        self.cursor.execute("update TestNCLOBs set NCLOBCol = :val",
                            val=nclob.read() + value)
        self.cursor.execute("select NCLOBCol from TestNCLOBs")
        nclob, = self.cursor.fetchone()
        self.assertEqual(nclob.read(), value + value)

    def test_1916_nclob_indirect(self):
        "1916 - test binding and fetching NCLOB data (indirectly)"
        self.__perform_test("NCLOB", oracledb.DB_TYPE_LONG)

    def test_1917_nclob_operations(self):
        "1917 - test operations on NCLOBs"
        self.__test_lob_operations("NCLOB")

    def test_1918_temporary_lobs(self):
        "1918 - test temporary LOBs"
        cursor = self.connection.cursor()
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select sys_context('USERENV', 'SID')
                from dual""")
        sid, = cursor.fetchone()
        temp_lobs = self.__get_temp_lobs(sid)
        self.assertEqual(temp_lobs, 0)
        cursor.execute("""
                select extract(xmlcol, '/').getclobval()
                from TestXML""")
        for lob, in cursor:
            value = lob.read()
            del lob
        cursor.close()
        temp_lobs = self.__get_temp_lobs(sid)
        self.assertEqual(temp_lobs, 0)

    def test_1919_assign_string_beyond_array_size(self):
        "1919 - test assign string to NCLOB beyond array size"
        nclobVar = self.cursor.var(oracledb.DB_TYPE_NCLOB)
        self.assertRaises(IndexError, nclobVar.setvalue, 1, "test char")

if __name__ == "__main__":
    test_env.run_test_cases()
