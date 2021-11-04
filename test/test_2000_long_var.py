#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
2000 - Module for testing long and long raw variables
"""

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def __perform_test(self, typ):
        name_part = "Long" if typ is oracledb.DB_TYPE_LONG else "LongRaw"

        self.cursor.execute(f"truncate table Test{name_part}s")
        self.cursor.setinputsizes(long_string=typ)
        long_string = ""
        for i in range(1, 11):
            char = chr(ord('A') + i - 1)
            long_string += char * 25000
            if i % 3 == 1:
                bind_value = None
            else:
                if typ is oracledb.DB_TYPE_LONG_RAW:
                    bind_value = long_string.encode()
                else:
                    bind_value = long_string
            self.cursor.execute(f"""
                    insert into Test{name_part}s (
                      IntCol,
                      {name_part}Col
                    ) values (
                      :integer_value,
                      :long_string
                    )""",
                    integer_value=i,
                    long_string=bind_value)
        self.connection.commit()
        self.cursor.execute("""
                select *
                from Test%ss
                order by IntCol""" % name_part)
        long_string = ""
        for integer_value, fetched_value in self.cursor:
            char = chr(ord('A') + integer_value - 1)
            long_string += char * 25000
            if integer_value % 3 == 1:
                expected_value = None
            else:
                if typ is oracledb.DB_TYPE_LONG_RAW:
                    expected_value = long_string.encode()
                else:
                    expected_value = long_string
            if fetched_value is not None:
                self.assertEqual(len(fetched_value), integer_value * 25000)
            self.assertEqual(fetched_value, expected_value)

    def test_2000_longs(self):
        "2000 - test binding and fetching long data"
        self.__perform_test(oracledb.DB_TYPE_LONG)

    def test_2001_long_with_execute_many(self):
        "2001 - test binding long data with executemany()"
        data = []
        self.cursor.execute("truncate table TestLongs")
        for i in range(5):
            char = chr(ord('A') + i)
            long_str = char * (32768 * (i + 1))
            data.append((i + 1, long_str))
        self.cursor.executemany("insert into TestLongs values (:1, :2)", data)
        self.connection.commit()
        self.cursor.execute("select * from TestLongs order by IntCol")
        fetched_data = self.cursor.fetchall()
        self.assertEqual(fetched_data, data)

    def test_2002_long_raws(self):
        "2002 - test binding and fetching long raw data"
        self.__perform_test(oracledb.DB_TYPE_LONG_RAW)

    def test_2003_long_cursor_description(self):
        "2003 - test cursor description is accurate for longs"
        self.cursor.execute("select * from TestLongs")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('LONGCOL', oracledb.DB_TYPE_LONG, None, None, None, None, True)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_2004_long_raw_cursor_description(self):
        "2004 - test cursor description is accurate for long raws"
        self.cursor.execute("select * from TestLongRaws")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('LONGRAWCOL', oracledb.DB_TYPE_LONG_RAW, None, None, None, None,
                    True)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_2005_array_size_too_large(self):
        "2005 - test array size too large generates an exception"
        self.cursor.arraysize = 268435456
        self.assertRaises(oracledb.DatabaseError, self.cursor.execute,
                          "select * from TestLongRaws")

if __name__ == "__main__":
    test_env.run_test_cases()
