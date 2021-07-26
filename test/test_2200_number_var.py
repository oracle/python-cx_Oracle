#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
2200 - Module for testing number variables
"""

import decimal
import sys

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def output_type_handler_binary_int(self, cursor, name, default_type, size,
                                       precision, scale):
        return cursor.var(oracledb.DB_TYPE_BINARY_INTEGER,
                          arraysize=cursor.arraysize)

    def output_type_handler_decimal(self, cursor, name, default_type, size,
                                    precision, scale):
        if default_type == oracledb.NUMBER:
            return cursor.var(str, 255, outconverter=decimal.Decimal,
                              arraysize=cursor.arraysize)

    def setUp(self):
        super().setUp()
        self.raw_data = []
        self.data_by_key = {}
        for i in range(1, 11):
            number_col = i + i * 0.25
            float_col = i + i * 0.75
            unconstrained_col = i ** 3 + i * 0.5
            if i % 2:
                nullable_col = 143 ** i
            else:
                nullable_col = None
            data_tuple = (i, 38 ** i, number_col, float_col,
                          unconstrained_col, nullable_col)
            self.raw_data.append(data_tuple)
            self.data_by_key[i] = data_tuple

    def test_2200_bind_boolean(self):
        "2200 - test binding in a boolean"
        result = self.cursor.callfunc("pkg_TestBooleans.GetStringRep", str,
                (True,))
        self.assertEqual(result, "TRUE")

    def test_2201_bind_boolean_as_number(self):
        "2201 - test binding in a boolean as a number"
        var = self.cursor.var(oracledb.NUMBER)
        var.setvalue(0, True)
        self.cursor.execute("select :1 from dual", [var])
        result, = self.cursor.fetchone()
        self.assertEqual(result, 1)
        var.setvalue(0, False)
        self.cursor.execute("select :1 from dual", [var])
        result, = self.cursor.fetchone()
        self.assertEqual(result, 0)

    def test_2202_bind_decimal(self):
        "2202 - test binding in a decimal.Decimal"
        self.cursor.execute("""
                select * from TestNumbers
                where NumberCol - :value1 - :value2 = trunc(NumberCol)""",
                value1=decimal.Decimal("0.20"),
                value2=decimal.Decimal("0.05"))
        expected_data = [self.data_by_key[1], self.data_by_key[5],
                         self.data_by_key[9]]
        self.assertEqual(self.cursor.fetchall(), expected_data)

    def test_2203_bind_float(self):
        "2203 - test binding in a float"
        self.cursor.execute("""
                select * from TestNumbers
                where NumberCol - :value = trunc(NumberCol)""",
                value=0.25)
        expected_data = [self.data_by_key[1], self.data_by_key[5],
                         self.data_by_key[9]]
        self.assertEqual(self.cursor.fetchall(), expected_data)

    def test_2204_bind_integer(self):
        "2204 - test binding in an integer"
        self.cursor.execute("""
                select * from TestNumbers
                where IntCol = :value""",
                value = 2)
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[2]])

    def test_2205_bind_large_long_as_oracle_number(self):
        "2205 - test binding in a large long integer as Oracle number"
        in_val = 6088343244
        value_var = self.cursor.var(oracledb.NUMBER)
        value_var.setvalue(0, in_val)
        self.cursor.execute("""
                begin
                  :value := :value + 5;
                end;""",
                value=value_var)
        value = value_var.getvalue()
        self.assertEqual(value, in_val + 5)

    def test_2206_bind_large_long_as_integer(self):
        "2206 - test binding in a large long integer as Python integer"
        long_value = -9999999999999999999
        self.cursor.execute("select :value from dual", value=long_value)
        result, = self.cursor.fetchone()
        self.assertEqual(result, long_value)

    def test_2207_bind_integer_after_string(self):
        "2207 - test binding in an integer after setting input sizes to string"
        self.cursor.setinputsizes(value=15)
        self.cursor.execute("""
                select * from TestNumbers
                where IntCol = :value""",
                value=3)
        self.assertEqual(self.cursor.fetchall(), [self.data_by_key[3]])

    def test_2208_bind_decimal_after_number(self):
        "2208 - test binding in a decimal after setting input sizes to number"
        cursor = self.connection.cursor()
        value = decimal.Decimal("319438950232418390.273596")
        cursor.setinputsizes(value=oracledb.NUMBER)
        cursor.outputtypehandler = self.output_type_handler_decimal
        cursor.execute("select :value from dual", value=value)
        out_value, = cursor.fetchone()
        self.assertEqual(out_value, value)

    def test_2209_bind_null(self):
        "2209 - test binding in a null"
        self.cursor.execute("""
                select * from TestNumbers
                where IntCol = :value""",
                value=None)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_2210_bind_number_array_direct(self):
        "2210 - test binding in a number array"
        return_value = self.cursor.var(oracledb.NUMBER)
        array = [r[2] for r in self.raw_data]
        statement = """
                begin
                  :return_value := pkg_TestNumberArrays.TestInArrays(
                      :start_value, :array);
                end;"""
        self.cursor.execute(statement, return_value=return_value,
                            start_value=5, array=array)
        self.assertEqual(return_value.getvalue(), 73.75)
        array = list(range(15))
        self.cursor.execute(statement, start_value=10, array=array)
        self.assertEqual(return_value.getvalue(), 115.0)

    def test_2211_bind_number_array_by_sizes(self):
        "2211 - test binding in a number array (with setinputsizes)"
        return_value = self.cursor.var(oracledb.NUMBER)
        self.cursor.setinputsizes(array = [oracledb.NUMBER, 10])
        array = [r[2] for r in self.raw_data]
        self.cursor.execute("""
                begin
                  :return_value := pkg_TestNumberArrays.TestInArrays(
                      :start_value, :array);
                end;""",
                return_value=return_value,
                start_value=6,
                array=array)
        self.assertEqual(return_value.getvalue(), 74.75)

    def test_2212_bind_number_array_by_var(self):
        "2212 - test binding in a number array (with arrayvar)"
        return_value = self.cursor.var(oracledb.NUMBER)
        array = self.cursor.arrayvar(oracledb.NUMBER,
                                     [r[2] for r in self.raw_data])
        self.cursor.execute("""
                begin
                  :return_value := pkg_TestNumberArrays.TestInArrays(
                      :integer_value, :array);
                end;""",
                return_value = return_value,
                integer_value = 7,
                array = array)
        self.assertEqual(return_value.getvalue(), 75.75)

    def test_2213_bind_zero_length_number_array_by_var(self):
        "2213 - test binding in a zero length number array (with arrayvar)"
        return_value = self.cursor.var(oracledb.NUMBER)
        array = self.cursor.arrayvar(oracledb.NUMBER, 0)
        self.cursor.execute("""
                begin
                  :return_value := pkg_TestNumberArrays.TestInArrays(
                      :integer_value, :array);
                end;""",
                return_value=return_value,
                integer_value=8,
                array=array)
        self.assertEqual(return_value.getvalue(), 8.0)
        self.assertEqual(array.getvalue(), [])

    def test_2214_bind_in_out_number_array_by_var(self):
        "2214 - test binding in/out a number array (with arrayvar)"
        array = self.cursor.arrayvar(oracledb.NUMBER, 10)
        original_data = [r[2] for r in self.raw_data]
        expected_data = [original_data[i - 1] * 10 for i in range(1, 6)] + \
                         original_data[5:]
        array.setvalue(0, original_data)
        self.cursor.execute("""
                begin
                  pkg_TestNumberArrays.TestInOutArrays(:num_elems, :array);
                end;""",
                num_elems=5,
                array=array)
        self.assertEqual(array.getvalue(), expected_data)

    def test_2215_bind_out_number_array_by_var(self):
        "2215 - test binding out a Number array (with arrayvar)"
        array = self.cursor.arrayvar(oracledb.NUMBER, 6)
        expected_data = [i * 100 for i in range(1, 7)]
        self.cursor.execute("""
                begin
                  pkg_TestNumberArrays.TestOutArrays(:num_elems, :array);
                end;""",
                num_elems=6,
                array=array)
        self.assertEqual(array.getvalue(), expected_data)

    def test_2216_bind_out_set_input_sizes(self):
        "2216 - test binding out with set input sizes defined"
        bind_vars = self.cursor.setinputsizes(value = oracledb.NUMBER)
        self.cursor.execute("""
                begin
                  :value := 5;
                end;""")
        self.assertEqual(bind_vars["value"].getvalue(), 5)

    def test_2217_bind_in_out_set_input_sizes(self):
        "2217 - test binding in/out with set input sizes defined"
        bind_vars = self.cursor.setinputsizes(value = oracledb.NUMBER)
        self.cursor.execute("""
                begin
                  :value := :value + 5;
                end;""",
                value = 1.25)
        self.assertEqual(bind_vars["value"].getvalue(), 6.25)

    def test_2218_bind_out_var(self):
        "2218 - test binding out with cursor.var() method"
        var = self.cursor.var(oracledb.NUMBER)
        self.cursor.execute("""
                begin
                  :value := 5;
                end;""",
                value = var)
        self.assertEqual(var.getvalue(), 5)

    def test_2219_bind_in_out_var_direct_set(self):
        "2219 - test binding in/out with cursor.var() method"
        var = self.cursor.var(oracledb.NUMBER)
        var.setvalue(0, 2.25)
        self.cursor.execute("""
                begin
                  :value := :value + 5;
                end;""",
                value = var)
        self.assertEqual(var.getvalue(), 7.25)

    def test_2220_cursor_description(self):
        "2220 - test cursor description is accurate"
        self.cursor.execute("select * from TestNumbers")
        expected_value = [
            ('INTCOL', oracledb.DB_TYPE_NUMBER, 10, None, 9, 0, False),
            ('LONGINTCOL', oracledb.DB_TYPE_NUMBER, 17, None, 16, 0, False),
            ('NUMBERCOL', oracledb.DB_TYPE_NUMBER, 13, None, 9, 2, False),
            ('FLOATCOL', oracledb.DB_TYPE_NUMBER, 127, None, 126, -127, False),
            ('UNCONSTRAINEDCOL', oracledb.DB_TYPE_NUMBER, 127, None, 0, -127,
                    False),
            ('NULLABLECOL', oracledb.DB_TYPE_NUMBER, 39, None, 38, 0, True)
        ]
        self.assertEqual(self.cursor.description, expected_value)

    def test_2221_fetchall(self):
        "2221 - test that fetching all of the data returns the correct results"
        self.cursor.execute("select * From TestNumbers order by IntCol")
        self.assertEqual(self.cursor.fetchall(), self.raw_data)
        self.assertEqual(self.cursor.fetchall(), [])

    def test_2222_fetchmany(self):
        "2222 - test that fetching data in chunks returns the correct results"
        self.cursor.execute("select * From TestNumbers order by IntCol")
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[0:3])
        self.assertEqual(self.cursor.fetchmany(2), self.raw_data[3:5])
        self.assertEqual(self.cursor.fetchmany(4), self.raw_data[5:9])
        self.assertEqual(self.cursor.fetchmany(3), self.raw_data[9:])
        self.assertEqual(self.cursor.fetchmany(3), [])

    def test_2223_fetchone(self):
        "2223 - test that fetching a single row returns the correct results"
        self.cursor.execute("""
                select *
                from TestNumbers
                where IntCol in (3, 4)
                order by IntCol""")
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[3])
        self.assertEqual(self.cursor.fetchone(), self.data_by_key[4])
        self.assertEqual(self.cursor.fetchone(), None)

    def test_2224_return_as_long(self):
        "2224 - test that fetching a long integer returns such in Python"
        self.cursor.execute("""
                select NullableCol
                from TestNumbers
                where IntCol = 9""")
        col, = self.cursor.fetchone()
        self.assertEqual(col, 25004854810776297743)

    def test_2225_return_constant_float(self):
        "2225 - test fetching a floating point number returns such in Python"
        self.cursor.execute("select 1.25 from dual")
        result, = self.cursor.fetchone()
        self.assertEqual(result, 1.25)

    def test_2226_return_constant_integer(self):
        "2226 - test that fetching an integer returns such in Python"
        self.cursor.execute("select 148 from dual")
        result, = self.cursor.fetchone()
        self.assertEqual(result, 148)
        self.assertTrue(isinstance(result, int), "integer not returned")

    def test_2227_acceptable_boundary_numbers(self):
        "2227 - test that acceptable boundary numbers are handled properly"
        in_values = [decimal.Decimal("9.99999999999999e+125"),
                     decimal.Decimal("-9.99999999999999e+125"), 0.0, 1e-130,
                     -1e-130]
        out_values = [int("9" * 15 + "0" * 111), -int("9" * 15 + "0" * 111),
                      0, 1e-130, -1e-130]
        for in_value, out_value in zip(in_values, out_values):
            self.cursor.execute("select :1 from dual", (in_value,))
            result, = self.cursor.fetchone()
            self.assertEqual(result, out_value)

    def test_2228_unacceptable_boundary_numbers(self):
        "2228 - test that unacceptable boundary numbers are rejected"
        in_values = [1e126, -1e126, float("inf"), float("-inf"),
                     float("NaN"), decimal.Decimal("1e126"),
                     decimal.Decimal("-1e126"), decimal.Decimal("inf"),
                     decimal.Decimal("-inf"), decimal.Decimal("NaN")]
        no_rep_err = "value cannot be represented as an Oracle number"
        invalid_err = "invalid number"
        expected_errors = [no_rep_err, no_rep_err, invalid_err, invalid_err,
                           invalid_err, no_rep_err, no_rep_err, invalid_err,
                           invalid_err, invalid_err]
        for in_value, error in zip(in_values, expected_errors):
            self.assertRaisesRegex(oracledb.DatabaseError, error,
                                   self.cursor.execute, "select :1 from dual",
                                   (in_value,))

    def test_2229_return_float_from_division(self):
        "2229 - test that fetching the result of division returns a float"
        self.cursor.execute("""
                select IntCol / 7
                from TestNumbers
                where IntCol = 1""")
        result, = self.cursor.fetchone()
        self.assertEqual(result, 1.0 / 7.0)
        self.assertTrue(isinstance(result, float), "float not returned")

    def test_2230_string_format(self):
        "2230 - test that string format is returned properly"
        var = self.cursor.var(oracledb.NUMBER)
        self.assertEqual(str(var),
                "<cx_Oracle.Var of type DB_TYPE_NUMBER with value None>")
        var.setvalue(0, 4)
        self.assertEqual(str(var),
                "<cx_Oracle.Var of type DB_TYPE_NUMBER with value 4.0>")

    def test_2231_bind_binary_double(self):
        "2231 - test that binding binary double is possible"
        statement = "select :1 from dual"
        self.cursor.setinputsizes(oracledb.DB_TYPE_BINARY_DOUBLE)
        self.cursor.execute(statement, (5,))
        self.assertEqual(self.cursor.bindvars[0].type,
                         oracledb.DB_TYPE_BINARY_DOUBLE)
        value, = self.cursor.fetchone()
        self.assertEqual(value, 5)
        self.cursor.execute(statement, (1.5,))
        self.assertEqual(self.cursor.bindvars[0].type,
                         oracledb.DB_TYPE_BINARY_DOUBLE)
        value, = self.cursor.fetchone()
        self.assertEqual(value, 1.5)
        self.cursor.execute(statement, (decimal.Decimal("NaN"),))
        self.assertEqual(self.cursor.bindvars[0].type,
                         oracledb.DB_TYPE_BINARY_DOUBLE)
        value, = self.cursor.fetchone()
        self.assertEqual(str(value), str(float("NaN")))

    def test_2232_fetch_binary_int(self):
        "2232 - test fetching numbers as binary integers"
        self.cursor.outputtypehandler = self.output_type_handler_binary_int
        for value in (1, 2 ** 31, 2 ** 63 - 1, -1, -2 ** 31, -2 ** 63 + 1):
            self.cursor.execute("select :1 from dual", [str(value)])
            fetched_value, = self.cursor.fetchone()
            self.assertEqual(value, fetched_value)

    def test_2233_out_bind_binary_int(self):
        "2233 - test binding native integer as an out bind"
        statement = "begin :value := 2.9; end;"
        simple_var = self.cursor.var(oracledb.DB_TYPE_BINARY_INTEGER)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), 2)

        statement = "begin :value := 1.5; end;"
        simple_var = self.cursor.var(oracledb.DB_TYPE_BINARY_INTEGER)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), 1)

    def test_2234_in_bind_binary_int(self):
        "2234 - test binding in a native integer"
        statement = "begin :value := :value + 2.5; end;"
        simple_var = self.cursor.var(oracledb.DB_TYPE_BINARY_INTEGER)
        simple_var.setvalue(0, 0)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), 2)

        simple_var.setvalue(0, -5)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), -2)

    def test_2235_setting_decimal_value_binary_int(self):
        "2235 - test setting decimal value for binary int"
        statement = "begin :value := :value + 2.5; end;"
        simple_var = self.cursor.var(oracledb.DB_TYPE_BINARY_INTEGER)
        simple_var.setvalue(0, 2.5)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), 4)

    def test_2236_out_bind_binary_int_with_large_value(self):
        "2236 - bind a large value to binary int"
        statement = "begin :value := POWER(2, 31) - 1; end;"
        simple_var = self.cursor.var(oracledb.DB_TYPE_BINARY_INTEGER)
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), 2**31 - 1)

        statement = "begin :value := POWER(-2, 31) - 1; end;"
        self.cursor.execute(statement, [simple_var])
        self.assertEqual(simple_var.getvalue(), -2**31 - 1)

if __name__ == "__main__":
    test_env.run_test_cases()
