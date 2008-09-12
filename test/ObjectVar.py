"""Module for testing object variables."""

import sys

class TestObjectVar(BaseTestCase):

    def __TestData(self, expectedIntValue, expectedObjectValue,
            expectedArrayValue):
        intValue, objectValue, arrayValue = self.cursor.fetchone()
        if objectValue is not None:
            attributeValues = []
            for attribute in objectValue.type.attributes:
                value = getattr(objectValue, attribute.name)
                attributeValues.append(value)
            objectValue = tuple(attributeValues)
        self.failUnlessEqual(intValue, expectedIntValue)
        self.failUnlessEqual(objectValue, expectedObjectValue)
        self.failUnlessEqual(arrayValue, expectedArrayValue)

    def testFetchData(self):
        "test fetching objects"
        self.cursor.execute("""
                select
                  IntCol,
                  ObjectCol,
                  ArrayCol
                from TestObjects
                order by IntCol""")
        self.failUnlessEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.NUMBER, 10, 22, 9, 0, 0),
                  ('OBJECTCOL', cx_Oracle.OBJECT, -1, 2000, 0, 0, 1),
                  ('ARRAYCOL', cx_Oracle.OBJECT, -1, 2000, 0, 0, 1) ])
        self.__TestData(1, (1, 'First row', 'First     ',
                cx_Oracle.Timestamp(2007, 3, 6, 0, 0, 0),
                cx_Oracle.Timestamp(2008, 9, 12, 16, 40)), [5, 10, None, 20])
        self.__TestData(2, None, [3, None, 9, 12, 15])
        self.__TestData(3, (3, 'Third row', 'Third     ',
                cx_Oracle.Timestamp(2007, 6, 21, 0, 0, 0),
                cx_Oracle.Timestamp(2007, 12, 13, 7, 30, 45)), None)

