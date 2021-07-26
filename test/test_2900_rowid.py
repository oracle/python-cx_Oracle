#------------------------------------------------------------------------------
# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
2900 - Module for testing Rowids
"""

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def __test_select_rowids(self, table_name):
        self.cursor.execute("select rowid, IntCol from %s""" % table_name)
        rowid_dict = dict(self.cursor)
        sql = "select IntCol from %s where rowid = :val" % table_name
        for rowid, int_val in rowid_dict.items():
            self.cursor.execute(sql, val = rowid)
            rows = self.cursor.fetchall()
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0][0], int_val)

    def test_2900_select_rowids_regular(self):
        "2900 - test selecting all rowids from a regular table"
        self.__test_select_rowids("TestNumbers")

    def test_2901_select_rowids_index_organised(self):
        "2901 - test selecting all rowids from an index organised table"
        self.__test_select_rowids("TestUniversalRowids")

    def test_2902_insert_invalid_rowid(self):
        "2902 - test inserting an invalid rowid"
        sql = "insert into TestRowids (IntCol, RowidCol) values (1, :rid)"
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute, sql,
                          rid=12345)
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute, sql,
                          rid="523lkhlf")

    def test_2903_insert_rowids(self):
        "2903 - test inserting rowids and verify they are inserted correctly"
        self.cursor.execute("select IntCol, rowid from TestNumbers")
        rows = self.cursor.fetchall()
        self.cursor.execute("truncate table TestRowids")
        self.cursor.executemany("""
                insert into TestRowids
                (IntCol, RowidCol)
                values (:1, :2)""", rows)
        self.connection.commit()
        self.cursor.execute("select IntCol, RowidCol from TestRowids")
        rows = self.cursor.fetchall()
        sql = "select IntCol from TestNumbers where rowid = :val"
        for int_val, rowid in rows:
            self.cursor.execute(sql, val = rowid)
            rows = self.cursor.fetchall()
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0][0], int_val)

if __name__ == "__main__":
    test_env.run_test_cases()
