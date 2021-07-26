#------------------------------------------------------------------------------
# Copyright (c) 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
3600 - Module for testing the conversions of outputtype handler.
"""

import datetime
import decimal
import sys
import unittest

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):

    def __test_type_handler(self, input_type, output_type, in_value,
                            expected_out_value):
        def type_handler(cursor, name, default_type, size,
                            precision, scale):
            return cursor.var(output_type, arraysize=cursor.arraysize)
        self.cursor.outputtypehandler = type_handler
        var = self.cursor.var(input_type)
        var.setvalue(0, in_value)
        self.cursor.execute("select :1 from dual", [var])
        fetched_value, = self.cursor.fetchone()
        self.assertEqual(fetched_value, expected_out_value)

    def __test_type_handler_blob(self, output_type):
        def type_handler(cursor, name, default_type, size,
                            precision, scale):
            if default_type == oracledb.DB_TYPE_BLOB:
                return cursor.var(output_type, arraysize=cursor.arraysize)
        self.cursor.outputtypehandler = type_handler
        in_value = b"Some binary data"
        self.cursor.execute("truncate table TestBLOBs")
        self.cursor.execute("insert into TestBLOBs values(1, :val)",
                            val=in_value)
        self.connection.commit()
        self.cursor.execute("select BlobCol, IntCol, BlobCol from TestBLOBs")
        self.assertEqual(self.cursor.fetchone(), (in_value, 1, in_value))

    def __test_type_handler_clob(self, output_type):
        def type_handler(cursor, name, default_type, size,
                            precision, scale):
            if default_type == oracledb.DB_TYPE_CLOB:
                return cursor.var(output_type, arraysize=cursor.arraysize)
        self.cursor.outputtypehandler = type_handler
        in_value = "Some clob data"
        self.cursor.execute("truncate table TestCLOBs")
        self.cursor.execute("insert into TestCLOBs values(1, :val)",
                            val=in_value)
        self.connection.commit()
        self.cursor.execute("select ClobCol, IntCol, ClobCol from TestCLOBs")
        self.assertEqual(self.cursor.fetchone(), (in_value, 1, in_value))

    def __test_type_handler_nclob(self, output_type):
        def type_handler(cursor, name, default_type, size,
                            precision, scale):
            if default_type == oracledb.DB_TYPE_NCLOB:
                return cursor.var(output_type, arraysize=cursor.arraysize)
        self.cursor.outputtypehandler = type_handler
        in_value = "Some nclob data"
        self.cursor.execute("truncate table TestNCLOBs")
        self.cursor.execute("insert into TestNCLOBs values(1, :val)",
                            val=in_value)
        self.connection.commit()
        self.cursor.execute("select NClobCol, IntCol, NClobCol from TestNCLOBs")
        self.assertEqual(self.cursor.fetchone(), (in_value, 1, in_value))

    def setUp(self):
        super().setUp()
        stmt = "ALTER SESSION SET NLS_DATE_FORMAT = 'YYYY-MM-DD HH24:MI:SS'" \
               "NLS_TIMESTAMP_FORMAT = 'YYYY-MM-DD HH24:MI:SS.FF6'" \
               "NLS_TIMESTAMP_TZ_FORMAT = 'YYYY-MM-DD HH24:MI:SS.FF6'" \
               "time_zone='Europe/London'"
        self.cursor.execute(stmt)

    def test_3600_VARCHAR_to_NUMBER(self):
        "3600 - outtypehandler conversion from varchar to number"
        self.__test_type_handler(oracledb.DB_TYPE_VARCHAR,
                                 oracledb.DB_TYPE_NUMBER, "31.5", 31.5)

    def test_3601_CHAR_to_NUMBER(self):
        "3601 - outtypehandler conversion from char to number"
        self.__test_type_handler(oracledb.DB_TYPE_CHAR,
                                 oracledb.DB_TYPE_NUMBER, "31.5", 31.5)

    def test_3602_LONG_to_NUMBER(self):
        "3602 - outtypehandler conversion from long to number"
        self.__test_type_handler(oracledb.DB_TYPE_LONG,
                                 oracledb.DB_TYPE_NUMBER, "31.5", 31.5)

    def test_3603_BINT_to_NUMBER(self):
        "3603 -test outtypehandler conversion from bint to number"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_INTEGER,
                                 oracledb.DB_TYPE_NUMBER, 31, 31)

    def test_3604_VARCHAR_to_BINT(self):
        "3604 outtypehandler conversion from varchar to bint"
        self.__test_type_handler(oracledb.DB_TYPE_VARCHAR,
                                 oracledb.DB_TYPE_BINARY_INTEGER, "31.5", 31)

    def test_3605_CHAR_to_BINT(self):
        "3605 outtypehandler conversion from char to bint"
        self.__test_type_handler(oracledb.DB_TYPE_CHAR,
                                 oracledb.DB_TYPE_BINARY_INTEGER, "31.5", 31)

    def test_3606_LONG_to_BINT(self):
        "3606 outtypehandler conversion from long to bint"
        self.__test_type_handler(oracledb.DB_TYPE_LONG,
                                 oracledb.DB_TYPE_BINARY_INTEGER, "31.5", 31)

    def test_3607_NUMBER_to_BINT(self):
        "3607 outtypehandler conversion from number to bint"
        self.__test_type_handler(oracledb.DB_TYPE_NUMBER,
                                 oracledb.DB_TYPE_BINARY_INTEGER, 31.5, 31)

    def test_3608_BINARY_DOUBLE_to_BINT(self):
        "3608 outtypehandler conversion from binary double to bint"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_DOUBLE,
                                 oracledb.DB_TYPE_BINARY_INTEGER, 31.5, 31)

    def test_3609_BINARY_FLOAT_to_BINT(self):
        "3609 outtypehandler conversion from varchar to bint"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_FLOAT,
                                 oracledb.DB_TYPE_BINARY_INTEGER, 31.5, 31)

    def test_3610_DATE_to_VARCHAR(self):
        "3610 outtypehandler conversion from date to varchar"
        in_val = datetime.date(2021, 2, 1)
        out_val = "2021-02-01 00:00:00"
        self.__test_type_handler(oracledb.DB_TYPE_DATE,
                                 oracledb.DB_TYPE_VARCHAR, in_val, out_val)

    def test_3611_DATE_to_CHAR(self):
        "3611 outtypehandler conversion from date to char"
        in_val = datetime.date(2021, 2, 1)
        out_val = "2021-02-01 00:00:00"
        self.__test_type_handler(oracledb.DB_TYPE_DATE,
                                 oracledb.DB_TYPE_CHAR, in_val, out_val)

    def test_3612_DATE_to_LONG(self):
        "3612 outtypehandler conversion from date to long"
        in_val = datetime.date(2021, 2, 1)
        out_val = "2021-02-01 00:00:00"
        self.__test_type_handler(oracledb.DB_TYPE_DATE,
                                 oracledb.DB_TYPE_LONG, in_val , out_val)

    def test_3613_NUMBER_to_VARCHAR(self):
        "3613 outtypehandler conversion from number to varchar"
        self.__test_type_handler(oracledb.DB_TYPE_NUMBER,
                                 oracledb.DB_TYPE_VARCHAR, 31.5, "31.5")

    def test_3614_NUMBER_to_CHAR(self):
        "3614 outtypehandler conversion from number to char"
        self.__test_type_handler(oracledb.DB_TYPE_NUMBER,
                                 oracledb.DB_TYPE_CHAR, 31.5, "31.5")

    def test_3615_NUMBER_to_LONG(self):
        "3615 outtypehandler conversion from number to long"
        self.__test_type_handler(oracledb.DB_TYPE_NUMBER,
                                 oracledb.DB_TYPE_LONG, 31.5, "31.5")

    def test_3616_INTERVAL_to_VARCHAR(self):
        "3616 outtypehandler conversion from interval to varchar"
        in_val = datetime.timedelta(days=-1, seconds=86314,
                                    microseconds=431152)
        out_val = "-000000001 23:58:34.431152000"
        self.__test_type_handler(oracledb.DB_TYPE_INTERVAL_DS,
                                 oracledb.DB_TYPE_VARCHAR, in_val, out_val)

    def test_3617_INTERVAL_to_CHAR(self):
        "3617 outtypehandler conversion from interval to char"
        in_val = datetime.timedelta(days=-1, seconds=86314,
                                    microseconds=431152)
        out_val = "-000000001 23:58:34.431152000"
        self.__test_type_handler(oracledb.DB_TYPE_INTERVAL_DS,
                                 oracledb.DB_TYPE_CHAR, in_val, out_val)

    def test_3618_INTERVAL_to_LONG(self):
        "3618 outtypehandler conversion from interval to long"
        in_val = datetime.timedelta(days=-1, seconds=86314,
                                    microseconds=431152)
        out_val = "-000000001 23:58:34.431152000"
        self.__test_type_handler(oracledb.DB_TYPE_INTERVAL_DS,
                                 oracledb.DB_TYPE_LONG, in_val, out_val)

    def test_3619_TIMESTAMP_to_VARCHAR(self):
        "3619 outtypehandler conversion from timestamp to varchar"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP,
                                 oracledb.DB_TYPE_VARCHAR, in_val, str(in_val))

    def test_3620_TIMESTAMP_to_CHAR(self):
        "3620 outtypehandler conversion from timestamp to char"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP,
                                 oracledb.DB_TYPE_CHAR, in_val, str(in_val))

    def test_3621_TIMESTAMP_to_LONG(self):
        "3621 outtypehandler conversion from timestamp to long"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP,
                                 oracledb.DB_TYPE_LONG, in_val, str(in_val))

    def test_3622_TIMESTAMP_TZ_to_VARCHAR(self):
        "3622 outtypehandler conversion from timestamptz to varchar"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_TZ,
                                 oracledb.DB_TYPE_VARCHAR, in_val, str(in_val))

    def test_3623_TIMESTAMP_TZ_to_CHAR(self):
        "3623 outtypehandler conversion from timestamptz to char"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_TZ,
                                 oracledb.DB_TYPE_CHAR, in_val, str(in_val))

    def test_3624_TIMESTAMP_TZ_to_LONG(self):
        "3624 outtypehandler conversion from timestamptz to long"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_TZ,
                                 oracledb.DB_TYPE_LONG, in_val, str(in_val))

    def test_3625_TIMESTAMP_LTZ_to_VARCHAR(self):
        "3625 outtypehandler conversion from timestampltz to varchar"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_LTZ,
                                 oracledb.DB_TYPE_VARCHAR, in_val, str(in_val))

    def test_3626_TIMESTAMP_LTZ_to_CHAR(self):
        "3626 outtypehandler conversion from timestamp_ltz to char"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_LTZ,
                                 oracledb.DB_TYPE_CHAR, in_val, str(in_val))

    def test_3627_TIMESTAMP_LTZ_to_LONG(self):
        "3627 outtypehandler conversion from timestamp_LTZ to long"
        in_val = datetime.datetime(2002, 12, 17, 1, 2, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_LTZ,
                                 oracledb.DB_TYPE_LONG, in_val, str(in_val))

    def test_3628_BINT_to_VARCHAR(self):
        "3628 outtypehandler conversion from bint to varchar"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_INTEGER,
                                 oracledb.DB_TYPE_VARCHAR, 31, "31")

    def test_3629_BINT_to_CHAR(self):
        "3629 outtypehandler conversion from bint to char"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_INTEGER,
                                 oracledb.DB_TYPE_CHAR, 31, "31")

    def test_3630_BINT_to_LONG(self):
        "3630 outtypehandler conversion from bint to long"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_INTEGER,
                                 oracledb.DB_TYPE_LONG, 31, "31")

    def test_3631_NUMBER_to_BINARY_DOUBLE(self):
        "3631 outtypehandler conversion from number to binary double"
        self.__test_type_handler(oracledb.DB_TYPE_NUMBER,
                                 oracledb.DB_TYPE_BINARY_DOUBLE, 31.5, 31.5)

    def test_3632_BINARY_FLOAT_to_BINARY_DOUBLE(self):
        "3632 outtypehandler conversion from binary float to binary double"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_FLOAT,
                                 oracledb.DB_TYPE_BINARY_DOUBLE, 31.5, 31.5)

    def test_3633_VARCHAR_to_BINARY_DOUBLE(self):
        "3633 outtypehandler conversion from varchar to binary double"
        self.__test_type_handler(oracledb.DB_TYPE_VARCHAR,
                                 oracledb.DB_TYPE_BINARY_DOUBLE, "31.5", 31.5)

    def test_3634_CHAR_to_BINARY_DOUBLE(self):
        "3634 outtypehandler conversion from char to binary double"
        self.__test_type_handler(oracledb.DB_TYPE_CHAR,
                                 oracledb.DB_TYPE_BINARY_DOUBLE, "31.5", 31.5)

    def test_3635_LONG_to_BINARY_DOUBLE(self):
        "3635 outtypehandler conversion from long to binary double"
        self.__test_type_handler(oracledb.DB_TYPE_LONG,
                                 oracledb.DB_TYPE_BINARY_DOUBLE, "31.5", 31.5)

    def test_3636_NUMBER_to_BINARY_FLOAT(self):
        "3636 outtypehandler conversion from number to binary float"
        self.__test_type_handler(oracledb.DB_TYPE_NUMBER,
                                 oracledb.DB_TYPE_BINARY_FLOAT, 31.5, 31.5)

    def test_3637_BINARY_DOUBLE_to_BINARY_FLOAT(self):
        "3637 outtypehandler conversion from binary double to binary float"
        self.__test_type_handler(oracledb.DB_TYPE_BINARY_DOUBLE,
                                 oracledb.DB_TYPE_BINARY_FLOAT, 31.5, 31.5)

    def test_3638_VARCHAR_to_BINARY_FLOAT(self):
        "3638 outtypehandler conversion from varchar to binary float"
        self.__test_type_handler(oracledb.DB_TYPE_VARCHAR,
                                 oracledb.DB_TYPE_BINARY_FLOAT, "31.5", 31.5)

    def test_3639_CHAR_to_BINARY_FLOAT(self):
        "3639 outtypehandler conversion from char to binary float"
        self.__test_type_handler(oracledb.DB_TYPE_CHAR,
                                 oracledb.DB_TYPE_BINARY_FLOAT, "31.5", 31.5)

    def test_3640_LONG_to_BINARY_FLOAT(self):
        "3640 outtypehandler conversion from varchar to binary float"
        self.__test_type_handler(oracledb.DB_TYPE_LONG,
                                 oracledb.DB_TYPE_BINARY_FLOAT, "31.5", 31.5)

    def test_3641_VARCHAR_to_CHAR(self):
        "3641 outtypehandler from varchar to char"
        self.__test_type_handler(oracledb.DB_TYPE_VARCHAR,
                                 oracledb.DB_TYPE_CHAR, "31.5", "31.5")

    def test_3642_VARCHAR_to_LONG(self):
        "3642 outtypehandler from varchar to long"
        self.__test_type_handler(oracledb.DB_TYPE_VARCHAR,
                                 oracledb.DB_TYPE_LONG, "31.5", "31.5")

    def test_3643_LONG_to_VARCHAR(self):
        "3643 outtypehandler from long to varchar"
        self.__test_type_handler(oracledb.DB_TYPE_LONG,
                                 oracledb.DB_TYPE_VARCHAR, "31.5", "31.5")

    def test_3644_LONG_to_CHAR(self):
        "3644 outtypehandler from long to varhcar"
        self.__test_type_handler(oracledb.DB_TYPE_LONG,
                                 oracledb.DB_TYPE_CHAR, "31.5", "31.5")

    def test_3645_CHAR_to_VARCHAR(self):
        "3645 outtypehandler from char to varchar"
        self.__test_type_handler(oracledb.DB_TYPE_CHAR,
                                 oracledb.DB_TYPE_VARCHAR, "31.5", "31.5")

    def test_3646_CHAR_to_LONG(self):
        "3646 outtypehandler from char to long"
        self.__test_type_handler(oracledb.DB_TYPE_CHAR,
                                 oracledb.DB_TYPE_LONG, "31.5", "31.5")

    def test_3647_TIMESTAMP_to_TIMESTAMP_TZ(self):
        "3647 outtypehandler from timestamp to timestamptz"
        val = datetime.datetime(2002, 12, 17, 0, 0, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP,
                                 oracledb.DB_TYPE_TIMESTAMP_LTZ, val, val)

    def test_3648_TIMESTAMP_to_TIMESTAMP_LTZ(self):
        "3648 outtypehandler from timestamp to timestampltz"
        val = datetime.datetime(2002, 12, 17, 0, 0, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP,
                                 oracledb.DB_TYPE_TIMESTAMP_LTZ, val, val)

    def test_3649_TIMESTAMP_TZ_to_TIMESTAMP(self):
        "3649 outtypehandler from timestamptz to timestamp"
        val = datetime.datetime(2002, 12, 17, 0, 0, 16, 400000)
        self.__test_type_handler(oracledb.DB_TYPE_TIMESTAMP_TZ,
                                 oracledb.DB_TYPE_TIMESTAMP, val, val)

    def test_3650_NUMBER_TO_DATE(self):
        "3650 outtypehandler conversion from number to date is invalid"
        self.assertRaises(oracledb.DatabaseError, self.__test_type_handler,
                          oracledb.DB_TYPE_NUMBER, oracledb.DB_TYPE_DATE, 3,
                          3)

    def test_3651_CLOB_TO_CHAR(self):
        "3651 outtypehandler from CLOB to CHAR"
        val = "Some Clob String"
        self.__test_type_handler(oracledb.DB_TYPE_CLOB,
                                 oracledb.DB_TYPE_CHAR, val, val)

    def test_3652_CLOB_TO_VARCHAR(self):
        "3652 outtypehandler from CLOB to VARCHAR"
        val = "Some Clob String"
        self.__test_type_handler(oracledb.DB_TYPE_CLOB,
                                 oracledb.DB_TYPE_VARCHAR, val, val)

    def test_3653_CLOB_TO_LONG(self):
        "3653 outtypehandler from CLOB to LONG"
        val = "Some Clob String"
        self.__test_type_handler(oracledb.DB_TYPE_CLOB,
                                 oracledb.DB_TYPE_LONG, val, val)

    def test_3654_BLOB_TO_RAW(self):
        "3654 outtypehandler from BLOB to RAW"
        val = b"Some binary data"
        self.__test_type_handler(oracledb.DB_TYPE_BLOB,
                                 oracledb.DB_TYPE_RAW, val, val)

    def test_3655_BLOB_TO_LONG_RAW(self):
        "3655 outtypehandler from BLOB to LONGRAW"
        val = b"Some binary data"
        self.__test_type_handler(oracledb.DB_TYPE_BLOB,
                                 oracledb.DB_TYPE_LONG_RAW, val, val)

    def test_3656_BLOB_TO_LONG_RAW(self):
        "3656 outtypehandler from permanant BLOBs to LONG_RAW"
        self.__test_type_handler_blob(oracledb.DB_TYPE_LONG_RAW)

    def test_3657_BLOB_TO_RAW(self):
        "3657 outtypehandler from permanant BLOBs to RAW"
        self.__test_type_handler_blob(oracledb.DB_TYPE_RAW)

    def test_3658_CLOB_TO_VARCHAR(self):
        "3658 outtypehandler from permanant CLOBs to VARCHAR"
        self.__test_type_handler_clob(oracledb.DB_TYPE_VARCHAR)

    def test_3659_CLOB_TO_CHAR(self):
        "3659 outtypehandler from permanant CLOBs to CHAR"
        self.__test_type_handler_clob(oracledb.DB_TYPE_CHAR)

    def test_3660_CLOB_TO_LONG(self):
        "3660 outtypehandler from permanant CLOBs to LONG"
        self.__test_type_handler_clob(oracledb.DB_TYPE_LONG)

    def test_3661_NCLOB_TO_CHAR(self):
        "3661 outtypehandler from NCLOB to CHAR"
        val = "Some nclob data"
        self.__test_type_handler(oracledb.DB_TYPE_NCLOB,
                                 oracledb.DB_TYPE_CHAR, val, val)

    def test_3662_NCLOB_TO_VARCHAR(self):
        "3662 outtypehandler from NCLOB to VARCHAR"
        val = "Some nclob data"
        self.__test_type_handler(oracledb.DB_TYPE_NCLOB,
                                 oracledb.DB_TYPE_VARCHAR, val, val)

    def test_3663_NCLOB_TO_LONG(self):
        "3663 outtypehandler from NCLOB to LONG"
        val = "Some nclob data"
        self.__test_type_handler(oracledb.DB_TYPE_NCLOB,
                                 oracledb.DB_TYPE_LONG, val, val)

    def test_3664_NCLOB_TO_VARCHAR(self):
        "3664 outtypehandler from permanant NCLOBs to VARCHAR"
        self.__test_type_handler_nclob(oracledb.DB_TYPE_VARCHAR)

    def test_3665_NCLOB_TO_CHAR(self):
        "3665 outtypehandler from permanant NCLOBs to CHAR"
        self.__test_type_handler_nclob(oracledb.DB_TYPE_CHAR)

    def test_3666_NCLOB_TO_LONG(self):
        "3666 outtypehandler from permanant NCLOBs to LONG"
        self.__test_type_handler_nclob(oracledb.DB_TYPE_LONG)

if __name__ == "__main__":
    test_env.run_test_cases()
