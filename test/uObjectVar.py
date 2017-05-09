#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing object variables."""

import cx_Oracle

class TestObjectVar(BaseTestCase):

    def __GetObjectAsTuple(self, obj):
        attributeValues = []
        for attribute in obj.type.attributes:
            value = getattr(obj, attribute.name)
            if isinstance(value, cx_Oracle.Object):
                if not value.type.iscollection:
                    value = self.__GetObjectAsTuple(value)
                else:
                    subValue = []
                    for v in value.aslist():
                        if isinstance(v, cx_Oracle.Object):
                            v = self.__GetObjectAsTuple(v)
                        subValue.append(v)
                    value = subValue
            elif isinstance(value, cx_Oracle.LOB):
                value = value.read()
            attributeValues.append(value)
        return tuple(attributeValues)

    def __TestData(self, expectedIntValue, expectedObjectValue,
            expectedArrayValue):
        intValue, objectValue, arrayValue = self.cursor.fetchone()
        if objectValue is not None:
            objectValue = self.__GetObjectAsTuple(objectValue)
        if arrayValue is not None:
            arrayValue = arrayValue.aslist()
        self.assertEqual(intValue, expectedIntValue)
        self.assertEqual(objectValue, expectedObjectValue)
        self.assertEqual(arrayValue, expectedArrayValue)

    def testFetchData(self):
        "test fetching objects"
        self.cursor.execute(u"""
                select
                  IntCol,
                  ObjectCol,
                  ArrayCol
                from TestObjects
                order by IntCol""")
        self.assertEqual(self.cursor.description,
                [ (u'INTCOL', cx_Oracle.NUMBER, 10, None, 9, 0, 0),
                  (u'OBJECTCOL', cx_Oracle.OBJECT, None, None, None, None, 1),
                  (u'ARRAYCOL', cx_Oracle.OBJECT, None, None, None, None, 1) ])
        self.__TestData(1, (1, u'First row', u'First     ', u'N First Row',
                u'N First   ', 2, 5, 12.5, 25.25, 50.125,
                cx_Oracle.Timestamp(2007, 3, 6, 0, 0, 0),
                cx_Oracle.Timestamp(2008, 9, 12, 16, 40),
                u'Short CLOB value', u'Short NCLOB Value', 'Short BLOB value',
                (11, 'Sub object 1'),
                [(5, 'first element'), (6, 'second element')]),
                [5, 10, None, 20])
        self.__TestData(2, None, [3, None, 9, 12, 15])
        self.__TestData(3, (3, u'Third row', u'Third     ', u'N Third Row',
                u'N Third   ', 4, 10, 43.25, 86.5, 192.125,
                cx_Oracle.Timestamp(2007, 6, 21, 0, 0, 0),
                cx_Oracle.Timestamp(2007, 12, 13, 7, 30, 45),
                u'Another short CLOB value', u'Another short NCLOB Value',
                'Yet another short BLOB value',
                (13, 'Sub object 3'),
                [(10, 'element #1'), (20, 'element #2'),
                 (30, 'element #3'), (40, 'element #4')]), None)

    def testObjectType(self):
        "test object type data"
        self.cursor.execute(u"""
                select ObjectCol
                from TestObjects
                where ObjectCol is not null
                  and rownum <= 1""")
        objValue, = self.cursor.fetchone()
        self.assertEqual(objValue.type.schema,
                self.connection.username.upper())
        self.assertEqual(objValue.type.name, u"UDT_OBJECT")
        self.assertEqual(objValue.type.attributes[0].name, "NUMBERVALUE")

