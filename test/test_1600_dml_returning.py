#------------------------------------------------------------------------------
# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
1600 - Module for testing DML returning clauses
"""

import TestEnv

import cx_Oracle

class TestCase(TestEnv.BaseTestCase):

    def test_1600_Insert(self):
        "1600 - test insert (single row) with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        intVal = 5
        strVal = "A test string"
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str)
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:intVal, :strVal)
                returning IntCol, StringCol into :intVar, :strVar""",
                intVal = intVal,
                strVal = strVal,
                intVar = intVar,
                strVar = strVar)
        self.assertEqual(intVar.values, [[intVal]])
        self.assertEqual(strVar.values, [[strVal]])

    def test_1601_InsertMany(self):
        "1601 - test insert (multiple rows) with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        intValues = [5, 8, 17, 24, 6]
        strValues = ["Test 5", "Test 8", "Test 17", "Test 24", "Test 6"]
        intVar = self.cursor.var(cx_Oracle.NUMBER, arraysize = len(intValues))
        strVar = self.cursor.var(str, arraysize = len(intValues))
        self.cursor.setinputsizes(None, None, intVar, strVar)
        data = list(zip(intValues, strValues))
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:intVal, :strVal)
                returning IntCol, StringCol into :intVar, :strVar""", data)
        self.assertEqual(intVar.values, [[v] for v in intValues])
        self.assertEqual(strVar.values, [[v] for v in strValues])

    def test_1602_InsertWithSmallSize(self):
        "1602 - test insert with DML returning into too small a variable"
        self.cursor.execute("truncate table TestTempTable")
        intVal = 6
        strVal = "A different test string"
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str, 2)
        parameters = dict(intVal = intVal, strVal = strVal, intVar = intVar,
                strVar = strVar)
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.execute, """
                insert into TestTempTable (IntCol, StringCol)
                values (:intVal, :strVal)
                returning IntCol, StringCol into :intVar, :strVar""",
                parameters)

    def test_1603_UpdateSingleRow(self):
        "1603 - test update single row with DML returning"
        intVal = 7
        strVal = "The updated value of the string"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""",
                (intVal, "The initial value of the string"))
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str)
        self.cursor.execute("""
                update TestTempTable set
                    StringCol = :strVal
                where IntCol = :intVal
                returning IntCol, StringCol into :intVar, :strVar""",
                intVal = intVal,
                strVal = strVal,
                intVar = intVar,
                strVar = strVar)
        self.assertEqual(intVar.values, [[intVal]])
        self.assertEqual(strVar.values, [[strVal]])

    def test_1604_UpdateNoRows(self):
        "1604 - test update no rows with DML returning"
        intVal = 8
        strVal = "The updated value of the string"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""",
                (intVal, "The initial value of the string"))
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str)
        self.cursor.execute("""
                update TestTempTable set
                    StringCol = :strVal
                where IntCol = :intVal
                returning IntCol, StringCol into :intVar, :strVar""",
                intVal = intVal + 1,
                strVal = strVal,
                intVar = intVar,
                strVar = strVar)
        self.assertEqual(intVar.values, [[]])
        self.assertEqual(strVar.values, [[]])
        self.assertEqual(intVar.getvalue(), [])
        self.assertEqual(strVar.getvalue(), [])

    def test_1605_UpdateMultipleRows(self):
        "1605 - test update multiple rows with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        for i in (8, 9, 10):
            self.cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (:1, :2)""",
                    (i, "The initial value of string %d" % i))
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str)
        self.cursor.execute("""
                update TestTempTable set
                    IntCol = IntCol + 15,
                    StringCol = 'The final value of string ' || to_char(IntCol)
                returning IntCol, StringCol into :intVar, :strVar""",
                intVar = intVar,
                strVar = strVar)
        self.assertEqual(self.cursor.rowcount, 3)
        self.assertEqual(intVar.values, [[23, 24, 25]])
        self.assertEqual(strVar.values, [[
                "The final value of string 8",
                "The final value of string 9",
                "The final value of string 10"
        ]])

    def test_1606_UpdateMultipleRowsExecuteMany(self):
        "1606 - test update multiple rows with DML returning (executeMany)"
        data = [(i, "The initial value of string %d" % i) \
                for i in range(1, 11)]
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""", data)
        intVar = self.cursor.var(cx_Oracle.NUMBER, arraysize = 3)
        strVar = self.cursor.var(str, arraysize = 3)
        self.cursor.setinputsizes(None, intVar, strVar)
        self.cursor.executemany("""
                update TestTempTable set
                    IntCol = IntCol + 25,
                    StringCol = 'Updated value of string ' || to_char(IntCol)
                where IntCol < :inVal
                returning IntCol, StringCol into :intVar, :strVar""",
                [[3], [8], [11]])
        self.assertEqual(intVar.values, [
                [26, 27],
                [28, 29, 30, 31, 32],
                [33, 34, 35]
        ])
        self.assertEqual(strVar.values, [
                [ "Updated value of string 1",
                  "Updated value of string 2" ],
                [ "Updated value of string 3",
                  "Updated value of string 4",
                  "Updated value of string 5",
                  "Updated value of string 6",
                  "Updated value of string 7" ],
                [ "Updated value of string 8",
                  "Updated value of string 9",
                  "Updated value of string 10" ]
        ])

    def test_1607_InsertAndReturnObject(self):
        "1607 - test inserting an object with DML returning"
        typeObj = self.connection.gettype("UDT_OBJECT")
        stringValue = "The string that will be verified"
        obj = typeObj.newobject()
        obj.STRINGVALUE = stringValue
        outVar = self.cursor.var(cx_Oracle.DB_TYPE_OBJECT,
                typename = "UDT_OBJECT")
        self.cursor.execute("""
                insert into TestObjects (IntCol, ObjectCol)
                values (4, :obj)
                returning ObjectCol into :outObj""",
                obj = obj, outObj = outVar)
        result, = outVar.getvalue()
        self.assertEqual(result.STRINGVALUE, stringValue)
        self.connection.rollback()

    def test_1608_InsertAndReturnRowid(self):
        "1608 - test inserting a row and returning a rowid"
        self.cursor.execute("truncate table TestTempTable")
        var = self.cursor.var(cx_Oracle.ROWID)
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

    def test_1609_InsertWithRefCursor(self):
        "1609 - test inserting with a REF cursor and returning a rowid"
        self.cursor.execute("truncate table TestTempTable")
        var = self.cursor.var(cx_Oracle.ROWID)
        inCursor = self.connection.cursor()
        inCursor.execute("""
                select StringCol
                from TestStrings
                where IntCol >= 5
                order by IntCol""")
        self.cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (187, pkg_TestRefCursors.TestInCursor(:1))
                returning rowid into :2""", (inCursor, var))
        rowid, = var.getvalue()
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                where rowid = :1""",
                (rowid,))
        self.assertEqual(self.cursor.fetchall(),
                [(187, 'String 7 (Modified)')])

    def test_1610_DeleteReturningDecreasingRowsReturned(self):
        "1610 - test delete returning decreasing number of rows"
        data = [(i, "Test String %d" % i) for i in range(1, 11)]
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""", data)
        results = []
        intVar = self.cursor.var(int)
        self.cursor.setinputsizes(None, intVar)
        for intVal in (5, 8, 10):
            self.cursor.execute("""
                    delete from TestTempTable
                    where IntCol < :1
                    returning IntCol into :2""", [intVal])
            results.append(intVar.getvalue())
        self.assertEqual(results, [ [1, 2, 3, 4], [5, 6, 7], [8, 9] ])

    def test_1611_DeleteReturningNoRowsAfterManyRows(self):
        "1611 - test delete returning no rows after returning many rows"
        data = [(i, "Test String %d" % i) for i in range(1, 11)]
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                insert into TestTempTable (IntCol, StringCol)
                values (:1, :2)""", data)
        intVar = self.cursor.var(int)
        self.cursor.execute("""
                delete from TestTempTable
                where IntCol < :1
                returning IntCol into :2""", [5, intVar])
        self.assertEqual(intVar.getvalue(), [1, 2, 3, 4])
        self.cursor.execute(None, [4, intVar])
        self.assertEqual(intVar.getvalue(), [])

if __name__ == "__main__":
    TestEnv.RunTestCases()
