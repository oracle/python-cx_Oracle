#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing cursor objects."""

import cx_Oracle

class TestCursor(BaseTestCase):

    def testExecuteNoArgs(self):
        """test executing a statement without any arguments"""
        result = self.cursor.execute(u"begin null; end;")
        self.assertEqual(result, None)

    def testExecuteNoStatementWithArgs(self):
        """test executing a None statement with bind variables"""
        self.assertRaises(cx_Oracle.ProgrammingError, self.cursor.execute,
                None, x = 5)

    def testExecuteEmptyKeywordArgs(self):
        """test executing a statement with args and empty keyword args"""
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        args = [simpleVar]
        kwArgs = {}
        result = self.cursor.execute(u"begin :1 := 25; end;", args, **kwArgs)
        self.assertEqual(result, None)
        self.assertEqual(simpleVar.getvalue(), 25)

    def testExecuteKeywordArgs(self):
        """test executing a statement with keyword arguments"""
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        result = self.cursor.execute(u"begin :value := 5; end;",
                value = simpleVar)
        self.assertEqual(result, None)
        self.assertEqual(simpleVar.getvalue(), 5)

    def testExecuteDictionaryArg(self):
        """test executing a statement with a dictionary argument"""
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        dictArg = { u"value" : simpleVar }
        result = self.cursor.execute(u"begin :value := 10; end;", dictArg)
        self.assertEqual(result, None)
        self.assertEqual(simpleVar.getvalue(), 10)

    def testExecuteMultipleMethod(self):
        """test executing a statement with both a dict arg and keyword args"""
        simpleVar = self.cursor.var(cx_Oracle.NUMBER)
        dictArg = { u"value" : simpleVar }
        self.assertRaises(cx_Oracle.InterfaceError, self.cursor.execute,
                u"begin :value := 15; end;", dictArg, value = simpleVar)

    def testExecuteAndModifyArraySize(self):
        """test executing a statement and then changing the array size"""
        self.cursor.execute(u"select IntCol from TestNumbers")
        self.cursor.arraysize = 20
        self.assertEqual(len(self.cursor.fetchall()), 10)

    def testCallProc(self):
        """test executing a stored procedure"""
        var = self.cursor.var(cx_Oracle.NUMBER)
        results = self.cursor.callproc(u"proc_Test", (u"hi", 5, var))
        self.assertEqual(results, [u"hi", 10, 2.0])

    def testCallProcNoArgs(self):
        """test executing a stored procedure without any arguments"""
        results = self.cursor.callproc(u"proc_TestNoArgs")
        self.assertEqual(results, [])

    def testCallFunc(self):
        """test executing a stored function"""
        results = self.cursor.callfunc(u"func_Test", cx_Oracle.NUMBER,
                (u"hi", 5))
        self.assertEqual(results, 7)

    def testCallFuncNoArgs(self):
        """test executing a stored function without any arguments"""
        results = self.cursor.callfunc(u"func_TestNoArgs", cx_Oracle.NUMBER)
        self.assertEqual(results, 712)

    def testExecuteManyByName(self):
        """test executing a statement multiple times (named args)"""
        self.cursor.execute(u"truncate table TestTempTable")
        rows = [ { u"value" : n } for n in range(250) ]
        self.cursor.arraysize = 100
        statement = u"insert into TestTempTable (IntCol) values (:value)"
        self.cursor.executemany(statement, rows)
        self.connection.commit()
        self.cursor.execute(u"select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def testExecuteManyByPosition(self):
        """test executing a statement multiple times (positional args)"""
        self.cursor.execute(u"truncate table TestTempTable")
        rows = [ [n] for n in range(230) ]
        self.cursor.arraysize = 100
        statement = u"insert into TestTempTable (IntCol) values (:1)"
        self.cursor.executemany(statement, rows)
        self.connection.commit()
        self.cursor.execute(u"select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def testExecuteManyWithPrepare(self):
        """test executing a statement multiple times (with prepare)"""
        self.cursor.execute(u"truncate table TestTempTable")
        rows = [ [n] for n in range(225) ]
        self.cursor.arraysize = 100
        statement = u"insert into TestTempTable (IntCol) values (:1)"
        self.cursor.prepare(statement)
        self.cursor.executemany(None, rows)
        self.connection.commit()
        self.cursor.execute(u"select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def testExecuteManyWithRebind(self):
        """test executing a statement multiple times (with rebind)"""
        self.cursor.execute(u"truncate table TestTempTable")
        rows = [ [n] for n in range(235) ]
        self.cursor.arraysize = 100
        statement = u"insert into TestTempTable (IntCol) values (:1)"
        self.cursor.executemany(statement, rows[:50])
        self.cursor.executemany(statement, rows[50:])
        self.connection.commit()
        self.cursor.execute(u"select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def testExecuteManyWithExecption(self):
        """test executing a statement multiple times (with exception)"""
        self.cursor.execute(u"truncate table TestTempTable")
        rows = [ { u"value" : n } for n in (1, 2, 3, 2, 5) ]
        statement = u"insert into TestTempTable (IntCol) values (:value)"
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.executemany,
                statement, rows)
        self.assertEqual(self.cursor.rowcount, 3)

    def testPrepare(self):
        """test preparing a statement and executing it multiple times"""
        self.assertEqual(self.cursor.statement, None)
        statement = u"begin :value := :value + 5; end;"
        self.cursor.prepare(statement)
        var = self.cursor.var(cx_Oracle.NUMBER)
        self.assertEqual(self.cursor.statement, statement)
        var.setvalue(0, 2)
        self.cursor.execute(None, value = var)
        self.assertEqual(var.getvalue(), 7)
        self.cursor.execute(None, value = var)
        self.assertEqual(var.getvalue(), 12)
        self.cursor.execute(u"begin :value2 := 3; end;", value2 = var)
        self.assertEqual(var.getvalue(), 3)

    def testExceptionOnClose(self):
        "confirm an exception is raised after closing a cursor"
        self.cursor.close()
        self.assertRaises(cx_Oracle.InterfaceError, self.cursor.execute,
                u"select 1 from dual")

    def testIterators(self):
        """test iterators"""
        self.cursor.execute(u"""
                select IntCol
                from TestNumbers
                where IntCol between 1 and 3
                order by IntCol""")
        rows = []
        for row in self.cursor:
            rows.append(row[0])
        self.assertEqual(rows, [1, 2, 3])

    def testIteratorsInterrupted(self):
        """test iterators (with intermediate execute)"""
        self.cursor.execute(u"truncate table TestTempTable")
        self.cursor.execute(u"""
                select IntCol
                from TestNumbers
                where IntCol between 1 and 3
                order by IntCol""")
        testIter = iter(self.cursor)
        value, = testIter.next()
        self.cursor.execute(u"insert into TestTempTable (IntCol) values (1)")
        self.assertRaises(cx_Oracle.InterfaceError, testIter.next) 

    def testBindNames(self):
        """test that bindnames() works correctly."""
        self.assertRaises(cx_Oracle.ProgrammingError,
                self.cursor.bindnames)
        self.cursor.prepare(u"begin null; end;")
        self.assertEqual(self.cursor.bindnames(), [])
        self.cursor.prepare(u"begin :retval := :inval + 5; end;")
        self.assertEqual(self.cursor.bindnames(), ["RETVAL", "INVAL"])
        self.cursor.prepare(u"begin :retval := :a * :a + :b * :b; end;")
        self.assertEqual(self.cursor.bindnames(), ["RETVAL", "A", "B"])
        self.cursor.prepare(u"begin :a := :b + :c + :d + :e + :f + :g + " + \
                ":h + :i + :j + :k + :l; end;")
        self.assertEqual(self.cursor.bindnames(),
                [u"A", u"B", u"C", u"D", u"E", u"F", u"G", u"H", u"I", u"J",
                 u"K", u"L"])

    def testBadPrepare(self):
        """test that subsequent executes succeed after bad prepare"""
        self.assertRaises(cx_Oracle.DatabaseError,
                self.cursor.execute,
                u"begin raise_application_error(-20000, 'this); end;")
        self.cursor.execute(u"begin null; end;")

    def testBadExecute(self):
        """test that subsequent fetches fail after bad execute"""
        self.assertRaises(cx_Oracle.DatabaseError,
                self.cursor.execute, u"select y from dual")
        self.assertRaises(cx_Oracle.InterfaceError,
                self.cursor.fetchall)

    def testSetInputSizesMultipleMethod(self):
        """test setting input sizes with both positional and keyword args"""
        self.assertRaises(cx_Oracle.InterfaceError,
                self.cursor.setinputsizes, 5, x = 5)

    def testSetInputSizesByPosition(self):
        """test setting input sizes with positional args"""
        var = self.cursor.var(cx_Oracle.STRING, 100)
        self.cursor.setinputsizes(None, 5, None, 10, None, cx_Oracle.NUMBER)
        self.cursor.execute(u"""
                begin
                  :1 := :2 || to_char(:3) || :4 || to_char(:5) || to_char(:6);
                end;""", [var, u'test_', 5, u'_second_', 3, 7])
        self.assertEqual(var.getvalue(), u"test_5_second_37")

