#------------------------------------------------------------------------------
# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
3500 - Module for testing the JSON data type.
"""

import datetime
import decimal
import unittest

import cx_Oracle as oracledb
import test_env

@unittest.skipUnless(test_env.get_client_version() >= (21, 0),
                     "unsupported client")
@unittest.skipUnless(test_env.get_server_version() >= (21, 0),
                     "unsupported server")
class TestCase(test_env.BaseTestCase):

    json_data = [
        True,
        False,
        'String',
        b'Some Bytes',
        {},
        {"name": None},
        {"name": "John"},
        {"age": 30},
        {"Permanent": True},
        {
            "employee": {
                "name":"John",
                "age": 30,
                "city": "Delhi",
                "Parmanent": True
            }
        },
        {
            "employees": ["John", "Matthew", "James"]
        },
        {
            "employees": [
                {
                    "employee1": {"name": "John", "city": "Delhi"}
                },
                {
                    "employee2": {"name": "Matthew", "city": "Mumbai"}
                },
                {
                    "employee3": {"name": "James", "city": "Bangalore"}
                }
            ]
        }
    ]

    def __bind_scalar_as_json(self, data):
        self.cursor.execute("truncate table TestJson")
        out_var = self.cursor.var(oracledb.DB_TYPE_JSON,
                                  arraysize=len(data))
        self.cursor.setinputsizes(None, oracledb.DB_TYPE_JSON, out_var)
        bind_data = list(enumerate(data))
        self.cursor.executemany("""
                insert into TestJson values (:1, :2)
                returning JsonCol into :json_out""", bind_data)
        self.connection.commit()
        self.assertEqual(out_var.values, [[v] for v in data])

    def test_3500_insert_and_fetch_single_json(self):
        "3500 - insert and fetch single row with JSON"
        self.cursor.execute("truncate table TestJson")
        self.cursor.setinputsizes(None, oracledb.DB_TYPE_JSON)
        self.cursor.execute("insert into TestJson values (:1, :2)",
                            [1, self.json_data])
        self.cursor.execute("select JsonCol from TestJson")
        result, = self.cursor.fetchone()
        self.assertEqual(result, self.json_data)

    def test_3501_execute_with_dml_returning(self):
        "3501 - inserting single rows with JSON and DML returning"
        json_val = self.json_data[11]
        self.cursor.execute("truncate table TestJson")
        json_out = self.cursor.var(oracledb.DB_TYPE_JSON)
        self.cursor.setinputsizes(None, oracledb.DB_TYPE_JSON, json_out)
        self.cursor.execute("""
                insert into TestJson values (:1, :2)
                returning  JsonCol into :json_out""",
                [1, json_val])
        self.assertEqual(json_out.getvalue(0), [json_val])

    def test_3502_insert_and_fetch_multiple_json(self):
        "3502 - insert and fetch multiple rows with JSON"
        self.cursor.execute("truncate table TestJson")
        self.cursor.setinputsizes(None, oracledb.DB_TYPE_JSON)
        data = list(enumerate(self.json_data))
        self.cursor.executemany("insert into TestJson values(:1, :2)", data)
        self.cursor.execute("select * from TestJson")
        fetched_data = self.cursor.fetchall()
        self.assertEqual(fetched_data, data)

    def test_3503_executemany_with_dml_returning(self):
        "3503 - inserting multiple rows with JSON and DML returning"
        self.cursor.execute("truncate table TestJson")
        int_values = [i for i in range(len(self.json_data))]
        out_int_var = self.cursor.var(int, arraysize=len(int_values))
        out_json_var = self.cursor.var(oracledb.DB_TYPE_JSON,
                                       arraysize=len(int_values))
        self.cursor.setinputsizes(None, oracledb.DB_TYPE_JSON, out_int_var,
                                  out_json_var)
        data = list(zip(int_values, self.json_data))
        self.cursor.executemany("""
                insert into TestJson
                values(:int_val, :json_val)
                returning IntCol, JsonCol into :int_var, :json_var""", data)
        self.assertEqual(out_int_var.values, [[v] for v in int_values])
        self.assertEqual(out_json_var.values, [[v] for v in self.json_data])

    def test_3504_boolean(self):
        "3504 - test binding boolean values as scalar JSON values"
        data = [
            True,
            False,
            True,
            True,
            False,
            True
        ]
        self.__bind_scalar_as_json(data)

    def test_3505_strings_and_bytes(self):
        "3505 - test binding strings/bytes values as scalar JSON values"
        data = [
            "String 1",
            b"A raw value",
            "A much longer string",
            b"A much longer RAW value",
            "Short string",
            b"Y"
        ]
        self.__bind_scalar_as_json(data)

    def test_3506_datetime(self):
        "3506 - test binding dates/intervals as scalar JSON values"
        data = [
            datetime.datetime.today(),
            datetime.datetime(2004, 2, 1, 3, 4, 5),
            datetime.datetime(2020, 12, 2, 13, 29, 14),
            datetime.timedelta(8.5),
            datetime.datetime(2002, 12, 13, 9, 36, 0),
            oracledb.Timestamp(2002, 12, 13, 9, 36, 0),
            datetime.datetime(2002, 12, 13)
        ]
        self.__bind_scalar_as_json(data)

    def test_3507_bind_number(self):
        "3507 - test binding number in json values"
        data = [
            0,
            1,
            25.25,
            6088343244,
            -9999999999999999999,
            decimal.Decimal("0.25"),
            decimal.Decimal("10.25"),
            decimal.Decimal("319438950232418390.273596")
        ]
        self.__bind_scalar_as_json(data)

if __name__ == "__main__":
    test_env.run_test_cases()
