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
import datetime

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

    def testBindNullIn(self):
        "test binding a null value (IN)"
        var = self.cursor.var(cx_Oracle.OBJECT, typename = "UDT_OBJECT")
        result = self.cursor.callfunc("pkg_TestBindObject.GetStringRep", str,
                (var,))
        self.assertEqual(result, "null")

    def testBindObjectIn(self):
        "test binding an object (IN)"
        typeObj = self.connection.gettype("UDT_OBJECT")
        obj = typeObj.newobject()
        obj.NUMBERVALUE = 13
        obj.STRINGVALUE = "Test String"
        result = self.cursor.callfunc("pkg_TestBindObject.GetStringRep", str,
                (obj,))
        self.assertEqual(result,
                "udt_Object(13, 'Test String', null, null, null, null, null)")
        obj.NUMBERVALUE = None
        obj.STRINGVALUE = "Test With Dates"
        obj.DATEVALUE = datetime.datetime(2016, 2, 10)
        obj.TIMESTAMPVALUE = datetime.datetime(2016, 2, 10, 14, 13, 50)
        result = self.cursor.callfunc("pkg_TestBindObject.GetStringRep", str,
                (obj,))
        self.assertEqual(result,
                "udt_Object(null, 'Test With Dates', null, " \
                "to_date('2016-02-10', 'YYYY-MM-DD'), " \
                "to_timestamp('2016-02-10 14:13:50', " \
                        "'YYYY-MM-DD HH24:MI:SS'), " \
                "null, null)")
        obj.DATEVALUE = None
        obj.TIMESTAMPVALUE = None
        subTypeObj = self.connection.gettype("UDT_SUBOBJECT")
        subObj = subTypeObj.newobject()
        subObj.SUBNUMBERVALUE = 15
        subObj.SUBSTRINGVALUE = "Sub String"
        obj.SUBOBJECTVALUE = subObj
        result = self.cursor.callfunc("pkg_TestBindObject.GetStringRep", str,
                (obj,))
        self.assertEqual(result,
                "udt_Object(null, 'Test With Dates', null, null, null, " \
                "udt_SubObject(15, 'Sub String'), null)")

    def testCopyObject(self):
        "test copying an object"
        typeObj = self.connection.gettype("UDT_OBJECT")
        obj = typeObj()
        obj.NUMBERVALUE = 5124
        obj.STRINGVALUE = "A test string"
        obj.DATEVALUE = datetime.datetime(2016, 2, 24)
        obj.TIMESTAMPVALUE = datetime.datetime(2016, 2, 24, 13, 39, 10)
        copiedObj = obj.copy()
        self.assertEqual(obj.NUMBERVALUE, copiedObj.NUMBERVALUE)
        self.assertEqual(obj.STRINGVALUE, copiedObj.STRINGVALUE)
        self.assertEqual(obj.DATEVALUE, copiedObj.DATEVALUE)
        self.assertEqual(obj.TIMESTAMPVALUE, copiedObj.TIMESTAMPVALUE)

    def testFetchData(self):
        "test fetching objects"
        self.cursor.execute("""
                select
                  IntCol,
                  ObjectCol,
                  ArrayCol
                from TestObjects
                order by IntCol""")
        self.assertEqual(self.cursor.description,
                [ ('INTCOL', cx_Oracle.NUMBER, 10, None, 9, 0, 0),
                  ('OBJECTCOL', cx_Oracle.OBJECT, None, None, None, None, 1),
                  ('ARRAYCOL', cx_Oracle.OBJECT, None, None, None, None, 1) ])
        self.__TestData(1, (1, 'First row', 'First     ',
                cx_Oracle.Timestamp(2007, 3, 6, 0, 0, 0),
                cx_Oracle.Timestamp(2008, 9, 12, 16, 40),
                (11, 'Sub object 1'),
                [(5, 'first element'), (6, 'second element')]),
                [5, 10, None, 20])
        self.__TestData(2, None, [3, None, 9, 12, 15])
        self.__TestData(3, (3, 'Third row', 'Third     ',
                cx_Oracle.Timestamp(2007, 6, 21, 0, 0, 0),
                cx_Oracle.Timestamp(2007, 12, 13, 7, 30, 45),
                (13, 'Sub object 3'),
                [(10, 'element #1'), (20, 'element #2'),
                 (30, 'element #3'), (40, 'element #4')]), None)

    def testGetObjectType(self):
        "test getting object type"
        typeObj = self.connection.gettype("UDT_OBJECT")
        self.assertEqual(typeObj.iscollection, False)
        self.assertEqual(typeObj.schema, self.connection.username.upper())
        self.assertEqual(typeObj.name, "UDT_OBJECT")
        expectedAttributeNames = ["NUMBERVALUE", "STRINGVALUE",
                "FIXEDCHARVALUE", "DATEVALUE", "TIMESTAMPVALUE",
                "SUBOBJECTVALUE", "SUBOBJECTARRAY"]
        actualAttributeNames = [a.name for a in typeObj.attributes]
        self.assertEqual(actualAttributeNames, expectedAttributeNames)
        typeObj = self.connection.gettype("UDT_OBJECTARRAY")
        self.assertEqual(typeObj.iscollection, True)
        self.assertEqual(typeObj.attributes, [])

    def testObjectType(self):
        "test object type data"
        self.cursor.execute("""
                select ObjectCol
                from TestObjects
                where ObjectCol is not null
                  and rownum <= 1""")
        objValue, = self.cursor.fetchone()
        self.assertEqual(objValue.type.schema,
                self.connection.username.upper())
        self.assertEqual(objValue.type.name, "UDT_OBJECT")
        self.assertEqual(objValue.type.attributes[0].name, "NUMBERVALUE")

