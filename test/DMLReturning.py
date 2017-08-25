"""Module for testing DML returning clauses."""

import sys

class TestDMLReturning(BaseTestCase):

    def testInsert(self):
        "test insert statement with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        intVal = 5
        strVal = "A test string"
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str)
        self.cursor.execute("""
                insert into TestTempTable
                values (:intVal, :strVal)
                returning IntCol, StringCol into :intVar, :strVar""",
                intVal = intVal,
                strVal = strVal,
                intVar = intVar,
                strVar = strVar)
        self.assertEqual(intVar.values, [intVal])
        self.assertEqual(strVar.values, [strVal])

    def testInsertWithSmallSize(self):
        "test insert statement with DML returning into too small a variable"
        self.cursor.execute("truncate table TestTempTable")
        intVal = 6
        strVal = "A different test string"
        intVar = self.cursor.var(cx_Oracle.NUMBER)
        strVar = self.cursor.var(str, 2)
        parameters = dict(intVal = intVal, strVal = strVal, intVar = intVar,
                strVar = strVar)
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.execute, """
                insert into TestTempTable
                values (:intVal, :strVal)
                returning IntCol, StringCol into :intVar, :strVar""",
                parameters)

    def testUpdateSingleRow(self):
        "test update single row statement with DML returning"
        intVal = 7
        strVal = "The updated value of the string"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("insert into TestTempTable values (:1, :2)",
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
        self.assertEqual(intVar.values, [intVal])
        self.assertEqual(strVar.values, [strVal])

    def testUpdateNoRows(self):
        "test update no rows statement with DML returning"
        intVal = 8
        strVal = "The updated value of the string"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("insert into TestTempTable values (:1, :2)",
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
        self.assertEqual(intVar.values, [])
        self.assertEqual(strVar.values, [])

    def testUpdateMultipleRows(self):
        "test update multiple rows statement with DML returning"
        self.cursor.execute("truncate table TestTempTable")
        for i in (8, 9, 10):
            self.cursor.execute("insert into TestTempTable values (:1, :2)",
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
        self.assertEqual(intVar.values, [23, 24, 25])
        self.assertEqual(strVar.values, [
                "The final value of string 8",
                "The final value of string 9",
                "The final value of string 10"
        ])

    def testInsertAndReturnObject(self):
        "test inserting an object with DML returning"
        typeObj = self.connection.gettype("UDT_OBJECT")
        stringValue = "The string that will be verified"
        obj = typeObj.newobject()
        obj.STRINGVALUE = stringValue
        outVar = self.cursor.var(cx_Oracle.OBJECT, typename = "UDT_OBJECT")
        self.cursor.execute("""
                insert into TestObjects (IntCol, ObjectCol)
                values (4, :obj)
                returning ObjectCol into :outObj""",
                obj = obj, outObj = outVar)
        result = outVar.getvalue()
        self.assertEqual(result.STRINGVALUE, stringValue)
        self.connection.rollback()

