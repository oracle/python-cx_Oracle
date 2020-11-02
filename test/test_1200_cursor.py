#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1200 - Module for testing cursors
"""

import TestEnv

import cx_Oracle
import decimal
import sys

class TestCase(TestEnv.BaseTestCase):

    def test_1200_CreateScrollableCursor(self):
        "1200 - test creating a scrollable cursor"
        cursor = self.connection.cursor()
        self.assertEqual(cursor.scrollable, False)
        cursor = self.connection.cursor(True)
        self.assertEqual(cursor.scrollable, True)
        cursor = self.connection.cursor(scrollable = True)
        self.assertEqual(cursor.scrollable, True)
        cursor.scrollable = False
        self.assertEqual(cursor.scrollable, False)

    def test_1201_ExecuteNoArgs(self):
        "1201 - test executing a statement without any arguments"
        result = self.cursor.execute("begin null; end;")
        self.assertEqual(result, None)

    def test_1202_ExecuteNoStatementWithArgs(self):
        "1202 - test executing a None statement with bind variables"
        self.assertRaises(cx_Oracle.ProgrammingError, self.cursor.execute,
                None, x = 5)

    def test_1203_ExecuteEmptyKeywordArgs(self):
        "1203 - test executing a statement with args and empty keyword args"
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        args = [simpleVar]
        kwArgs = {}
        result = self.cursor.execute("begin :1 := 25; end;", args, **kwArgs)
        self.assertEqual(result, None)
        self.assertEqual(simpleVar.getvalue(), 25)

    def test_1204_ExecuteKeywordArgs(self):
        "1204 - test executing a statement with keyword arguments"
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        result = self.cursor.execute("begin :value := 5; end;",
                value = simpleVar)
        self.assertEqual(result, None)
        self.assertEqual(simpleVar.getvalue(), 5)

    def test_1205_ExecuteDictionaryArg(self):
        "1205 - test executing a statement with a dictionary argument"
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        dictArg = { "value" : simpleVar }
        result = self.cursor.execute("begin :value := 10; end;", dictArg)
        self.assertEqual(result, None)
        self.assertEqual(simpleVar.getvalue(), 10)

    def test_1206_ExecuteMultipleMethod(self):
        "1206 - test executing a statement with both a dict and keyword args"
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        dictArg = { "value" : simpleVar }
        self.assertRaises(cx_Oracle.InterfaceError, self.cursor.execute,
                "begin :value := 15; end;", dictArg, value = simpleVar)

    def test_1207_ExecuteAndModifyArraySize(self):
        "1207 - test executing a statement and then changing the array size"
        self.cursor.execute("select IntCol from TestNumbers")
        self.cursor.arraysize = 20
        self.assertEqual(len(self.cursor.fetchall()), 10)

    def test_1208_CallProc(self):
        "1208 - test executing a stored procedure"
        var = self.cursor.var(cx_Oracle.NUMBER)
        results = self.cursor.callproc("proc_Test", ("hi", 5, var))
        self.assertEqual(results, ["hi", 10, 2.0])

    def test_1209_CallProcAllKeywords(self):
        "1209 - test executing a stored procedure with keyword args"
        kwargs = dict(a_InOutValue=self.cursor.var(cx_Oracle.NUMBER),
                a_InValue="hi", a_OutValue=self.cursor.var(cx_Oracle.NUMBER))
        kwargs['a_InOutValue'].setvalue(0, 5)
        results = self.cursor.callproc("proc_Test", keywordParameters=kwargs)
        self.assertEqual(results, [])
        self.assertEqual(kwargs['a_InOutValue'].getvalue(), 10)
        self.assertEqual(kwargs['a_OutValue'].getvalue(), 2.0)

    def test_1210_CallProcOnlyLastKeyword(self):
        "1210 - test executing a stored procedure with last arg as keyword arg"
        kwargs = dict(a_OutValue = self.cursor.var(cx_Oracle.NUMBER))
        results = self.cursor.callproc("proc_Test", ("hi", 5), kwargs)
        self.assertEqual(results, ["hi", 10])
        self.assertEqual(kwargs['a_OutValue'].getvalue(), 2.0)

    def test_1211_CallProcRepeatedKeywordParameters(self):
        "1211 - test executing a stored procedure, repeated keyword arg"
        kwargs = dict(a_InValue="hi",
                a_OutValue=self.cursor.var(cx_Oracle.NUMBER))
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.callproc,
                "proc_Test", parameters=("hi", 5), keywordParameters=kwargs)

    def test_1212_CallProcNoArgs(self):
        "1212 - test executing a stored procedure without any arguments"
        results = self.cursor.callproc("proc_TestNoArgs")
        self.assertEqual(results, [])

    def test_1213_CallFunc(self):
        "1213 - test executing a stored function"
        results = self.cursor.callfunc("func_Test", cx_Oracle.NUMBER,
                ("hi", 5))
        self.assertEqual(results, 7)

    def test_1214_CallFuncNoArgs(self):
        "1214 - test executing a stored function without any arguments"
        results = self.cursor.callfunc("func_TestNoArgs", cx_Oracle.NUMBER)
        self.assertEqual(results, 712)

    def test_1215_CallFuncNegative(self):
        "1215 - test executing a stored function with wrong parameters"
        funcName = "func_Test"
        self.assertRaises(TypeError, self.cursor.callfunc, cx_Oracle.NUMBER,
                funcName, ("hi", 5))
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.callfunc,
                funcName, cx_Oracle.NUMBER, ("hi", 5, 7))
        self.assertRaises(TypeError, self.cursor.callfunc, funcName,
                cx_Oracle.NUMBER, "hi", 7)
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.callfunc,
                funcName, cx_Oracle.NUMBER, [5, "hi"])
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.callfunc,
                funcName, cx_Oracle.NUMBER)
        self.assertRaises(TypeError, self.cursor.callfunc, funcName,
                cx_Oracle.NUMBER, 5)

    def test_1216_ExecuteManyByName(self):
        "1216 - test executing a statement multiple times (named args)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [ { "value" : n } for n in range(250) ]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:value)"
        self.cursor.executemany(statement, rows)
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1217_ExecuteManyByPosition(self):
        "1217 - test executing a statement multiple times (positional args)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [ [n] for n in range(230) ]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:1)"
        self.cursor.executemany(statement, rows)
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1218_ExecuteManyWithPrepare(self):
        "1218 - test executing a statement multiple times (with prepare)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [ [n] for n in range(225) ]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:1)"
        self.cursor.prepare(statement)
        self.cursor.executemany(None, rows)
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1219_ExecuteManyWithRebind(self):
        "1219 - test executing a statement multiple times (with rebind)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [ [n] for n in range(235) ]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:1)"
        self.cursor.executemany(statement, rows[:50])
        self.cursor.executemany(statement, rows[50:])
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1220_ExecuteManyWithInputSizesWrong(self):
        "1220 - test executing multiple times (with input sizes wrong)"
        cursor = self.connection.cursor()
        cursor.setinputsizes(cx_Oracle.NUMBER)
        data = [[decimal.Decimal("25.8")], [decimal.Decimal("30.0")]]
        cursor.executemany("declare t number; begin t := :1; end;", data)

    def test_1221_ExecuteManyMultipleBatches(self):
        "1221 - test executing multiple times (with multiple batches)"
        self.cursor.execute("truncate table TestTempTable")
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.executemany(sql, [(1, None), (2, None)])
        self.cursor.executemany(sql, [(3, None), (4, "Testing")])

    def test_1222_ExecuteManyNumeric(self):
        "1222 - test executemany() with various numeric types"
        self.cursor.execute("truncate table TestTempTable")
        data = [(1, 5), (2, 7.0), (3, 6.5), (4, 2 ** 65),
                (5, decimal.Decimal("24.5"))]
        sql = "insert into TestTempTable (IntCol, NumberCol) values (:1, :2)"
        self.cursor.executemany(sql, data)
        self.cursor.execute("""
                select IntCol, NumberCol
                from TestTempTable
                order by IntCol""")
        self.assertEqual(self.cursor.fetchall(), data)

    def test_1223_ExecuteManyWithResize(self):
        "1223 - test executing a statement multiple times (with resize)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [ ( 1, "First" ),
                 ( 2, "Second" ),
                 ( 3, "Third" ),
                 ( 4, "Fourth" ),
                 ( 5, "Fifth" ),
                 ( 6, "Sixth" ),
                 ( 7, "Seventh and the longest one" ) ]
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.executemany(sql, rows)
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                order by IntCol""")
        fetchedRows = self.cursor.fetchall()
        self.assertEqual(fetchedRows, rows)

    def test_1224_ExecuteManyWithExecption(self):
        "1224 - test executing a statement multiple times (with exception)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [ { "value" : n } for n in (1, 2, 3, 2, 5) ]
        statement = "insert into TestTempTable (IntCol) values (:value)"
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.executemany,
                statement, rows)
        self.assertEqual(self.cursor.rowcount, 3)

    def test_1225_ExecuteManyWithInvalidParameters(self):
        "1225 - test calling executemany() with invalid parameters"
        self.assertRaises(TypeError, self.cursor.executemany,
                "insert into TestTempTable (IntCol, StringCol) values (:1, :2)",
                "These are not valid parameters")

    def test_1226_ExecuteManyNoParameters(self):
        "1226 - test calling executemany() without any bind parameters"
        numRows = 5
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                declare
                    t_Id          number;
                begin
                    select nvl(count(*), 0) + 1 into t_Id
                    from TestTempTable;

                    insert into TestTempTable (IntCol, StringCol)
                    values (t_Id, 'Test String ' || t_Id);
                end;""", numRows)
        self.assertEqual(self.cursor.rowcount, numRows)
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, numRows)

    def test_1227_ExecuteManyBoundEarlier(self):
        "1227 - test calling executemany() with binds performed earlier"
        numRows = 9
        self.cursor.execute("truncate table TestTempTable")
        var = self.cursor.var(int, arraysize = numRows)
        self.cursor.setinputsizes(var)
        self.cursor.executemany("""
                declare
                    t_Id          number;
                begin
                    select nvl(count(*), 0) + 1 into t_Id
                    from TestTempTable;

                    insert into TestTempTable (IntCol, StringCol)
                    values (t_Id, 'Test String ' || t_Id);

                    select sum(IntCol) into :1
                    from TestTempTable;
                end;""", numRows)
        self.assertEqual(self.cursor.rowcount, numRows)
        expectedData = [1, 3, 6, 10, 15, 21, 28, 36, 45]
        self.assertEqual(var.values, expectedData)

    def test_1228_Prepare(self):
        "1228 - test preparing a statement and executing it multiple times"
        self.assertEqual(self.cursor.statement, None)
        statement = "begin :value := :value + 5; end;"
        self.cursor.prepare(statement)
        var = self.cursor.var(cx_Oracle.NUMBER)
        self.assertEqual(self.cursor.statement, statement)
        var.setvalue(0, 2)
        self.cursor.execute(None, value = var)
        self.assertEqual(var.getvalue(), 7)
        self.cursor.execute(None, value = var)
        self.assertEqual(var.getvalue(), 12)
        self.cursor.execute("begin :value2 := 3; end;", value2 = var)
        self.assertEqual(var.getvalue(), 3)

    def test_1229_ExceptionOnClose(self):
        "1229 - confirm an exception is raised after closing a cursor"
        self.cursor.close()
        self.assertRaises(cx_Oracle.InterfaceError, self.cursor.execute,
                "select 1 from dual")

    def test_1230_Iterators(self):
        "1230 - test iterators"
        self.cursor.execute("""
                select IntCol
                from TestNumbers
                where IntCol between 1 and 3
                order by IntCol""")
        rows = []
        for row in self.cursor:
            rows.append(row[0])
        self.assertEqual(rows, [1, 2, 3])

    def test_1231_IteratorsInterrupted(self):
        "1231 - test iterators (with intermediate execute)"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("""
                select IntCol
                from TestNumbers
                where IntCol between 1 and 3
                order by IntCol""")
        testIter = iter(self.cursor)
        if sys.version_info[0] >= 3:
            value, = next(testIter)
        else:
            value, = testIter.next()
        self.cursor.execute("insert into TestTempTable (IntCol) values (1)")
        if sys.version_info[0] >= 3:
            self.assertRaises(cx_Oracle.InterfaceError, next, testIter)
        else:
            self.assertRaises(cx_Oracle.InterfaceError, testIter.next)

    def test_1232_BindNames(self):
        "1232 - test that bindnames() works correctly."
        self.assertRaises(cx_Oracle.ProgrammingError, self.cursor.bindnames)
        self.cursor.prepare("begin null; end;")
        self.assertEqual(self.cursor.bindnames(), [])
        self.cursor.prepare("begin :retval := :inval + 5; end;")
        self.assertEqual(self.cursor.bindnames(), ["RETVAL", "INVAL"])
        self.cursor.prepare("begin :retval := :a * :a + :b * :b; end;")
        self.assertEqual(self.cursor.bindnames(), ["RETVAL", "A", "B"])
        self.cursor.prepare("begin :a := :b + :c + :d + :e + :f + :g + " + \
                ":h + :i + :j + :k + :l; end;")
        self.assertEqual(self.cursor.bindnames(),
                ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"])
        self.cursor.prepare("select :a * :a + :b * :b from dual")
        self.assertEqual(self.cursor.bindnames(), ["A", "B"])

    def test_1233_BadPrepare(self):
        "1233 - test that subsequent executes succeed after bad prepare"
        self.assertRaises(cx_Oracle.DatabaseError,
                self.cursor.execute,
                "begin raise_application_error(-20000, 'this); end;")
        self.cursor.execute("begin null; end;")

    def test_1234_BadExecute(self):
        "1234 - test that subsequent fetches fail after bad execute"
        self.assertRaises(cx_Oracle.DatabaseError,
                self.cursor.execute, "select y from dual")
        self.assertRaises(cx_Oracle.InterfaceError,
                self.cursor.fetchall)

    def test_1235_ScrollAbsoluteExceptionAfter(self):
        "1235 - test scrolling absolute yields an exception (after result set)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        self.assertRaises(cx_Oracle.DatabaseError, cursor.scroll, 12,
                "absolute")

    def test_1236_ScrollAbsoluteInBuffer(self):
        "1236 - test scrolling absolute (when in buffers)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        self.assertTrue(cursor.arraysize > 1,
                "array size must exceed 1 for this test to work correctly")
        cursor.scroll(1, mode = "absolute")
        row = cursor.fetchone()
        self.assertEqual(row[0], 1.25)
        self.assertEqual(cursor.rowcount, 1)

    def test_1237_ScrollAbsoluteNotInBuffer(self):
        "1237 - test scrolling absolute (when not in buffers)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.scroll(6, mode = "absolute")
        row = cursor.fetchone()
        self.assertEqual(row[0], 7.5)
        self.assertEqual(cursor.rowcount, 6)

    def test_1238_ScrollFirstInBuffer(self):
        "1238 - test scrolling to first row in result set (in buffers)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        cursor.scroll(mode = "first")
        row = cursor.fetchone()
        self.assertEqual(row[0], 1.25)
        self.assertEqual(cursor.rowcount, 1)

    def test_1239_ScrollFirstNotInBuffer(self):
        "1239 - test scrolling to first row in result set (not in buffers)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        cursor.fetchmany()
        cursor.scroll(mode = "first")
        row = cursor.fetchone()
        self.assertEqual(row[0], 1.25)
        self.assertEqual(cursor.rowcount, 1)

    def test_1240_ScrollLast(self):
        "1240 - test scrolling to last row in result set"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.scroll(mode = "last")
        row = cursor.fetchone()
        self.assertEqual(row[0], 12.5)
        self.assertEqual(cursor.rowcount, 10)

    def test_1241_ScrollRelativeExceptionAfter(self):
        "1241 - test scrolling relative yields an exception (after result set)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        self.assertRaises(cx_Oracle.DatabaseError, cursor.scroll, 15)

    def test_1242_ScrollRelativeExceptionBefore(self):
        "1242 - test scrolling relative yields exception (before result set)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        self.assertRaises(cx_Oracle.DatabaseError, cursor.scroll, -5)

    def test_1243_ScrollRelativeInBuffer(self):
        "1243 - test scrolling relative (when in buffers)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        self.assertTrue(cursor.arraysize > 1,
                "array size must exceed 1 for this test to work correctly")
        cursor.scroll(2 - cursor.rowcount)
        row = cursor.fetchone()
        self.assertEqual(row[0], 2.5)
        self.assertEqual(cursor.rowcount, 2)

    def test_1244_ScrollRelativeNotInBuffer(self):
        "1244 - test scrolling relative (when not in buffers)"
        cursor = self.connection.cursor(scrollable = True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        cursor.fetchmany()
        self.assertTrue(cursor.arraysize > 1,
                "array size must exceed 2 for this test to work correctly")
        cursor.scroll(3 - cursor.rowcount)
        row = cursor.fetchone()
        self.assertEqual(row[0], 3.75)
        self.assertEqual(cursor.rowcount, 3)

    def test_1245_ScrollNoRows(self):
        "1245 - test scrolling when there are no rows"
        self.cursor.execute("truncate table TestTempTable")
        cursor = self.connection.cursor(scrollable = True)
        cursor.execute("select * from TestTempTable")
        cursor.scroll(mode = "last")
        self.assertEqual(cursor.fetchall(), [])
        cursor.scroll(mode = "first")
        self.assertEqual(cursor.fetchall(), [])
        self.assertRaises(cx_Oracle.DatabaseError, cursor.scroll, 1,
                mode = "absolute")

    def test_1246_ScrollDifferingArrayAndFetchSizes(self):
        "1246 - test scrolling with differing array and fetch array sizes"
        self.cursor.execute("truncate table TestTempTable")
        for i in range(30):
            self.cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (:1, null)""",
                    (i + 1,))
        for arraySize in range(1, 6):
            cursor = self.connection.cursor(scrollable = True)
            cursor.arraysize = arraySize
            cursor.execute("select IntCol from TestTempTable order by IntCol")
            for numRows in range(1, arraySize + 1):
                cursor.scroll(15, "absolute")
                rows = cursor.fetchmany(numRows)
                self.assertEqual(rows[0][0], 15)
                self.assertEqual(cursor.rowcount, 15 + numRows - 1)
                cursor.scroll(9)
                rows = cursor.fetchmany(numRows)
                numRowsFetched = len(rows)
                self.assertEqual(rows[0][0], 15 + numRows + 8)
                self.assertEqual(cursor.rowcount,
                        15 + numRows + numRowsFetched + 7)
                cursor.scroll(-12)
                rows = cursor.fetchmany(numRows)
                self.assertEqual(rows[0][0], 15 + numRows + numRowsFetched - 5)
                self.assertEqual(cursor.rowcount,
                        15 + numRows + numRowsFetched + numRows - 6)

    def test_1247_SetInputSizesNegative(self):
        "1247 - test cursor.setinputsizes() with invalid parameters"
        val = decimal.Decimal(5)
        self.assertRaises(cx_Oracle.InterfaceError,
                self.cursor.setinputsizes, val, x = val)
        self.assertRaises(TypeError, self.cursor.setinputsizes, val)

    def test_1248_SetInputSizesNoParameters(self):
        "1248 - test setting input sizes without any parameters"
        self.cursor.setinputsizes()
        self.cursor.execute("select :val from dual", val = "Test Value")
        self.assertEqual(self.cursor.fetchall(), [("Test Value",)])

    def test_1249_SetInputSizesEmptyDict(self):
        "1249 - test setting input sizes with an empty dictionary"
        emptyDict = {}
        self.cursor.prepare("select 236 from dual")
        self.cursor.setinputsizes(**emptyDict)
        self.cursor.execute(None, emptyDict)
        self.assertEqual(self.cursor.fetchall(), [(236,)])

    def test_1250_SetInputSizesEmptyList(self):
        "1250 - test setting input sizes with an empty list"
        emptyList = {}
        self.cursor.prepare("select 239 from dual")
        self.cursor.setinputsizes(*emptyList)
        self.cursor.execute(None, emptyList)
        self.assertEqual(self.cursor.fetchall(), [(239,)])

    def test_1251_SetInputSizesByPosition(self):
        "1251 - test setting input sizes with positional args"
        var = self.cursor.var(cx_Oracle.STRING, 100)
        self.cursor.setinputsizes(None, 5, None, 10, None, cx_Oracle.NUMBER)
        self.cursor.execute("""
                begin
                  :1 := :2 || to_char(:3) || :4 || to_char(:5) || to_char(:6);
                end;""", [var, 'test_', 5, '_second_', 3, 7])
        self.assertEqual(var.getvalue(), "test_5_second_37")

    def test_1252_StringFormat(self):
        "1252 - test string format of cursor"
        formatString = "<cx_Oracle.Cursor on <cx_Oracle.Connection to %s@%s>>"
        expectedValue = formatString % \
                (TestEnv.GetMainUser(), TestEnv.GetConnectString())
        self.assertEqual(str(self.cursor), expectedValue)

    def test_1253_CursorFetchRaw(self):
        "1253 - test cursor.fetchraw()"
        cursor = self.connection.cursor()
        cursor.arraysize = 25
        cursor.execute("select LongIntCol from TestNumbers order by IntCol")
        self.assertEqual(cursor.fetchraw(), 10)
        self.assertEqual(cursor.fetchvars[0].getvalue(), 38)

    def test_1254_Parse(self):
        "1254 - test parsing statements"
        sql = "select LongIntCol from TestNumbers where IntCol = :val"
        self.cursor.parse(sql)
        self.assertEqual(self.cursor.statement, sql)
        self.assertEqual(self.cursor.description,
                [ ('LONGINTCOL', cx_Oracle.DB_TYPE_NUMBER, 17, None, 16, 0,
                        0) ])

    def test_1255_SetOutputSize(self):
        "1255 - test cursor.setoutputsize() does not fail (but does nothing)"
        self.cursor.setoutputsize(100, 2)

    def test_1256_VarNegative(self):
        "1256 - test cursor.var() with invalid parameters"
        self.assertRaises(TypeError, self.cursor.var, 5)

    def test_1257_ArrayVarNegative(self):
        "1257 - test cursor.arrayvar() with invalid parameters"
        self.assertRaises(TypeError, self.cursor.arrayvar, 5, 1)

    def test_1258_BooleanWithoutPlsql(self):
        "1258 - test binding boolean data without the use of PL/SQL"
        self.cursor.execute("truncate table TestTempTable")
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.execute(sql, (False, "Value should be 0"))
        self.cursor.execute(sql, (True, "Value should be 1"))
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                order by IntCol""")
        self.assertEqual(self.cursor.fetchall(),
                [ (0, "Value should be 0"), (1, "Value should be 1") ])

    def test_1259_AsContextManager(self):
        "1259 - test using a cursor as a context manager"
        with self.cursor as cursor:
            cursor.execute("truncate table TestTempTable")
            cursor.execute("select count(*) from TestTempTable")
            count, = cursor.fetchone()
            self.assertEqual(count, 0)
        self.assertRaises(cx_Oracle.InterfaceError, self.cursor.close)

    def test_1260_QueryRowCount(self):
        "1260 - test that rowcount attribute is reset to zero on query execute"
        sql = "select * from dual where 1 = :s"
        self.cursor.execute(sql, [0])
        self.cursor.fetchone()
        self.assertEqual(self.cursor.rowcount, 0)
        self.cursor.execute(sql, [1])
        self.cursor.fetchone()
        self.assertEqual(self.cursor.rowcount, 1)
        self.cursor.execute(sql, [1])
        self.cursor.fetchone()
        self.assertEqual(self.cursor.rowcount, 1)
        self.cursor.execute(sql, [0])
        self.cursor.fetchone()
        self.assertEqual(self.cursor.rowcount, 0)

    def test_1261_VarTypeNameNone(self):
        "1261 - test that the typename attribute can be passed a value of None"
        valueToSet = 5
        var = self.cursor.var(int, typename=None)
        var.setvalue(0, valueToSet)
        self.assertEqual(var.getvalue(), valueToSet)

    def test_1262_VarTypeWithObjectType(self):
        "1262 - test that an object type can be used as type in cursor.var()"
        objType = self.connection.gettype("UDT_OBJECT")
        var = self.cursor.var(objType)
        self.cursor.callproc("pkg_TestBindObject.BindObjectOut",
                (28, "Bind obj out", var))
        obj = var.getvalue()
        result = self.cursor.callfunc("pkg_TestBindObject.GetStringRep", str,
                (obj,))
        self.assertEqual(result,
                "udt_Object(28, 'Bind obj out', null, null, null, null, null)")

    def test_1263_FetchXMLType(self):
        "1263 - test that fetching an XMLType returns a string"
        intVal = 5
        label = "IntCol"
        expectedResult = "<%s>%s</%s>" % (label, intVal, label)
        self.cursor.execute("""
                select XMLElement("%s", IntCol)
                from TestStrings
                where IntCol = :intVal""" % label,
                intVal = intVal)
        result, = self.cursor.fetchone()
        self.assertEqual(result, expectedResult)

    def test_1264_LastRowid(self):
        "1264 - test last rowid"

        # no statement executed: no rowid
        self.assertEqual(None, self.cursor.lastrowid)

        # DDL statement executed: no rowid
        self.cursor.execute("truncate table TestTempTable")
        self.assertEqual(None, self.cursor.lastrowid)

        # statement prepared: no rowid
        self.cursor.prepare("insert into TestTempTable (IntCol) values (:1)")
        self.assertEqual(None, self.cursor.lastrowid)

        # multiple rows inserted: rowid of last row inserted
        rows = [(n,) for n in range(225)]
        self.cursor.executemany(None, rows)
        rowid = self.cursor.lastrowid
        self.cursor.execute("""
                select rowid
                from TestTempTable
                where IntCol = :1""", rows[-1])
        self.assertEqual(rowid, self.cursor.fetchone()[0])

        # statement executed but no rows updated: no rowid
        self.cursor.execute("delete from TestTempTable where 1 = 0")
        self.assertEqual(None, self.cursor.lastrowid)

        # stetement executed with one row updated: rowid of updated row
        self.cursor.execute("""
                update TestTempTable set
                    StringCol = 'Modified'
                where IntCol = :1""", rows[-2])
        rowid = self.cursor.lastrowid
        self.cursor.execute("""
                select rowid
                from TestTempTable
                where IntCol = :1""", rows[-2])
        self.assertEqual(rowid, self.cursor.fetchone()[0])

        # statement executed with many rows updated: rowid of last updated row
        self.cursor.execute("""
                update TestTempTable set
                    StringCol = 'Row ' || to_char(IntCol)
                where IntCol = :1""", rows[-3])
        rowid = self.cursor.lastrowid
        self.cursor.execute("""
                select StringCol
                from TestTempTable
                where rowid = :1""", [rowid])
        self.assertEqual("Row %s" % rows[-3], self.cursor.fetchone()[0])

    def test_1265_PrefetchRows(self):
        "1265 - test prefetch rows"
        self.setUpRoundTripChecker()

        # perform simple query and verify only one round trip is needed
        with self.connection.cursor() as cursor:
            cursor.execute("select sysdate from dual").fetchall()
            self.assertRoundTrips(1)

        # set prefetchrows to 1 and verify that two round trips are now needed
        with self.connection.cursor() as cursor:
            cursor.prefetchrows = 1
            cursor.execute("select sysdate from dual").fetchall()
            self.assertRoundTrips(2)

        # simple DDL only requires a single round trip
        with self.connection.cursor() as cursor:
            cursor.execute("truncate table TestTempTable")
            self.assertRoundTrips(1)

        # array execution only requires a single round trip
        numRows = 590
        with self.connection.cursor() as cursor:
            sql = "insert into TestTempTable (IntCol) values (:1)"
            data = [(n + 1,) for n in range(numRows)]
            cursor.executemany(sql, data)
            self.assertRoundTrips(1)

        # setting prefetch and array size to 1 requires a round-trip for each
        # row
        with self.connection.cursor() as cursor:
            cursor.prefetchrows = 1
            cursor.arraysize = 1
            cursor.execute("select IntCol from TestTempTable").fetchall()
            self.assertRoundTrips(numRows + 1)

        # setting prefetch and array size to 300 requires 2 round-trips
        with self.connection.cursor() as cursor:
            cursor.prefetchrows = 300
            cursor.arraysize = 300
            cursor.execute("select IntCol from TestTempTable").fetchall()
            self.assertRoundTrips(2)

if __name__ == "__main__":
    TestEnv.RunTestCases()
