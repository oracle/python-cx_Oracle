#------------------------------------------------------------------------------
# Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
3200 - Module for testing features introduced in 12.1
"""

import datetime
import unittest

import cx_Oracle as oracledb
import test_env

@unittest.skipUnless(test_env.get_client_version() >= (12, 1),
                     "unsupported client")
class TestCase(test_env.BaseTestCase):

    def test_3200_array_dml_row_counts_off(self):
        "3200 - test executing with arraydmlrowcounts mode disabled"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [(1, "First"), (2, "Second")]
        sql = "insert into TestArrayDML (IntCol,StringCol) values (:1,:2)"
        self.cursor.executemany(sql, rows, arraydmlrowcounts=False)
        self.assertRaises(oracledb.DatabaseError,
                          self.cursor.getarraydmlrowcounts)
        rows = [(3, "Third"), (4, "Fourth")]
        self.cursor.executemany(sql, rows)
        self.assertRaises(oracledb.DatabaseError,
                          self.cursor.getarraydmlrowcounts)

    def test_3201_array_dml_row_counts_on(self):
        "3201 - test executing with arraydmlrowcounts mode enabled"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First", 100),
            (2, "Second", 200),
            (3, "Third", 300),
            (4, "Fourth", 300),
            (5, "Fifth", 300)
        ]
        sql = "insert into TestArrayDML (IntCol,StringCol,IntCol2) " \
                "values (:1,:2,:3)"
        self.cursor.executemany(sql, rows, arraydmlrowcounts=True)
        self.connection.commit()
        self.assertEqual(self.cursor.getarraydmlrowcounts(), [1, 1, 1, 1, 1])
        self.cursor.execute("select count(*) from TestArrayDML")
        count, = self.cursor.fetchone()
        self.assertEqual(count, len(rows))

    def test_3202_bind_plsql_boolean_collection_in(self):
        "3202 - test binding a boolean collection (in)"
        type_obj = self.connection.gettype("PKG_TESTBOOLEANS.UDT_BOOLEANLIST")
        obj = type_obj.newobject()
        obj.setelement(1, True)
        obj.extend([True, False, True, True, False, True])
        result = self.cursor.callfunc("pkg_TestBooleans.TestInArrays", int,
                                      (obj,))
        self.assertEqual(result, 5)

    def test_3203_bind_plsql_boolean_collection_out(self):
        "3203 - test binding a boolean collection (out)"
        type_obj = self.connection.gettype("PKG_TESTBOOLEANS.UDT_BOOLEANLIST")
        obj = type_obj.newobject()
        self.cursor.callproc("pkg_TestBooleans.TestOutArrays", (6, obj))
        self.assertEqual(obj.aslist(), [True, False, True, False, True, False])

    def test_3204_bind_plql_date_collection_in(self):
        "3204 - test binding a PL/SQL date collection (in)"
        type_obj = self.connection.gettype("PKG_TESTDATEARRAYS.UDT_DATELIST")
        obj = type_obj.newobject()
        obj.setelement(1, datetime.datetime(2016, 2, 5))
        obj.append(datetime.datetime(2016, 2, 8, 12, 15, 30))
        obj.append(datetime.datetime(2016, 2, 12, 5, 44, 30))
        result = self.cursor.callfunc("pkg_TestDateArrays.TestInArrays",
                                      oracledb.NUMBER,
                                      (2, datetime.datetime(2016, 2, 1), obj))
        self.assertEqual(result, 24.75)

    def test_3205_bind_plqsl_date_collection_in_out(self):
        "3205 - test binding a PL/SQL date collection (in/out)"
        type_obj = self.connection.gettype("PKG_TESTDATEARRAYS.UDT_DATELIST")
        obj = type_obj.newobject()
        obj.setelement(1, datetime.datetime(2016, 1, 1))
        obj.append(datetime.datetime(2016, 1, 7))
        obj.append(datetime.datetime(2016, 1, 13))
        obj.append(datetime.datetime(2016, 1, 19))
        self.cursor.callproc("pkg_TestDateArrays.TestInOutArrays", (4, obj))
        expected_values = [
            datetime.datetime(2016, 1, 8),
            datetime.datetime(2016, 1, 14),
            datetime.datetime(2016, 1, 20),
            datetime.datetime(2016, 1, 26)
        ]
        self.assertEqual(obj.aslist(), expected_values)

    def test_3206_bind_plsql_date_collection_out(self):
        "3206 - test binding a PL/SQL date collection (out)"
        type_obj = self.connection.gettype("PKG_TESTDATEARRAYS.UDT_DATELIST")
        obj = type_obj.newobject()
        self.cursor.callproc("pkg_TestDateArrays.TestOutArrays", (3, obj))
        expected_values = [
            datetime.datetime(2002, 12, 13, 4, 48),
            datetime.datetime(2002, 12, 14, 9, 36),
            datetime.datetime(2002, 12, 15, 14, 24)
        ]
        self.assertEqual(obj.aslist(), expected_values)

    def test_3207_bind_plsql_number_collection_in(self):
        "3207 - test binding a PL/SQL number collection (in)"
        type_name = "PKG_TESTNUMBERARRAYS.UDT_NUMBERLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        obj.setelement(1, 10)
        obj.extend([20, 30, 40, 50])
        result = self.cursor.callfunc("pkg_TestNumberArrays.TestInArrays", int,
                                      (5, obj))
        self.assertEqual(result, 155)

    def test_3208_bind_plsql_number_collection_in_out(self):
        "3208 - test binding a PL/SQL number collection (in/out)"
        type_name = "PKG_TESTNUMBERARRAYS.UDT_NUMBERLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        obj.setelement(1, 5)
        obj.extend([8, 3, 2])
        self.cursor.callproc("pkg_TestNumberArrays.TestInOutArrays", (4, obj))
        self.assertEqual(obj.aslist(), [50, 80, 30, 20])

    def test_3209_bind_plsql_number_collection_out(self):
        "3209 - test binding a PL/SQL number collection (out)"
        type_name = "PKG_TESTNUMBERARRAYS.UDT_NUMBERLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        self.cursor.callproc("pkg_TestNumberArrays.TestOutArrays", (3, obj))
        self.assertEqual(obj.aslist(), [100, 200, 300])

    def test_3210_bind_plsql_record_array(self):
        "3210 - test binding an array of PL/SQL records (in)"
        rec_type = self.connection.gettype("PKG_TESTRECORDS.UDT_RECORD")
        array_type = self.connection.gettype("PKG_TESTRECORDS.UDT_RECORDARRAY")
        array_obj = array_type.newobject()
        for i in range(3):
            obj = rec_type.newobject()
            obj.NUMBERVALUE = i + 1
            obj.STRINGVALUE = "String in record #%d" % (i + 1)
            obj.DATEVALUE = datetime.datetime(2017, i + 1, 1)
            obj.TIMESTAMPVALUE = datetime.datetime(2017, 1, i + 1)
            obj.BOOLEANVALUE = (i % 2) == 1
            obj.PLSINTEGERVALUE = i * 5
            obj.BINARYINTEGERVALUE = i * 2
            array_obj.append(obj)
        result = self.cursor.callfunc("pkg_TestRecords.TestInArrays", str,
                                      (array_obj,))
        self.assertEqual(result,
                         "udt_Record(1, 'String in record #1', " \
                         "to_date('2017-01-01', 'YYYY-MM-DD'), " \
                         "to_timestamp('2017-01-01 00:00:00', " \
                         "'YYYY-MM-DD HH24:MI:SS'), false, 0, 0); " \
                         "udt_Record(2, 'String in record #2', " \
                         "to_date('2017-02-01', 'YYYY-MM-DD'), " \
                         "to_timestamp('2017-01-02 00:00:00', " \
                         "'YYYY-MM-DD HH24:MI:SS'), true, 5, 2); " \
                         "udt_Record(3, 'String in record #3', " \
                         "to_date('2017-03-01', 'YYYY-MM-DD'), " \
                         "to_timestamp('2017-01-03 00:00:00', " \
                         "'YYYY-MM-DD HH24:MI:SS'), false, 10, 4)")

    def test_3211_bind_plsql_record_in(self):
        "3211 - test binding a PL/SQL record (in)"
        type_obj = self.connection.gettype("PKG_TESTRECORDS.UDT_RECORD")
        obj = type_obj.newobject()
        obj.NUMBERVALUE = 18
        obj.STRINGVALUE = "A string in a record"
        obj.DATEVALUE = datetime.datetime(2016, 2, 15)
        obj.TIMESTAMPVALUE = datetime.datetime(2016, 2, 12, 14, 25, 36)
        obj.BOOLEANVALUE = False
        obj.PLSINTEGERVALUE = 21
        obj.BINARYINTEGERVALUE = 5
        result = self.cursor.callfunc("pkg_TestRecords.GetStringRep", str,
                                      (obj,))
        self.assertEqual(result,
                         "udt_Record(18, 'A string in a record', " \
                         "to_date('2016-02-15', 'YYYY-MM-DD'), " \
                         "to_timestamp('2016-02-12 14:25:36', " \
                         "'YYYY-MM-DD HH24:MI:SS'), false, 21, 5)")

    def test_3212_bind_plsql_record_out(self):
        "3212 - test binding a PL/SQL record (out)"
        type_obj = self.connection.gettype("PKG_TESTRECORDS.UDT_RECORD")
        obj = type_obj.newobject()
        obj.NUMBERVALUE = 5
        obj.STRINGVALUE = "Test value"
        obj.DATEVALUE = datetime.datetime.today()
        obj.TIMESTAMPVALUE = datetime.datetime.today()
        obj.BOOLEANVALUE = False
        obj.PLSINTEGERVALUE = 23
        obj.BINARYINTEGERVALUE = 9
        self.cursor.callproc("pkg_TestRecords.TestOut", (obj,))
        self.assertEqual(obj.NUMBERVALUE, 25)
        self.assertEqual(obj.STRINGVALUE, "String in record")
        self.assertEqual(obj.DATEVALUE, datetime.datetime(2016, 2, 16))
        self.assertEqual(obj.TIMESTAMPVALUE,
                         datetime.datetime(2016, 2, 16, 18, 23, 55))
        self.assertEqual(obj.BOOLEANVALUE, True)
        self.assertEqual(obj.PLSINTEGERVALUE, 45)
        self.assertEqual(obj.BINARYINTEGERVALUE, 10)

    def test_3213_bind_plsql_string_collection_in(self):
        "3213 - test binding a PL/SQL string collection (in)"
        type_name = "PKG_TESTSTRINGARRAYS.UDT_STRINGLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        obj.setelement(1, "First element")
        obj.setelement(2, "Second element")
        obj.setelement(3, "Third element")
        result = self.cursor.callfunc("pkg_TestStringArrays.TestInArrays", int,
                                      (5, obj))
        self.assertEqual(result, 45)

    def test_3214_bind_plsql_string_collection_in_out(self):
        "3214 - test binding a PL/SQL string collection (in/out)"
        type_name = "PKG_TESTSTRINGARRAYS.UDT_STRINGLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        obj.setelement(1, "The first element")
        obj.append("The second element")
        obj.append("The third and final element")
        self.cursor.callproc("pkg_TestStringArrays.TestInOutArrays", (3, obj))
        expected_values = [
            'Converted element # 1 originally had length 17',
            'Converted element # 2 originally had length 18',
            'Converted element # 3 originally had length 27'
        ]
        self.assertEqual(obj.aslist(), expected_values)

    def test_3215_bind_plsql_string_collection_out(self):
        "3215 - test binding a PL/SQL string collection (out)"
        type_name = "PKG_TESTSTRINGARRAYS.UDT_STRINGLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        self.cursor.callproc("pkg_TestStringArrays.TestOutArrays", (4, obj))
        expected_values = [
            'Test out element # 1',
            'Test out element # 2',
            'Test out element # 3',
            'Test out element # 4'
        ]
        self.assertEqual(obj.aslist(), expected_values)

    def test_3216_bind_plsql_string_collection_out_with_holes(self):
        "3216 - test binding a PL/SQL string collection (out with holes)"
        type_name = "PKG_TESTSTRINGARRAYS.UDT_STRINGLIST"
        type_obj = self.connection.gettype(type_name)
        obj = type_obj.newobject()
        self.cursor.callproc("pkg_TestStringArrays.TestIndexBy", (obj,))
        self.assertEqual(obj.first(), -1048576)
        self.assertEqual(obj.last(), 8388608)
        self.assertEqual(obj.next(-576), 284)
        self.assertEqual(obj.prev(284), -576)
        self.assertEqual(obj.size(), 4)
        self.assertEqual(obj.exists(-576), True)
        self.assertEqual(obj.exists(-577), False)
        self.assertEqual(obj.getelement(284), 'Third element')
        expected_list = [
            "First element",
            "Second element",
            "Third element",
            "Fourth element"
        ]
        self.assertEqual(obj.aslist(), expected_list)
        expected_dict = {
            -1048576: 'First element',
            -576: 'Second element',
            284: 'Third element',
            8388608: 'Fourth element'
        }
        self.assertEqual(obj.asdict(), expected_dict)
        obj.delete(-576)
        obj.delete(284)
        expected_list.pop(2)
        expected_list.pop(1)
        self.assertEqual(obj.aslist(), expected_list)
        expected_dict.pop(-576)
        expected_dict.pop(284)
        self.assertEqual(obj.asdict(), expected_dict)

    def test_3217_exception_in_iteration(self):
        "3217 - test executing with arraydmlrowcounts with exception"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First"),
            (2, "Second"),
            (2, "Third"),
            (4, "Fourth")
        ]
        sql = "insert into TestArrayDML (IntCol,StringCol) values (:1,:2)"
        self.assertRaises(oracledb.DatabaseError, self.cursor.executemany,
                          sql, rows, arraydmlrowcounts=True)
        self.assertEqual(self.cursor.getarraydmlrowcounts(), [1, 1])

    def test_3218_executing_delete(self):
        "3218 - test executing delete statement with arraydmlrowcount mode"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First", 100),
            (2, "Second", 200),
            (3, "Third", 300),
            (4, "Fourth", 300),
            (5, "Fifth", 300),
            (6, "Sixth", 400),
            (7, "Seventh", 400),
            (8, "Eighth", 500)
        ]
        sql = "insert into TestArrayDML (IntCol,StringCol,IntCol2) " \
                "values (:1, :2, :3)"
        self.cursor.executemany(sql, rows)
        rows = [(200,), (300,), (400,)]
        statement = "delete from TestArrayDML where IntCol2 = :1"
        self.cursor.executemany(statement, rows, arraydmlrowcounts=True)
        self.assertEqual(self.cursor.getarraydmlrowcounts(), [1, 3, 2])
        self.assertEqual(self.cursor.rowcount, 6)

    def test_3219_executing_update(self):
        "3219 - test executing update statement with arraydmlrowcount mode"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First",100),
            (2, "Second",200),
            (3, "Third",300),
            (4, "Fourth",300),
            (5, "Fifth",300),
            (6, "Sixth",400),
            (7, "Seventh",400),
            (8, "Eighth",500)
        ]
        sql = "insert into TestArrayDML (IntCol,StringCol,IntCol2) " \
                "values (:1, :2, :3)"
        self.cursor.executemany(sql, rows)
        rows = [
            ("One", 100),
            ("Two", 200),
            ("Three", 300),
            ("Four", 400)
        ]
        sql = "update TestArrayDML set StringCol = :1 where IntCol2 = :2"
        self.cursor.executemany(sql, rows, arraydmlrowcounts=True)
        self.assertEqual(self.cursor.getarraydmlrowcounts(), [1, 1, 3, 2])
        self.assertEqual(self.cursor.rowcount, 7)

    def test_3220_implicit_results(self):
        "3220 - test getimplicitresults() returns the correct data"
        self.cursor.execute("""
                declare
                    c1 sys_refcursor;
                    c2 sys_refcursor;
                begin
                    open c1 for
                    select NumberCol
                    from TestNumbers
                    where IntCol between 3 and 5;

                    dbms_sql.return_result(c1);

                    open c2 for
                    select NumberCol
                    from TestNumbers
                    where IntCol between 7 and 10;

                    dbms_sql.return_result(c2);
                end;""")
        results = self.cursor.getimplicitresults()
        self.assertEqual(len(results), 2)
        self.assertEqual([n for n, in results[0]], [3.75, 5, 6.25])
        self.assertEqual([n for n, in results[1]], [8.75, 10, 11.25, 12.5])

    def test_3221_implicit_results_no_statement(self):
        "3221 - test getimplicitresults() without executing a statement"
        self.assertRaises(oracledb.InterfaceError,
                          self.cursor.getimplicitresults)

    def test_3222_insert_with_batch_error(self):
        "3222 - test executing insert with multiple distinct batch errors"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First", 100),
            (2, "Second", 200),
            (2, "Third", 300),
            (4, "Fourth", 400),
            (5, "Fourth", 1000)
        ]
        sql = "insert into TestArrayDML (IntCol, StringCol, IntCol2) " \
                "values (:1, :2, :3)"
        self.cursor.executemany(sql, rows, batcherrors=True,
                                arraydmlrowcounts=True)
        user = test_env.get_main_user()
        expected_errors = [
            ( 4, 1438, "ORA-01438: value larger than specified " \
                       "precision allowed for this column" ),
            ( 2, 1, "ORA-00001: unique constraint " \
                    "(%s.TESTARRAYDML_PK) violated" % user.upper())
        ]
        actual_errors = [(e.offset, e.code, e.message) \
                        for e in self.cursor.getbatcherrors()]
        self.assertEqual(actual_errors, expected_errors)
        self.assertEqual(self.cursor.getarraydmlrowcounts(), [1, 1, 0, 1, 0])

    def test_3223_batch_error_false(self):
        "3223 - test batcherrors mode set to False"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First", 100),
            (2, "Second", 200),
            (2, "Third", 300)
        ]
        sql = "insert into TestArrayDML (IntCol, StringCol, IntCol2) " \
                "values (:1, :2, :3)"
        self.assertRaises(oracledb.IntegrityError, self.cursor.executemany,
                          sql, rows, batcherrors=False)

    def test_3224_update_with_batch_error(self):
        "3224 - test executing in succession with batch error"
        self.cursor.execute("truncate table TestArrayDML")
        rows = [
            (1, "First", 100),
            (2, "Second", 200),
            (3, "Third", 300),
            (4, "Second", 300),
            (5, "Fifth", 300),
            (6, "Sixth", 400),
            (6, "Seventh", 400),
            (8, "Eighth", 100)
        ]
        sql = "insert into TestArrayDML (IntCol, StringCol, IntCol2) " \
                "values (:1, :2, :3)"
        self.cursor.executemany(sql, rows, batcherrors=True)
        user = test_env.get_main_user()
        expected_errors = [
            ( 6, 1, "ORA-00001: unique constraint " \
                    "(%s.TESTARRAYDML_PK) violated" % user.upper())
        ]
        actual_errors = [(e.offset, e.code, e.message) \
                        for e in self.cursor.getbatcherrors()]
        self.assertEqual(actual_errors, expected_errors)
        rows = [
            (101, "First"),
            (201, "Second"),
            (3000, "Third"),
            (900, "Ninth"),
            (301, "Third")
        ]
        sql = "update TestArrayDML set IntCol2 = :1 where StringCol = :2"
        self.cursor.executemany(sql, rows, arraydmlrowcounts=True,
                                batcherrors=True)
        expected_errors = [
            (2, 1438, "ORA-01438: value larger than specified " \
                      "precision allowed for this column")
        ]
        actual_errors = [(e.offset, e.code, e.message) \
                        for e in self.cursor.getbatcherrors()]
        self.assertEqual(actual_errors, expected_errors)
        self.assertEqual(self.cursor.getarraydmlrowcounts(), [1, 2, 0, 0, 1])
        self.assertEqual(self.cursor.rowcount, 4)

    def test_3225_implicit_results(self):
        "3225 - test using implicit cursors to execute new statements"
        cursor = self.connection.cursor()
        cursor.execute("""
                declare
                    c1 sys_refcursor;
                begin
                    open c1 for
                    select NumberCol
                    from TestNumbers
                    where IntCol between 3 and 5;

                    dbms_sql.return_result(c1);
                end;""")
        results = cursor.getimplicitresults()
        self.assertEqual(len(results), 1)
        self.assertEqual([n for n, in results[0]], [3.75, 5, 6.25])
        results[0].execute("select :1 from dual", (7,))
        row, = results[0].fetchone()
        self.assertEqual(row, 7)

if __name__ == "__main__":
    test_env.run_test_cases()
