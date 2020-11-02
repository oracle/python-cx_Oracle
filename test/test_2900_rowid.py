#------------------------------------------------------------------------------
# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
2900 - Module for testing Rowids
"""

import TestEnv

import cx_Oracle

class TestCase(TestEnv.BaseTestCase):

    def __TestSelectRowids(self, tableName):
        self.cursor.execute("select rowid, IntCol from %s""" % tableName)
        rowidDict = dict(self.cursor)
        sql = "select IntCol from %s where rowid = :val" % tableName
        for rowid, intVal in rowidDict.items():
            self.cursor.execute(sql, val = rowid)
            rows = self.cursor.fetchall()
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0][0], intVal)

    def test_2900_SelectRowidsRegular(self):
        "2900 - test selecting all rowids from a regular table"
        self.__TestSelectRowids("TestNumbers")

    def test_2901_SelectRowidsIndexOrganised(self):
        "2901 - test selecting all rowids from an index organised table"
        self.__TestSelectRowids("TestUniversalRowids")

    def test_2902_InsertInvalidRowid(self):
        "2902 - test inserting an invalid rowid"
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.execute,
                "insert into TestRowids (IntCol, RowidCol) values (1, :rid)",
                rid = 12345)
        self.assertRaises(cx_Oracle.DatabaseError, self.cursor.execute,
                "insert into TestRowids (IntCol, RowidCol) values (1, :rid)",
                rid = "523lkhlf")

    def test_2903_InsertRowids(self):
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
        for intVal, rowid in rows:
            self.cursor.execute(sql, val = rowid)
            rows = self.cursor.fetchall()
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0][0], intVal)

if __name__ == "__main__":
    TestEnv.RunTestCases()
