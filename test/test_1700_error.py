#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1700 - Module for testing error objects
"""

import pickle

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def test_1700_parse_error(self):
        "1700 - test parse error returns offset correctly"
        with self.assertRaises(oracledb.Error) as cm:
            self.cursor.execute("begin t_Missing := 5; end;")
        error_obj, = cm.exception.args
        self.assertEqual(error_obj.offset, 6)

    def test_1701_pickle_error(self):
        "1701 - test picking/unpickling an error object"
        with self.assertRaises(oracledb.Error) as cm:
            self.cursor.execute("""
                    begin
                        raise_application_error(-20101, 'Test!');
                    end;""")
        error_obj, = cm.exception.args
        self.assertEqual(type(error_obj), oracledb._Error)
        self.assertTrue("Test!" in error_obj.message)
        self.assertEqual(error_obj.code, 20101)
        self.assertEqual(error_obj.offset, 0)
        self.assertTrue(isinstance(error_obj.isrecoverable, bool))
        new_error_obj = pickle.loads(pickle.dumps(error_obj))
        self.assertEqual(type(new_error_obj), oracledb._Error)
        self.assertTrue(new_error_obj.message == error_obj.message)
        self.assertTrue(new_error_obj.code == error_obj.code)
        self.assertTrue(new_error_obj.offset == error_obj.offset)
        self.assertTrue(new_error_obj.context == error_obj.context)
        self.assertTrue(new_error_obj.isrecoverable == error_obj.isrecoverable)

if __name__ == "__main__":
    test_env.run_test_cases()
