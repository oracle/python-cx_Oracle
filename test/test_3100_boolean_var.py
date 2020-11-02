#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
3100 - Module for testing boolean variables
"""

import unittest
import TestEnv

import cx_Oracle

@unittest.skipUnless(TestEnv.GetClientVersion() >= (12, 1),
                     "unsupported client")
class TestCase(TestEnv.BaseTestCase):

    def __testBindValueAsBoolean(self, value):
        expectedResult = str(bool(value)).upper()
        var = self.cursor.var(bool)
        var.setvalue(0, value)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (var,))
        self.assertEqual(result, expectedResult)

    def test_3100_BindFalse(self):
        "3100 - test binding in a False value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (False,))
        self.assertEqual(result, "FALSE")

    def test_3101_BindFloatAsBoolean(self):
        "3101 - test binding in a float as a boolean"
        self.__testBindValueAsBoolean(0.0)
        self.__testBindValueAsBoolean(1.0)

    def test_3102_BindIntegerAsBoolean(self):
        "3102 - test binding in an integer as a boolean"
        self.__testBindValueAsBoolean(0)
        self.__testBindValueAsBoolean(1)

    def test_3103_BindNull(self):
        "3103 - test binding in a null value"
        self.cursor.setinputsizes(None, bool)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (None,))
        self.assertEqual(result, "NULL")

    def test_3104_BindOutFalse(self):
        "3104 - test binding out a boolean value (False)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10",
                cx_Oracle.DB_TYPE_BOOLEAN, (15,))
        self.assertEqual(result, False)

    def test_3105_BindOutTrue(self):
        "3105 - test binding out a boolean value (True)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10", bool,
                (5,))
        self.assertEqual(result, True)

    def test_3106_BindStringAsBoolean(self):
        "3106 - test binding in a string as a boolean"
        self.__testBindValueAsBoolean("")
        self.__testBindValueAsBoolean("0")

    def test_3107_BindTrue(self):
        "3107 - test binding in a True value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (True,))
        self.assertEqual(result, "TRUE")

if __name__ == "__main__":
    TestEnv.RunTestCases()
