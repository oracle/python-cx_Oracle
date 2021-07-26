#------------------------------------------------------------------------------
#Copyright (c) 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
3800 - Module for testing the input and output type handlers.
"""

import json

import cx_Oracle as oracledb
import test_env

class Building(object):

    def __init__(self, building_id, description, num_floors):
        self.building_id = building_id
        self.description = description
        self.num_floors = num_floors

    def __repr__(self):
        return "<Building %s: %s>" % (self.building_id, self.description)

    def __eq__(self, other):
        if isinstance(other, Building):
            return other.building_id == self.building_id \
                    and other.description == self.description \
                    and other.num_floors == self.num_floors
        return NotImplemented

    def to_json(self):
        return json.dumps(self.__dict__)

    @classmethod
    def from_json(cls, value):
        result = json.loads(value)
        return cls(**result)


class TestCase(test_env.BaseTestCase):

    def building_in_converter(self, value):
        return value.to_json()

    def input_type_handler(self, cursor, value, num_elements):
        if isinstance(value, Building):
            return cursor.var(oracledb.STRING, arraysize=num_elements,
                              inconverter=self.building_in_converter)

    def output_type_handler(self, cursor, name, default_type, size, precision,
                            scale):
        if default_type == oracledb.STRING:
            return cursor.var(default_type, arraysize=cursor.arraysize,
                              outconverter=Building.from_json)

    def test_3800(self):
        "3800 - binding unsupported python object without input type handler"
        self.cursor.execute("truncate table TestTempTable")
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        building = Building(1, "The First Building", 5)
        self.assertRaises(oracledb.NotSupportedError, self.cursor.execute, sql,
                          (building.building_id, building))

    def test_3801(self):
        "3801 - not callable input type handler"
        self.cursor.execute("truncate table TestTempTable")
        building = Building(1, "The First Building", 5)
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.inputtypehandler = 5
        self.assertRaises(TypeError, self.cursor.execute, sql,
                          (building.building_id, building))

    def test_3802(self):
        "3802 - binding unsupported python object with input type handler"
        self.cursor.execute("truncate table TestTempTable")
        building = Building(1, "The First Building", 5)
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        self.cursor.inputtypehandler = self.input_type_handler
        self.cursor.execute(sql, (building.building_id, building))
        self.connection.commit()
        self.cursor.execute("select IntCol, StringCol from TestTempTable")
        self.assertEqual(self.cursor.fetchall(),
                         [(building.building_id, building.to_json())])

    def test_3803(self):
        "3803 - input type handler and output type handler on cursor level"
        self.cursor.execute("truncate table TestTempTable")
        building_one = Building(1, "The First Building", 5)
        building_two = Building(2, "The Second Building", 87)
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        cursor_one = self.connection.cursor()
        cursor_two = self.connection.cursor()
        cursor_one.inputtypehandler = self.input_type_handler
        cursor_one.execute(sql, (building_one.building_id, building_one))
        self.connection.commit()
        cursor_one.execute("select IntCol, StringCol from TestTempTable")
        self.assertEqual(cursor_one.fetchall(),
                         [(building_one.building_id, building_one.to_json())])
        self.assertRaises(oracledb.NotSupportedError, cursor_two.execute,
                          sql, (building_two.building_id, building_two))
        cursor_two.outputtypehandler = self.output_type_handler
        cursor_two.execute("select IntCol, StringCol from TestTempTable")
        self.assertEqual(cursor_two.fetchall(),
                         [(building_one.building_id, building_one)])

    def test_3804(self):
        "3804 - input type handler and output type handler on connection level"
        self.cursor.execute("truncate table TestTempTable")
        building_one = Building(1, "The First Building", 5)
        building_two = Building(2, "The Second Building", 87)
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        connection = test_env.get_connection()
        connection.inputtypehandler = self.input_type_handler
        cursor_one = connection.cursor()
        cursor_two = connection.cursor()
        cursor_one.execute(sql, (building_one.building_id, building_one))
        cursor_two.execute(sql, (building_two.building_id, building_two))
        connection.commit()
        expected_data = [
            (building_one.building_id, building_one),
            (building_two.building_id, building_two)
        ]
        connection.outputtypehandler = self.output_type_handler
        cursor_one.execute("select IntCol, StringCol from TestTempTable")
        self.assertEqual(cursor_one.fetchall(), expected_data)
        cursor_two.execute("select IntCol, StringCol from TestTempTable")
        self.assertEqual(cursor_two.fetchall(), expected_data)
        other_cursor = self.connection.cursor()
        self.assertRaises(oracledb.NotSupportedError, other_cursor.execute,
                          sql, (building_one.building_id, building_one))

if __name__ == "__main__":
    test_env.run_test_cases()
