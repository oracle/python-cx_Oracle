#------------------------------------------------------------------------------
# Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1200 - Module for testing cursors
"""

import decimal

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def test_1200_create_scrollable_cursor(self):
        "1200 - test creating a scrollable cursor"
        cursor = self.connection.cursor()
        self.assertEqual(cursor.scrollable, False)
        cursor = self.connection.cursor(True)
        self.assertEqual(cursor.scrollable, True)
        cursor = self.connection.cursor(scrollable=True)
        self.assertEqual(cursor.scrollable, True)
        cursor.scrollable = False
        self.assertEqual(cursor.scrollable, False)

    def test_1201_execute_no_args(self):
        "1201 - test executing a statement without any arguments"
        result = self.cursor.execute("begin null; end;")
        self.assertEqual(result, None)

    def test_1202_execute_no_statement_with_args(self):
        "1202 - test executing a None statement with bind variables"
        self.assertRaises(oracledb.ProgrammingError, self.cursor.execute, None,
                          x=5)

    def test_1203_execute_empty_keyword_args(self):
        "1203 - test executing a statement with args and empty keyword args"
        simple_var = self.cursor.var(oracledb.NUMBER)
        args = [simple_var]
        kwargs = {}
        result = self.cursor.execute("begin :1 := 25; end;", args, **kwargs)
        self.assertEqual(result, None)
        self.assertEqual(simple_var.getvalue(), 25)

    def test_1204_execute_keyword_args(self):
        "1204 - test executing a statement with keyword arguments"
        simple_var = self.cursor.var(oracledb.NUMBER)
        result = self.cursor.execute("begin :value := 5; end;",
                                     value=simple_var)
        self.assertEqual(result, None)
        self.assertEqual(simple_var.getvalue(), 5)

    def test_1205_execute_dictionary_arg(self):
        "1205 - test executing a statement with a dictionary argument"
        simple_var = self.cursor.var(oracledb.NUMBER)
        dict_arg = dict(value=simple_var)
        result = self.cursor.execute("begin :value := 10; end;", dict_arg)
        self.assertEqual(result, None)
        self.assertEqual(simple_var.getvalue(), 10)

    def test_1206_execute_multiple_arg_types(self):
        "1206 - test executing a statement with both a dict and keyword args"
        simple_var = self.cursor.var(oracledb.NUMBER)
        dict_arg = dict(value=simple_var)
        self.assertRaises(oracledb.InterfaceError, self.cursor.execute,
                          "begin :value := 15; end;", dict_arg,
                          value=simple_var)

    def test_1207_execute_and_modify_array_size(self):
        "1207 - test executing a statement and then changing the array size"
        self.cursor.execute("select IntCol from TestNumbers")
        self.cursor.arraysize = 20
        self.assertEqual(len(self.cursor.fetchall()), 10)

    def test_1208_callproc(self):
        "1208 - test executing a stored procedure"
        var = self.cursor.var(oracledb.NUMBER)
        results = self.cursor.callproc("proc_Test", ("hi", 5, var))
        self.assertEqual(results, ["hi", 10, 2.0])

    def test_1209_callproc_all_keywords(self):
        "1209 - test executing a stored procedure with all args keyword args"
        inout_value = self.cursor.var(oracledb.NUMBER)
        inout_value.setvalue(0, 5)
        out_value = self.cursor.var(oracledb.NUMBER)
        kwargs = dict(a_InOutValue=inout_value, a_InValue="hi",
                      a_OutValue=out_value)
        results = self.cursor.callproc("proc_Test", [], kwargs)
        self.assertEqual(results, [])
        self.assertEqual(inout_value.getvalue(), 10)
        self.assertEqual(out_value.getvalue(), 2.0)

    def test_1210_callproc_only_last_keyword(self):
        "1210 - test executing a stored procedure with last arg as keyword arg"
        out_value = self.cursor.var(oracledb.NUMBER)
        kwargs = dict(a_OutValue=out_value)
        results = self.cursor.callproc("proc_Test", ("hi", 5), kwargs)
        self.assertEqual(results, ["hi", 10])
        self.assertEqual(out_value.getvalue(), 2.0)

    def test_1211_callproc_repeated_keyword_parameters(self):
        "1211 - test executing a stored procedure, repeated keyword arg"
        kwargs = dict(a_InValue="hi",
                      a_OutValue=self.cursor.var(oracledb.NUMBER))
        self.assertRaises(oracledb.DatabaseError, self.cursor.callproc,
                          "proc_Test", ("hi", 5), kwargs)

    def test_1212_callproc_no_args(self):
        "1212 - test executing a stored procedure without any arguments"
        results = self.cursor.callproc("proc_TestNoArgs")
        self.assertEqual(results, [])

    def test_1213_callfunc(self):
        "1213 - test executing a stored function"
        results = self.cursor.callfunc("func_Test", oracledb.NUMBER, ("hi", 5))
        self.assertEqual(results, 7)

    def test_1214_callfunc_no_args(self):
        "1214 - test executing a stored function without any arguments"
        results = self.cursor.callfunc("func_TestNoArgs", oracledb.NUMBER)
        self.assertEqual(results, 712)

    def test_1215_callfunc_negative(self):
        "1215 - test executing a stored function with wrong parameters"
        func_name = "func_Test"
        self.assertRaises(TypeError, self.cursor.callfunc, oracledb.NUMBER,
                          func_name, ("hi", 5))
        self.assertRaises(oracledb.DatabaseError, self.cursor.callfunc,
                          func_name, oracledb.NUMBER, ("hi", 5, 7))
        self.assertRaises(TypeError, self.cursor.callfunc, func_name,
                          oracledb.NUMBER, "hi", 7)
        self.assertRaises(oracledb.DatabaseError, self.cursor.callfunc,
                          func_name, oracledb.NUMBER, [5, "hi"])
        self.assertRaises(oracledb.DatabaseError, self.cursor.callfunc,
                          func_name, oracledb.NUMBER)
        self.assertRaises(TypeError, self.cursor.callfunc, func_name,
                          oracledb.NUMBER, 5)

    def test_1216_executemany_by_name(self):
        "1216 - test executing a statement multiple times (named args)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [{"value": n} for n in range(250)]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:value)"
        self.cursor.executemany(statement, rows)
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1217_executemany_by_position(self):
        "1217 - test executing a statement multiple times (positional args)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [[n] for n in range(230)]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:1)"
        self.cursor.executemany(statement, rows)
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1218_executemany_with_prepare(self):
        "1218 - test executing a statement multiple times (with prepare)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [[n] for n in range(225)]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:1)"
        self.cursor.prepare(statement)
        self.cursor.executemany(None, rows)
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1219_executemany_with_rebind(self):
        "1219 - test executing a statement multiple times (with rebind)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [[n] for n in range(235)]
        self.cursor.arraysize = 100
        statement = "insert into TestTempTable (IntCol) values (:1)"
        self.cursor.executemany(statement, rows[:50])
        self.cursor.executemany(statement, rows[50:])
        self.connection.commit()
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_1220_executemany_with_input_sizes_wrong(self):
        "1220 - test executing multiple times (with input sizes wrong)"
        cursor = self.connection.cursor()
        cursor.setinputsizes(oracledb.NUMBER)
        data = [[decimal.Decimal("25.8")], [decimal.Decimal("30.0")]]
        cursor.executemany("declare t number; begin t := :1; end;", data)

    def test_1221_executemany_with_multiple_batches(self):
        "1221 - test executing multiple times (with multiple batches)"
        self.cursor.execute("truncate table TestTempTable")
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.executemany(sql, [(1, None), (2, None)])
        self.cursor.executemany(sql, [(3, None), (4, "Testing")])

    def test_1222_executemany_numeric(self):
        "1222 - test executemany() with various numeric types"
        self.cursor.execute("truncate table TestTempTable")
        data = [
            (1, 5),
            (2, 7.0),
            (3, 6.5),
            (4, 2 ** 65),
            (5, decimal.Decimal("24.5"))
        ]
        sql = "insert into TestTempTable (IntCol, NumberCol) values (:1, :2)"
        self.cursor.executemany(sql, data)
        self.cursor.execute("""
                select IntCol, NumberCol
                from TestTempTable
                order by IntCol""")
        self.assertEqual(self.cursor.fetchall(), data)

    def test_1223_executemany_with_resize(self):
        "1223 - test executing a statement multiple times (with resize)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [
             (1, "First"),
             (2, "Second"),
             (3, "Third"),
             (4, "Fourth"),
             (5, "Fifth"),
             (6, "Sixth"),
             (7, "Seventh and the longest one")
        ]
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.executemany(sql, rows)
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                order by IntCol""")
        fetched_rows = self.cursor.fetchall()
        self.assertEqual(fetched_rows, rows)

    def test_1224_executemany_with_exception(self):
        "1224 - test executing a statement multiple times (with exception)"
        self.cursor.execute("truncate table TestTempTable")
        rows = [{"value": n} for n in (1, 2, 3, 2, 5)]
        statement = "insert into TestTempTable (IntCol) values (:value)"
        self.assertRaises(oracledb.DatabaseError, self.cursor.executemany,
                          statement, rows)
        self.assertEqual(self.cursor.rowcount, 3)

    def test_1225_executemany_with_invalid_parameters(self):
        "1225 - test calling executemany() with invalid parameters"
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.assertRaises(TypeError, self.cursor.executemany, sql,
                          "These are not valid parameters")

    def test_1226_executemany_no_parameters(self):
        "1226 - test calling executemany() without any bind parameters"
        num_rows = 5
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.executemany("""
                declare
                    t_Id          number;
                begin
                    select nvl(count(*), 0) + 1 into t_Id
                    from TestTempTable;

                    insert into TestTempTable (IntCol, StringCol)
                    values (t_Id, 'Test String ' || t_Id);
                end;""", num_rows)
        self.assertEqual(self.cursor.rowcount, num_rows)
        self.cursor.execute("select count(*) from TestTempTable")
        count, = self.cursor.fetchone()
        self.assertEqual(count, num_rows)

    def test_1227_executemany_bound_earlier(self):
        "1227 - test calling executemany() with binds performed earlier"
        num_rows = 9
        self.cursor.execute("truncate table TestTempTable")
        var = self.cursor.var(int, arraysize=num_rows)
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
                end;""", num_rows)
        self.assertEqual(self.cursor.rowcount, num_rows)
        expected_data = [1, 3, 6, 10, 15, 21, 28, 36, 45]
        self.assertEqual(var.values, expected_data)

    def test_1228_prepare(self):
        "1228 - test preparing a statement and executing it multiple times"
        self.assertEqual(self.cursor.statement, None)
        statement = "begin :value := :value + 5; end;"
        self.cursor.prepare(statement)
        var = self.cursor.var(oracledb.NUMBER)
        self.assertEqual(self.cursor.statement, statement)
        var.setvalue(0, 2)
        self.cursor.execute(None, value = var)
        self.assertEqual(var.getvalue(), 7)
        self.cursor.execute(None, value = var)
        self.assertEqual(var.getvalue(), 12)
        self.cursor.execute("begin :value2 := 3; end;", value2 = var)
        self.assertEqual(var.getvalue(), 3)

    def test_1229_exception_on_close(self):
        "1229 - confirm an exception is raised after closing a cursor"
        self.cursor.close()
        self.assertRaises(oracledb.InterfaceError, self.cursor.execute,
                          "select 1 from dual")

    def test_1230_iterators(self):
        "1230 - test iterators"
        self.cursor.execute("""
                select IntCol
                from TestNumbers
                where IntCol between 1 and 3
                order by IntCol""")
        rows = [v for v, in self.cursor]
        self.assertEqual(rows, [1, 2, 3])

    def test_1231_iterators_interrupted(self):
        "1231 - test iterators (with intermediate execute)"
        self.cursor.execute("truncate table TestTempTable")
        self.cursor.execute("""
                select IntCol
                from TestNumbers
                where IntCol between 1 and 3
                order by IntCol""")
        test_iter = iter(self.cursor)
        value, = next(test_iter)
        self.cursor.execute("insert into TestTempTable (IntCol) values (1)")
        self.assertRaises(oracledb.InterfaceError, next, test_iter)

    def test_1232_bind_names(self):
        "1232 - test that bindnames() works correctly."
        self.assertRaises(oracledb.ProgrammingError, self.cursor.bindnames)
        self.cursor.prepare("begin null; end;")
        self.assertEqual(self.cursor.bindnames(), [])
        self.cursor.prepare("begin :retval := :inval + 5; end;")
        self.assertEqual(self.cursor.bindnames(), ["RETVAL", "INVAL"])
        self.cursor.prepare("begin :retval := :a * :a + :b * :b; end;")
        self.assertEqual(self.cursor.bindnames(), ["RETVAL", "A", "B"])
        self.cursor.prepare("begin :a := :b + :c + :d + :e + :f + :g + " + \
                            ":h + :i + :j + :k + :l; end;")
        names = ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"]
        self.assertEqual(self.cursor.bindnames(), names)
        self.cursor.prepare("select :a * :a + :b * :b from dual")
        self.assertEqual(self.cursor.bindnames(), ["A", "B"])

    def test_1233_bad_execute(self):
        "1233 - test that subsequent executes succeed after bad execute"
        self.assertRaises(oracledb.DatabaseError,
                          self.cursor.execute,
                          "begin raise_application_error(-20000, 'this); end;")
        self.cursor.execute("begin null; end;")

    def test_1234_fetch_after_bad_execute(self):
        "1234 - test that subsequent fetches fail after bad execute"
        self.assertRaises(oracledb.DatabaseError,
                          self.cursor.execute, "select y from dual")
        self.assertRaises(oracledb.InterfaceError, self.cursor.fetchall)

    def test_1235_scroll_absolute_exception_after(self):
        "1235 - test scrolling absolute yields an exception (after result set)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        self.assertRaises(oracledb.DatabaseError, cursor.scroll, 12,
                          "absolute")

    def test_1236_scroll_absolute_in_buffer(self):
        "1236 - test scrolling absolute (when in buffers)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        self.assertTrue(cursor.arraysize > 1,
                "array size must exceed 1 for this test to work correctly")
        cursor.scroll(1, mode="absolute")
        row = cursor.fetchone()
        self.assertEqual(row[0], 1.25)
        self.assertEqual(cursor.rowcount, 1)

    def test_1237_scroll_absolute_not_in_buffer(self):
        "1237 - test scrolling absolute (when not in buffers)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.scroll(6, mode="absolute")
        row = cursor.fetchone()
        self.assertEqual(row[0], 7.5)
        self.assertEqual(cursor.rowcount, 6)

    def test_1238_scroll_first_in_buffer(self):
        "1238 - test scrolling to first row in result set (in buffers)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        cursor.scroll(mode="first")
        row = cursor.fetchone()
        self.assertEqual(row[0], 1.25)
        self.assertEqual(cursor.rowcount, 1)

    def test_1239_scroll_first_not_in_buffer(self):
        "1239 - test scrolling to first row in result set (not in buffers)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        cursor.fetchmany()
        cursor.scroll(mode="first")
        row = cursor.fetchone()
        self.assertEqual(row[0], 1.25)
        self.assertEqual(cursor.rowcount, 1)

    def test_1240_scroll_last(self):
        "1240 - test scrolling to last row in result set"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.scroll(mode="last")
        row = cursor.fetchone()
        self.assertEqual(row[0], 12.5)
        self.assertEqual(cursor.rowcount, 10)

    def test_1241_scroll_relative_exception_after(self):
        "1241 - test scrolling relative yields an exception (after result set)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        self.assertRaises(oracledb.DatabaseError, cursor.scroll, 15)

    def test_1242_scroll_relative_exception_before(self):
        "1242 - test scrolling relative yields exception (before result set)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        self.assertRaises(oracledb.DatabaseError, cursor.scroll, -5)

    def test_1243_scroll_relative_in_buffer(self):
        "1243 - test scrolling relative (when in buffers)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        message = "array size must exceed 1 for this test to work correctly"
        self.assertTrue(cursor.arraysize > 1, message)
        cursor.scroll(2 - cursor.rowcount)
        row = cursor.fetchone()
        self.assertEqual(row[0], 2.5)
        self.assertEqual(cursor.rowcount, 2)

    def test_1244_scroll_relative_not_in_buffer(self):
        "1244 - test scrolling relative (when not in buffers)"
        cursor = self.connection.cursor(scrollable=True)
        cursor.arraysize = self.cursor.arraysize
        cursor.execute("""
                select NumberCol
                from TestNumbers
                order by IntCol""")
        cursor.fetchmany()
        cursor.fetchmany()
        message = "array size must exceed 1 for this test to work correctly"
        self.assertTrue(cursor.arraysize > 1, message)
        cursor.scroll(3 - cursor.rowcount)
        row = cursor.fetchone()
        self.assertEqual(row[0], 3.75)
        self.assertEqual(cursor.rowcount, 3)

    def test_1245_scroll_no_rows(self):
        "1245 - test scrolling when there are no rows"
        self.cursor.execute("truncate table TestTempTable")
        cursor = self.connection.cursor(scrollable=True)
        cursor.execute("select * from TestTempTable")
        cursor.scroll(mode="last")
        self.assertEqual(cursor.fetchall(), [])
        cursor.scroll(mode="first")
        self.assertEqual(cursor.fetchall(), [])
        self.assertRaises(oracledb.DatabaseError, cursor.scroll, 1,
                          mode="absolute")

    def test_1246_scroll_differing_array_and_fetch_sizes(self):
        "1246 - test scrolling with differing array and fetch array sizes"
        self.cursor.execute("truncate table TestTempTable")
        for i in range(30):
            self.cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (:1, null)""",
                    (i + 1,))
        for arraysize in range(1, 6):
            cursor = self.connection.cursor(scrollable = True)
            cursor.arraysize = arraysize
            cursor.execute("select IntCol from TestTempTable order by IntCol")
            for num_rows in range(1, arraysize + 1):
                cursor.scroll(15, "absolute")
                rows = cursor.fetchmany(num_rows)
                self.assertEqual(rows[0][0], 15)
                self.assertEqual(cursor.rowcount, 15 + num_rows - 1)
                cursor.scroll(9)
                rows = cursor.fetchmany(num_rows)
                num_rows_fetched = len(rows)
                self.assertEqual(rows[0][0], 15 + num_rows + 8)
                self.assertEqual(cursor.rowcount,
                        15 + num_rows + num_rows_fetched + 7)
                cursor.scroll(-12)
                rows = cursor.fetchmany(num_rows)
                count = 15 + num_rows + num_rows_fetched - 5
                self.assertEqual(rows[0][0], count)
                count = 15 + num_rows + num_rows_fetched + num_rows - 6
                self.assertEqual(cursor.rowcount, count)

    def test_1247_set_input_sizes_negative(self):
        "1247 - test cursor.setinputsizes() with invalid parameters"
        val = decimal.Decimal(5)
        self.assertRaises(oracledb.InterfaceError,
                          self.cursor.setinputsizes, val, x=val)
        self.assertRaises(TypeError, self.cursor.setinputsizes, val)

    def test_1248_set_input_sizes_no_parameters(self):
        "1248 - test setting input sizes without any parameters"
        self.cursor.setinputsizes()
        self.cursor.execute("select :val from dual", val="Test Value")
        self.assertEqual(self.cursor.fetchall(), [("Test Value",)])

    def test_1249_set_input_sizes_empty_dict(self):
        "1249 - test setting input sizes with an empty dictionary"
        empty_dict = {}
        self.cursor.prepare("select 236 from dual")
        self.cursor.setinputsizes(**empty_dict)
        self.cursor.execute(None, empty_dict)
        self.assertEqual(self.cursor.fetchall(), [(236,)])

    def test_1250_set_input_sizes_empty_list(self):
        "1250 - test setting input sizes with an empty list"
        empty_list = {}
        self.cursor.prepare("select 239 from dual")
        self.cursor.setinputsizes(*empty_list)
        self.cursor.execute(None, empty_list)
        self.assertEqual(self.cursor.fetchall(), [(239,)])

    def test_1251_set_input_sizes_by_position(self):
        "1251 - test setting input sizes with positional args"
        var = self.cursor.var(oracledb.STRING, 100)
        self.cursor.setinputsizes(None, 5, None, 10, None, oracledb.NUMBER)
        self.cursor.execute("""
                begin
                  :1 := :2 || to_char(:3) || :4 || to_char(:5) || to_char(:6);
                end;""", [var, 'test_', 5, '_second_', 3, 7])
        self.assertEqual(var.getvalue(), "test_5_second_37")

    def test_1252_string_format(self):
        "1252 - test string format of cursor"
        format_string = "<cx_Oracle.Cursor on <cx_Oracle.Connection to %s@%s>>"
        expected_value = format_string % \
                (test_env.get_main_user(), test_env.get_connect_string())
        self.assertEqual(str(self.cursor), expected_value)

    def test_1253_cursor_fetch_raw(self):
        "1253 - test cursor.fetchraw()"
        cursor = self.connection.cursor()
        cursor.arraysize = 25
        cursor.execute("select LongIntCol from TestNumbers order by IntCol")
        self.assertEqual(cursor.fetchraw(), 10)
        self.assertEqual(cursor.fetchvars[0].getvalue(), 38)

    def test_1254_parse_query(self):
        "1254 - test parsing query statements"
        sql = "select LongIntCol from TestNumbers where IntCol = :val"
        self.cursor.parse(sql)
        self.assertEqual(self.cursor.statement, sql)
        self.assertEqual(self.cursor.description,
                         [('LONGINTCOL', oracledb.DB_TYPE_NUMBER, 17, None,
                           16, 0, 0)])

    def test_1255_set_output_size(self):
        "1255 - test cursor.setoutputsize() does not fail (but does nothing)"
        self.cursor.setoutputsize(100, 2)

    def test_1256_var_negative(self):
        "1256 - test cursor.var() with invalid parameters"
        self.assertRaises(TypeError, self.cursor.var, 5)

    def test_1257_arrayvar_negative(self):
        "1257 - test cursor.arrayvar() with invalid parameters"
        self.assertRaises(TypeError, self.cursor.arrayvar, 5, 1)

    def test_1258_boolean_without_plsql(self):
        "1258 - test binding boolean data without the use of PL/SQL"
        self.cursor.execute("truncate table TestTempTable")
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.execute(sql, (False, "Value should be 0"))
        self.cursor.execute(sql, (True, "Value should be 1"))
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                order by IntCol""")
        expected_value = [(0, "Value should be 0"), (1, "Value should be 1")]
        self.assertEqual(self.cursor.fetchall(), expected_value)

    def test_1259_as_context_manager(self):
        "1259 - test using a cursor as a context manager"
        with self.cursor as cursor:
            cursor.execute("truncate table TestTempTable")
            cursor.execute("select count(*) from TestTempTable")
            count, = cursor.fetchone()
            self.assertEqual(count, 0)
        self.assertRaises(oracledb.InterfaceError, self.cursor.close)

    def test_1260_query_row_count(self):
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

    def test_1261_var_type_name_none(self):
        "1261 - test that the typename attribute can be passed a value of None"
        value_to_set = 5
        var = self.cursor.var(int, typename=None)
        var.setvalue(0, value_to_set)
        self.assertEqual(var.getvalue(), value_to_set)

    def test_1262_var_type_with_object_type(self):
        "1262 - test that an object type can be used as type in cursor.var()"
        obj_type = self.connection.gettype("UDT_OBJECT")
        var = self.cursor.var(obj_type)
        self.cursor.callproc("pkg_TestBindObject.BindObjectOut",
                             (28, "Bind obj out", var))
        obj = var.getvalue()
        result = self.cursor.callfunc("pkg_TestBindObject.GetStringRep", str,
                                      (obj,))
        exp = "udt_Object(28, 'Bind obj out', null, null, null, null, null)"
        self.assertEqual(result, exp)

    def test_1263_fetch_xmltype(self):
        "1263 - test that fetching an XMLType returns a string"
        int_val = 5
        label = "IntCol"
        expected_result = "<%s>%s</%s>" % (label, int_val, label)
        self.cursor.execute("""
                select XMLElement("%s", IntCol)
                from TestStrings
                where IntCol = :int_val""" % label,
                int_val=int_val)
        result, = self.cursor.fetchone()
        self.assertEqual(result, expected_result)

    def test_1264_lastrowid(self):
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

    def test_1265_prefetchrows(self):
        "1265 - test prefetch rows"
        self.setup_round_trip_checker()

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
        num_rows = 590
        with self.connection.cursor() as cursor:
            sql = "insert into TestTempTable (IntCol) values (:1)"
            data = [(n + 1,) for n in range(num_rows)]
            cursor.executemany(sql, data)
            self.assertRoundTrips(1)

        # setting prefetch and array size to 1 requires a round-trip for each
        # row
        with self.connection.cursor() as cursor:
            cursor.prefetchrows = 1
            cursor.arraysize = 1
            cursor.execute("select IntCol from TestTempTable").fetchall()
            self.assertRoundTrips(num_rows + 1)

        # setting prefetch and array size to 300 requires 2 round-trips
        with self.connection.cursor() as cursor:
            cursor.prefetchrows = 300
            cursor.arraysize = 300
            cursor.execute("select IntCol from TestTempTable").fetchall()
            self.assertRoundTrips(2)

    def test_1266_refcursor_prefetchrows(self):
        "1266 - test prefetch rows and arraysize using a refcursor"
        self.setup_round_trip_checker()

        # simple DDL only requires a single round trip
        with self.connection.cursor() as cursor:
            cursor.execute("truncate table TestTempTable")
            self.assertRoundTrips(1)

        # array execution only requires a single round trip
        num_rows = 590
        with self.connection.cursor() as cursor:
            sql = "insert into TestTempTable (IntCol) values (:1)"
            data = [(n + 1,) for n in range(num_rows)]
            cursor.executemany(sql, data)
            self.assertRoundTrips(1)

        # create refcursor and execute stored procedure
        with self.connection.cursor() as cursor:
            refcursor = self.connection.cursor()
            refcursor.prefetchrows = 150
            refcursor.arraysize = 50
            cursor.callproc("myrefcursorproc", [refcursor])
            refcursor.fetchall()
            self.assertRoundTrips(4)

    def test_1267_existing_cursor_prefetchrows(self):
        "1267 - test prefetch rows using existing cursor"
        self.setup_round_trip_checker()

        # Set prefetch rows on an existing cursor
        num_rows = 590
        with self.connection.cursor() as cursor:
            cursor.execute("truncate table TestTempTable")
            sql = "insert into TestTempTable (IntCol) values (:1)"
            data = [(n + 1,) for n in range(num_rows)]
            cursor.executemany(sql, data)
            cursor.prefetchrows = 300
            cursor.arraysize = 300
            cursor.execute("select IntCol from TestTempTable").fetchall()
            self.assertRoundTrips(4)

    def test_1268_bind_names_with_single_line_comments(self):
        "1268 - test bindnames() with single line comments"
        self.cursor.prepare("""--begin :value2 := :a + :b + :c +:a +3; end;
                            begin :value2 := :a + :c +3; end;
                            """)
        self.assertEqual(self.cursor.bindnames(), ["VALUE2", "A", "C"])

    def test_1269_bind_names_with_multi_line_comments(self):
        "1269 - test bindnames() with multi line comments"
        self.cursor.prepare("""/*--select * from :a where :a = 1
                            select * from table_names where :a = 1*/
                            select * from :table_name where :value = 1
                            """)
        self.assertEqual(self.cursor.bindnames(), ["TABLE_NAME", "VALUE"])

    def test_1270_execute_bind_names_with_incorrect_bind(self):
        "1270 - test executing a statement with an incorrect named bind"
        statement = "select * from TestStrings where IntCol = :value"
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute,
                          statement, value2=3)

    def test_1271_execute_with_named_binds(self):
        "1271 - test executing a statement with named binds"
        statement = "select * from TestNumbers where IntCol = :value1 " + \
                    "and LongIntCol = :value2"
        result = self.cursor.execute(statement, value1=1, value2=38)
        self.assertEqual(len(result.fetchall()), 1)

    def test_1272_execute_bind_position_with_incorrect_bind(self):
        "1272 - test executing a statement with an incorrect positional bind"
        statement = "select * from TestNumbers where IntCol = :value " + \
                    "and LongIntCol = :value2"
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute,
                          statement, [3])

    def test_1273_execute_with_positional_binds(self):
        "1273 - test executing a statement with positional binds"
        statement = "select * from TestNumbers where IntCol = :value " + \
                    "and LongIntCol = :value2"
        result = self.cursor.execute(statement, [1,38])
        self.assertEqual(len(result.fetchall()), 1)

    def test_1274_execute_with_rebinding_bind_name(self):
        "1274 - test executing a statement after rebinding a named bind"
        statement = "begin :value := :value2 + 5; end;"
        simple_var = self.cursor.var(oracledb.NUMBER)
        simple_var2 = self.cursor.var(oracledb.NUMBER)
        simple_var2.setvalue(0, 5)
        result = self.cursor.execute(statement, value=simple_var,
                                     value2=simple_var2)
        self.assertEqual(result, None)
        self.assertEqual(simple_var.getvalue(), 10)

        simple_var = self.cursor.var(oracledb.NATIVE_FLOAT)
        simple_var2 = self.cursor.var(oracledb.NATIVE_FLOAT)
        simple_var2.setvalue(0, 10)
        result = self.cursor.execute(statement, value=simple_var,
                                     value2=simple_var2)
        self.assertEqual(result, None)
        self.assertEqual(simple_var.getvalue(), 15)

    def test_1275_bind_names_with_strings(self):
        "1275 - test bindnames() with strings in the statement"
        statement = """
                    begin
                    :value := to_date('20021231 12:31:00',
                    'YYYYMMDD HH24:MI:SS');
                    end;"""
        self.cursor.prepare(statement)
        self.assertEqual(self.cursor.bindnames(), ["VALUE"])

    def test_1276_bind_by_name_with_duplicates(self):
        "1276 - test executing a PL/SQL statement with duplicate binds"
        statement = "begin :value := :value + 5; end;"
        simple_var = self.cursor.var(oracledb.NUMBER)
        simple_var.setvalue(0, 5)
        result = self.cursor.execute(statement, value=simple_var)
        self.assertEqual(result, None)
        self.assertEqual(simple_var.getvalue(), 10)

    def test_1277_positional_bind_with_duplicates(self):
        "1277 - test executing a PL/SQL statement with duplicate binds"
        statement = "begin :value := :value + 5; end;"
        simple_var = self.cursor.var(oracledb.NUMBER)
        simple_var.setvalue(0, 5)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), 10)

    def test_1278_execute_with_incorrect_bind_values(self):
        "1278 - test executing a statement with an incorrect number of binds"
        statement = "begin :value := :value2 + 5; end;"
        var = self.cursor.var(oracledb.NUMBER)
        var.setvalue(0, 5)
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute,
                          statement)
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute,
                          statement, value=var)
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute,
                          statement, value=var, value2=var, value3=var)

    def test_1279_change_in_size_on_successive_bind(self):
        "1279 - change in size on subsequent binds does not use optimised path"
        self.cursor.execute("truncate table TestTempTable")
        data = [
            (1, "Test String #1"),
            (2, "ABC" * 100)
        ]
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        for row in data:
            self.cursor.execute(sql, row)
        self.connection.commit()
        self.cursor.execute("select IntCol, StringCol from TestTempTable")
        self.assertEqual(self.cursor.fetchall(), data)

    def test_1280_change_in_type_on_successive_bind(self):
        "1280 - change in type on subsequent binds cannot use optimised path"
        sql = "select :1 from dual"
        self.cursor.execute(sql, ('W',))
        row, = self.cursor.fetchone()
        self.assertEqual(row, 'W')
        self.cursor.execute(sql, ('S',))
        row, = self.cursor.fetchone()
        self.assertEqual(row, 'S')
        self.cursor.execute(sql, (7,))
        row, = self.cursor.fetchone()
        self.assertEqual(row, '7')

    def test_1281_dml_can_use_optimised_path(self):
        "1281 - test that dml can use optimised path"
        data_to_insert = [
            (1, "Test String #1"),
            (2, "Test String #2"),
            (3, "Test String #3")
        ]
        self.cursor.execute("truncate table TestTempTable")
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        for row in data_to_insert:
            with self.connection.cursor() as cursor:
                cursor.execute(sql, row)
        self.connection.commit()
        self.cursor.execute("""
                select IntCol, StringCol
                from TestTempTable
                order by IntCol""")
        self.assertEqual(self.cursor.fetchall(), data_to_insert)

    def test_1282_parse_plsql(self):
        "1282 - test parsing plsql statements"
        sql = "begin :value := 5; end;"
        self.cursor.parse(sql)
        self.assertEqual(self.cursor.statement, sql)
        self.assertEqual(self.cursor.description, None)

    def test_1283_parse_ddl(self):
        "1283 - test parsing ddl statements"
        sql = "truncate table TestTempTable"
        self.cursor.parse(sql)
        self.assertEqual(self.cursor.statement, sql)
        self.assertEqual(self.cursor.description, None)

    def test_1284_parse_dml(self):
        "1284 - test parsing dml statements"
        sql = "insert into TestTempTable (IntCol) values (1)"
        self.cursor.parse(sql)
        self.assertEqual(self.cursor.statement, sql)
        self.assertEqual(self.cursor.description, None)

    def test_1285_executemany_with_plsql_binds(self):
        "1285 - test executing plsql statements multiple times (with binds)"
        var = self.cursor.var(int, arraysize=5)
        self.cursor.setinputsizes(var)
        data = [[25], [30], [None], [35], [None]]
        exepected_data = [25, 30, None, 35, None]
        self.cursor.executemany("declare t number; begin t := :1; end;", data)
        self.assertEqual(var.values, exepected_data)

    def test_1286_encodingErrors_deprecation(self):
        "1286 - test to verify encodingErrors is deprecated"
        errors = 'strict'
        self.assertRaises(oracledb.ProgrammingError, self.cursor.var,
                          oracledb.NUMBER, encoding_errors=errors,
                          encodingErrors=errors)

    def test_1287_keywordParameters_deprecation(self):
        "1287 - test to verify keywordParameters is deprecated"
        out_value = self.cursor.var(oracledb.NUMBER)
        kwargs = dict(a_OutValue=out_value)
        self.assertRaises(oracledb.ProgrammingError, self.cursor.callproc,
                          "proc_Test", ("hi", 5), kwargs,
                          keywordParameters=kwargs)
        extra_amount = self.cursor.var(oracledb.NUMBER)
        extra_amount.setvalue(0, 5)
        kwargs = dict(a_ExtraAmount=extra_amount, a_String="hi")
        self.assertRaises(oracledb.ProgrammingError, self.cursor.callfunc,
                          "func_Test", oracledb.NUMBER, [], kwargs,
                          keywordParameters=kwargs)

if __name__ == "__main__":
    test_env.run_test_cases()
