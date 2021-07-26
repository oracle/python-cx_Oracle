#------------------------------------------------------------------------------
# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
1600 - Module for testing DML returning clauses
"""

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def test_1600_insert(self):
        "1600 - test insert (single row) with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        int_val = 5
        str_val = "A test string"
        int_var = self.cursor.var(oracledb.NUMBER)
        str_var = self.cursor.var(str)
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:int_val, :str_val)
                returning IntCol, StringCol into :int_var, :str_var""",
                int_val=int_val,
                str_val=str_val,
                int_var=int_var,
                str_var=str_var)
        self.assertEqual(int_var.values, [[int_val]])
        self.assertEqual(str_var.values, [[str_val]])

    def test_1601_insert_many(self):
        "1601 - test insert (multiple rows) with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        int_values = [5, 8, 17, 24, 6]
        str_values = ["Test 5", "Test 8", "Test 17", "Test 24", "Test 6"]
        int_var = self.cursor.var(oracledb.NUMBER, arraysize=len(int_values))
        str_var = self.cursor.var(str, arraysize=len(int_values))
        self.cursor.setinputsizes(None, None, int_var, str_var)
        data = list(zip(int_values, str_values))
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:int_val, :str_val)
                returning IntCol, StringCol into :int_var, :str_var""", data)
        self.assertEqual(int_var.values, [[v] for v in int_values])
        self.assertEqual(str_var.values, [[v] for v in str_values])

    def test_1602_insert_with_small_size(self):
        "1602 - test insert with DML returning into too small a variable"
        self.cursor.execute("truncate table TestTempTable")
        int_val = 6
        str_val = "A different test string"
        int_var = self.cursor.var(oracledb.NUMBER)
        str_var = self.cursor.var(str, 2)
        parameters = dict(int_val=int_val, str_val=str_val, int_var=int_var,
                          str_var=str_var)
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute, """
                insert into TestTempTable (IntCol, StringCol)
                values (:int_val, :str_val)
                returning IntCol, StringCol into :int_var, :str_var""",
                parameters)

    def test_1603_update_single_row(self):
        "1603 - test update single row with DML returning"
        int_val = 7
        str_val = "The updated value of the string"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""",
                (int_val, "The initial value of the string"))
        int_var = self.cursor.var(oracledb.NUMBER)
        str_var = self.cursor.var(str)
        self.cursor.execute("""
                update TestTempTable set
                    StringCol = :str_val
                where IntCol = :int_val
                returning IntCol, StringCol into :int_var, :str_var""",
                int_val=int_val,
                str_val=str_val,
                int_var=int_var,
                str_var=str_var)
        self.assertEqual(int_var.values, [[int_val]])
        self.assertEqual(str_var.values, [[str_val]])

    def test_1604_update_no_rows(self):
        "1604 - test update no rows with DML returning"
        int_val = 8
        str_val = "The updated value of the string"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""",
                (int_val, "The initial value of the string"))
        int_var = self.cursor.var(oracledb.NUMBER)
        str_var = self.cursor.var(str)
        self.cursor.execute("""
                update TestTempTable set
                    StringCol = :str_val
                where IntCol = :int_val
                returning IntCol, StringCol into :int_var, :str_var""",
                int_val=int_val + 1,
                str_val=str_val,
                int_var=int_var,
                str_var=str_var)
        self.assertEqual(int_var.values, [[]])
        self.assertEqual(str_var.values, [[]])
        self.assertEqual(int_var.getvalue(), [])
        self.assertEqual(str_var.getvalue(), [])

    def test_1605_update_multiple_rows(self):
        "1605 - test update multiple rows with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        for i in (8, 9, 10):
            self.cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (:1, :2)""",
                    (i, "The initial value of string %d" % i))
        int_var = self.cursor.var(oracledb.NUMBER)
        str_var = self.cursor.var(str)
        self.cursor.execute("""
                update TestTempTable set
                    IntCol = IntCol + 15,
                    StringCol = 'The final value of string ' || to_char(IntCol)
                returning IntCol, StringCol into :int_var, :str_var""",
                int_var=int_var,
                str_var=str_var)
        self.assertEqual(self.cursor.rowcount, 3)
        self.assertEqual(int_var.values, [[23, 24, 25]])
        expected_values = [[
            "The final value of string 8",
            "The final value of string 9",
            "The final value of string 10"
        ]]
        self.assertEqual(str_var.values, expected_values)

    def test_1606_update_multiple_rows_executemany(self):
        "1606 - test update multiple rows with DML returning (executeMany)"
        data = [(i, "The initial value of string %d" % i) \
                for i in range(1, 11)]
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""", data)
        int_var = self.cursor.var(oracledb.NUMBER, arraysize=3)
        str_var = self.cursor.var(str, arraysize=3)
        self.cursor.setinputsizes(None, int_var, str_var)
        self.cursor.executemany("""
                update TestTempTable set
                    IntCol = IntCol + 25,
                    StringCol = 'Updated value of string ' || to_char(IntCol)
                where IntCol < :inVal
                returning IntCol, StringCol into :int_var, :str_var""",
                [[3], [8], [11]])
        expected_values = [
                [26, 27],
                [28, 29, 30, 31, 32],
                [33, 34, 35]
        ]
        self.assertEqual(int_var.values, expected_values)
        expected_values = [
            [
                "Updated value of string 1",
                "Updated value of string 2"
            ],
            [
                "Updated value of string 3",
                "Updated value of string 4",
                "Updated value of string 5",
                "Updated value of string 6",
                "Updated value of string 7"
            ],
            [
                "Updated value of string 8",
                "Updated value of string 9",
                "Updated value of string 10"
            ]
        ]
        self.assertEqual(str_var.values, expected_values)

    def test_1607_insert_and_return_object(self):
        "1607 - test inserting an object with DML returning"
        type_obj = self.connection.gettype("UDT_OBJECT")
        string_value = "The string that will be verified"
        obj = type_obj.newobject()
        obj.STRINGVALUE = string_value
        out_var = self.cursor.var(oracledb.DB_TYPE_OBJECT,
                                  typename="UDT_OBJECT")
        self.cursor.execute("""
                insert into TestObjects (IntCol, ObjectCol)
                values (4, :obj)
                returning ObjectCol into :outObj""",
                obj=obj, outObj=out_var)
        result, = out_var.getvalue()
        self.assertEqual(result.STRINGVALUE, string_value)
        self.connection.rollback()

    def test_1608_insert_and_return_rowid(self):
        "1608 - test inserting a row and returning a rowid"
        self.cursor.execute("truncate table TestTempTable")
        var = self.cursor.var(oracledb.ROWID)
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (278, 'String 278')
                returning rowid into :1""", (var,))
        rowid, = var.getvalue()
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                where rowid = :1""",
                (rowid,))
        self.assertEqual(self.cursor.fetchall(), [(278, 'String 278')])

    def test_1609_insert_with_ref_cursor(self):
        "1609 - test inserting with a REF cursor and returning a rowid"
        self.cursor.execute("truncate table TestTempTable")
        var = self.cursor.var(oracledb.ROWID)
        in_cursor = self.connection.cursor()
        in_cursor.execute("""
                select StringCol
                from TestStrings
                where IntCol >= 5
                order by IntCol""")
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (187, pkg_TestRefCursors.TestInCursor(:1))
                returning rowid into :2""", (in_cursor, var))
        rowid, = var.getvalue()
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                where rowid = :1""",
                (rowid,))
        self.assertEqual(self.cursor.fetchall(),
                [(187, 'String 7 (Modified)')])

    def test_1610_delete_returning_decreasing_rows_returned(self):
        "1610 - test delete returning decreasing number of rows"
        data = [(i, "Test String %d" % i) for i in range(1, 11)]
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""", data)
        results = []
        int_var = self.cursor.var(int)
        self.cursor.setinputsizes(None, int_var)
        for int_val in (5, 8, 10):
            self.cursor.execute("""
                    delete from TestTempTable
                    where IntCol < :1
                    returning IntCol into :2""", [int_val])
            results.append(int_var.getvalue())
        self.assertEqual(results, [[1, 2, 3, 4], [5, 6, 7], [8, 9]])

    def test_1611_delete_returning_no_rows_after_many_rows(self):
        "1611 - test delete returning no rows after returning many rows"
        data = [(i, "Test String %d" % i) for i in range(1, 11)]
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""", data)
        int_var = self.cursor.var(int)
        self.cursor.execute("""
                delete from TestTempTable
                where IntCol < :1
                returning IntCol into :2""", [5, int_var])
        self.assertEqual(int_var.getvalue(), [1, 2, 3, 4])
        self.cursor.execute(None, [4, int_var])
        self.assertEqual(int_var.getvalue(), [])

if __name__ == "__main__":
    test_env.run_test_cases()
