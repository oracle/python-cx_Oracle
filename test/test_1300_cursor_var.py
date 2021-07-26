#------------------------------------------------------------------------------
# Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1300 - Module for testing cursor variables
"""

import sys

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def test_1300_bind_cursor(self):
        "1300 - test binding in a cursor"
        cursor = self.connection.cursor()
        self.assertEqual(cursor.description, None)
        self.cursor.execute("""
                begin
                  open :cursor for select 'X' StringValue from dual;
                end;""",
                cursor=cursor)
        varchar_ratio, nvarchar_ratio = test_env.get_charset_ratios()
        expected_value = [
            ('STRINGVALUE', oracledb.DB_TYPE_CHAR, 1, varchar_ratio, None,
                    None, True)
        ]
        self.assertEqual(cursor.description, expected_value)
        self.assertEqual(cursor.fetchall(), [('X',)])

    def test_1301_bind_cursor_in_package(self):
        "1301 - test binding in a cursor from a package"
        cursor = self.connection.cursor()
        self.assertEqual(cursor.description, None)
        self.cursor.callproc("pkg_TestRefCursors.TestOutCursor", (2, cursor))
        varchar_ratio, nvarchar_ratio = test_env.get_charset_ratios()
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('STRINGCOL', oracledb.DB_TYPE_VARCHAR, 20, 20 * varchar_ratio,
                    None, None, False)
        ]
        self.assertEqual(cursor.description, expected_value)
        self.assertEqual(cursor.fetchall(), [(1, 'String 1'), (2, 'String 2')])

    def test_1302_bind_self(self):
        "1302 - test that binding the cursor itself is not supported"
        cursor = self.connection.cursor()
        sql = """
                begin
                    open :pcursor for
                        select 1 from dual;
                end;"""
        self.assertRaises(oracledb.DatabaseError, cursor.execute, sql,
                          pcursor=cursor)

    def test_1303_execute_after_close(self):
        "1303 - test returning a ref cursor after closing it"
        out_cursor = self.connection.cursor()
        sql = """
                begin
                    open :pcursor for
                    select IntCol
                    from TestNumbers
                    order by IntCol;
                end;"""
        self.cursor.execute(sql, pcursor=out_cursor)
        rows = out_cursor.fetchall()
        out_cursor.close()
        out_cursor = self.connection.cursor()
        self.cursor.execute(sql, pcursor=out_cursor)
        rows2 = out_cursor.fetchall()
        self.assertEqual(rows, rows2)

    def test_1304_fetch_cursor(self):
        "1304 - test fetching a cursor"
        self.cursor.execute("""
                select
                  IntCol,
                  cursor(select IntCol + 1 from dual) CursorValue
                from TestNumbers
                order by IntCol""")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('CURSORVALUE', oracledb.DB_TYPE_CURSOR, None, None, None, None,
                    True)
        ]
        self.assertEqual(self.cursor.description, expected_value)
        for i in range(1, 11):
            number, cursor = self.cursor.fetchone()
            self.assertEqual(number, i)
            self.assertEqual(cursor.fetchall(), [(i + 1,)])

    def test_1305_ref_cursor_binds(self):
        "1305 - test that ref cursor binds cannot use optimised path"
        ref_cursor = self.connection.cursor()
        sql = """
                begin
                    open :rcursor for
                        select IntCol, StringCol
                        from TestStrings where IntCol
                        between :start_value and :end_value;
                end;"""
        self.cursor.execute(sql, rcursor=ref_cursor, start_value=2,
                            end_value=4)
        expected_value = [
            (2, 'String 2'),
            (3, 'String 3'),
            (4, 'String 4')
        ]
        rows = ref_cursor.fetchall()
        ref_cursor.close()
        self.assertEqual(rows, expected_value)
        ref_cursor = self.connection.cursor()
        self.cursor.execute(sql, rcursor=ref_cursor, start_value=5,
                            end_value=6)
        expected_value = [
            (5, 'String 5'),
            (6, 'String 6')
        ]
        rows = ref_cursor.fetchall()
        self.assertEqual(rows, expected_value)

if __name__ == "__main__":
    test_env.run_test_cases()
