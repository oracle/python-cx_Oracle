"""Module for testing cursor variables."""

import struct
import sys

class TestCursorVar(BaseTestCase):

    def testBindCursor(self):
        "test binding in a cursor"
        cursor = self.connection.cursor()
        self.assertEqual(cursor.description, None)
        self.cursor.execute(u"""
                begin
                  open :cursor for select 'X' StringValue from dual;
                end;""",
                cursor = cursor)
        self.assertEqual(cursor.description,
                [ ('STRINGVALUE', cx_Oracle.FIXED_CHAR, 1, 1, 0, 0, 1) ])
        self.assertEqual(cursor.fetchall(), [('X',)])

    def testBindCursorInPackage(self):
        "test binding in a cursor from a package"
        cursor = self.connection.cursor()
        self.assertEqual(cursor.description, None)
        self.cursor.callproc(u"pkg_TestOutCursors.TestOutCursor", (2, cursor))
        self.assertEqual(cursor.description,
                [ ('INTCOL', cx_Oracle.NUMBER, 10, 22, 9, 0, 0),
                  ('STRINGCOL', cx_Oracle.STRING, 20, 20, 0, 0, 0) ])
        self.assertEqual(cursor.fetchall(),
                [ (1, 'String 1'), (2, 'String 2') ])

    def testFetchCursor(self):
        "test fetching a cursor"
        self.cursor.execute(u"""
                select
                  IntCol,
                  cursor(select IntCol + 1 from dual) CursorValue
                from TestNumbers
                order by IntCol""")
        size = struct.calcsize('P')
        self.assertEqual(self.cursor.description,
                [ (u'INTCOL', cx_Oracle.NUMBER, 10, 22, 9, 0, 0),
                  (u'CURSORVALUE', cx_Oracle.CURSOR, -1, size, 0, 0, 1) ])
        for i in range(1, 11):
            number, cursor = self.cursor.fetchone()
            self.assertEqual(number, i)
            self.assertEqual(cursor.fetchall(), [(i + 1,)])

