#------------------------------------------------------------------------------
# Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing boolean variables."""

import TestEnv

import cx_Oracle

class TestCase(TestEnv.BaseTestCase):

    def __testBindValueAsBoolean(self, value):
        expectedResult = str(bool(value)).upper()
        var = self.cursor.var(bool)
        var.setvalue(0, value)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (var,))
        self.assertEqual(result, expectedResult)

    def testBindFalse(self):
        "test binding in a False value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (False,))
        self.assertEqual(result, "FALSE")

    def testBindFloatAsBoolean(self):
        "test binding in a float as a boolean"
        self.__testBindValueAsBoolean(0.0)
        self.__testBindValueAsBoolean(1.0)

    def testBindIntegerAsBoolean(self):
        "test binding in an integer as a boolean"
        self.__testBindValueAsBoolean(0)
        self.__testBindValueAsBoolean(1)

    def testBindNull(self):
        "test binding in a null value"
        self.cursor.setinputsizes(None, bool)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (None,))
        self.assertEqual(result, "NULL")

    def testBindOutFalse(self):
        "test binding out a boolean value (False)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10",
                cx_Oracle.DB_TYPE_BOOLEAN, (15,))
        self.assertEqual(result, False)

    def testBindOutTrue(self):
        "test binding out a boolean value (True)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10", bool,
                (5,))
        self.assertEqual(result, True)

    def testBindStringAsBoolean(self):
        "test binding in a string as a boolean"
        self.__testBindValueAsBoolean("")
        self.__testBindValueAsBoolean("0")

    def testBindTrue(self):
        "test binding in a True value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (True,))
        self.assertEqual(result, "TRUE")

if __name__ == "__main__":
    TestEnv.RunTestCases()
