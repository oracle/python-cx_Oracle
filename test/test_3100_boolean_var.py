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

import cx_Oracle as oracledb
import test_env

@unittest.skipUnless(test_env.get_client_version() >= (12, 1),
                     "unsupported client")
class TestCase(test_env.BaseTestCase):

    def __test_bind_value_as_boolean(self, value):
        expected_result = str(bool(value)).upper()
        var = self.cursor.var(bool)
        var.setvalue(0, value)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                                      (var,))
        self.assertEqual(result, expected_result)

    def test_3100_bind_false(self):
        "3100 - test binding in a False value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                                      (False,))
        self.assertEqual(result, "FALSE")

    def test_3101_bind_float_as_boolean(self):
        "3101 - test binding in a float as a boolean"
        self.__test_bind_value_as_boolean(0.0)
        self.__test_bind_value_as_boolean(1.0)

    def test_3102_bind_integer_as_boolean(self):
        "3102 - test binding in an integer as a boolean"
        self.__test_bind_value_as_boolean(0)
        self.__test_bind_value_as_boolean(1)

    def test_3103_bind_null(self):
        "3103 - test binding in a null value"
        self.cursor.setinputsizes(None, bool)
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                                      (None,))
        self.assertEqual(result, "NULL")

    def test_3104_bind_out_false(self):
        "3104 - test binding out a boolean value (False)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10",
                                      oracledb.DB_TYPE_BOOLEAN, (15,))
        self.assertEqual(result, False)

    def test_3105_bind_out_true(self):
        "3105 - test binding out a boolean value (True)"
        result = self.cursor.callfunc("pkg_TestBooleans.IsLessThan10", bool,
                                      (5,))
        self.assertEqual(result, True)

    def test_3106_bind_string_as_boolean(self):
        "3106 - test binding in a string as a boolean"
        self.__test_bind_value_as_boolean("")
        self.__test_bind_value_as_boolean("0")

    def test_3107_bind_true(self):
        "3107 - test binding in a True value"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                                      (True,))
        self.assertEqual(result, "TRUE")

if __name__ == "__main__":
    test_env.run_test_cases()
