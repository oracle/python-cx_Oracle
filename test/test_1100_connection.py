#------------------------------------------------------------------------------
# Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1100 - Module for testing connections
"""

import random
import string
import threading
import time
import unittest

import cx_Oracle as oracledb
import test_env

class TestCase(test_env.BaseTestCase):
    requires_connection = False

    def __connect_and_drop(self):
        """Connect to the database, perform a query and drop the connection."""
        connection = test_env.get_connection(threaded=True)
        cursor = connection.cursor()
        cursor.execute("select count(*) from TestNumbers")
        count, = cursor.fetchone()
        self.assertEqual(count, 10)

    def __verify_fetched_data(self, connection):
        expected_data = [f"String {i + 1}" for i in range(10)]
        sql = "select StringCol from TestStrings order by IntCol"
        for i in range(5):
            with connection.cursor() as cursor:
                fetched_data = [s for s, in cursor.execute(sql)]
                self.assertEqual(fetched_data, expected_data)

    def __verify_args(self, connection):
        self.assertEqual(connection.username, test_env.get_main_user(),
                         "user name differs")
        self.assertEqual(connection.tnsentry, test_env.get_connect_string(),
                         "tnsentry differs")
        self.assertEqual(connection.dsn, test_env.get_connect_string(),
                         "dsn differs")

    def __verify_attributes(self, connection, attrName, value, sql):
        setattr(connection, attrName, value)
        cursor = connection.cursor()
        cursor.execute(sql)
        result, = cursor.fetchone()
        self.assertEqual(result, value, "%s value mismatch" % attrName)

    def test_1100_all_args(self):
        "1100 - connection to database with user, password, TNS separate"
        connection = test_env.get_connection()
        self.__verify_args(connection)

    def test_1101_app_context(self):
        "1101 - test use of application context"
        namespace = "CLIENTCONTEXT"
        app_context_entries = [
            ( namespace, "ATTR1", "VALUE1" ),
            ( namespace, "ATTR2", "VALUE2" ),
            ( namespace, "ATTR3", "VALUE3" )
        ]
        connection = test_env.get_connection(appcontext=app_context_entries)
        cursor = connection.cursor()
        for namespace, name, value in app_context_entries:
            cursor.execute("select sys_context(:1, :2) from dual",
                    (namespace, name))
            actual_value, = cursor.fetchone()
            self.assertEqual(actual_value, value)

    def test_1102_app_context_negative(self):
        "1102 - test invalid use of application context"
        self.assertRaises(TypeError, oracledb.connect,
                          user=test_env.get_main_user(),
                          password=test_env.get_main_password(),
                          dsn=test_env.get_connect_string(),
                          appcontext=[('userenv', 'action')])

    def test_1103_attributes(self):
        "1103 - test connection end-to-end tracing attributes"
        connection = test_env.get_connection()
        if test_env.get_client_version() >= (12, 1) \
                and not self.is_on_oracle_cloud(connection):
            sql = "select dbop_name from v$sql_monitor " \
                  "where sid = sys_context('userenv', 'sid')" \
                  "and status = 'EXECUTING'"
            self.__verify_attributes(connection, "dbop", "oracledb_dbop", sql)
        sql = "select sys_context('userenv', 'action') from dual"
        self.__verify_attributes(connection, "action", "oracledb_Action", sql)
        sql = "select sys_context('userenv', 'module') from dual"
        self.__verify_attributes(connection, "module", "oracledb_Module", sql)
        sql = "select sys_context('userenv', 'client_info') from dual"
        self.__verify_attributes(connection, "clientinfo", "oracledb_cinfo",
                                 sql)
        sql = "select sys_context('userenv', 'client_identifier') from dual"
        self.__verify_attributes(connection, "client_identifier",
                                 "oracledb_cid", sql)

    def test_1104_autocommit(self):
        "1104 - test use of autocommit"
        connection = test_env.get_connection()
        cursor = connection.cursor()
        other_connection = test_env.get_connection()
        other_cursor = other_connection.cursor()
        cursor.execute("truncate table TestTempTable")
        cursor.execute("insert into TestTempTable (IntCol) values (1)")
        other_cursor.execute("select IntCol from TestTempTable")
        rows = other_cursor.fetchall()
        self.assertEqual(rows, [])
        connection.autocommit = True
        cursor.execute("insert into TestTempTable (IntCol) values (2)")
        other_cursor.execute("select IntCol from TestTempTable order by IntCol")
        rows = other_cursor.fetchall()
        self.assertEqual(rows, [(1,), (2,)])

    def test_1105_bad_connect_string(self):
        "1105 - connection to database with bad connect string"
        self.assertRaises(oracledb.DatabaseError, oracledb.connect,
                          test_env.get_main_user())
        self.assertRaises(oracledb.DatabaseError, oracledb.connect,
                          test_env.get_main_user() + "@" + \
                          test_env.get_connect_string())
        self.assertRaises(oracledb.DatabaseError, oracledb.connect,
                          test_env.get_main_user() + "@" + \
                          test_env.get_connect_string() + "/" + \
                          test_env.get_main_password())

    def test_1106_bad_password(self):
        "1106 - connection to database with bad password"
        self.assertRaises(oracledb.DatabaseError, oracledb.connect,
                          user=test_env.get_main_user(),
                          password=test_env.get_main_password() + "X",
                          dsn=test_env.get_connect_string())

    def test_1107_change_password(self):
        "1107 - test changing password"
        connection = test_env.get_connection()
        if self.is_on_oracle_cloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        sys_random = random.SystemRandom()
        new_password = "".join(sys_random.choice(string.ascii_letters) \
                       for i in range(20))
        connection.changepassword(test_env.get_main_password(), new_password)
        connection = oracledb.connect(dsn=test_env.get_connect_string(),
                                      user=test_env.get_main_user(),
                                      password=new_password)
        connection.changepassword(new_password, test_env.get_main_password())

    def test_1108_change_password_negative(self):
        "1108 - test changing password to an invalid value"
        connection = test_env.get_connection()
        if self.is_on_oracle_cloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        new_password = "1" * 150
        self.assertRaises(oracledb.DatabaseError, connection.changepassword,
                test_env.get_main_password(), new_password)

    def test_1109_parse_password(self):
        "1109 - test connecting with password containing / and @ symbols"
        connection = test_env.get_connection()
        if self.is_on_oracle_cloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        sys_random = random.SystemRandom()
        chars = list(sys_random.choice(string.ascii_letters) for i in range(20))
        chars[4] = "/"
        chars[8] = "@"
        new_password = "".join(chars)
        connection.changepassword(test_env.get_main_password(), new_password)
        try:
            arg = "%s/%s@%s" % (test_env.get_main_user(), new_password,
                                test_env.get_connect_string())
            oracledb.connect(arg)
        finally:
            connection.changepassword(new_password,
                                      test_env.get_main_password())

    def test_1110_encodings(self):
        "1110 - connection with only encoding/nencoding specified should work"
        connection = oracledb.connect(test_env.get_main_user(),
                                      test_env.get_main_password(),
                                      test_env.get_connect_string())
        encoding = connection.encoding
        nencoding = connection.nencoding
        alt_encoding = "ISO-8859-1"
        connection = oracledb.connect(test_env.get_main_user(),
                                      test_env.get_main_password(),
                                      test_env.get_connect_string(),
                                      encoding=alt_encoding)
        self.assertEqual(connection.encoding, alt_encoding)
        self.assertEqual(connection.nencoding, nencoding)
        connection = oracledb.connect(test_env.get_main_user(),
                                      test_env.get_main_password(),
                                      test_env.get_connect_string(),
                                      nencoding=alt_encoding)
        self.assertEqual(connection.encoding, encoding)
        self.assertEqual(connection.nencoding, alt_encoding)

    def test_1111_different_encodings(self):
        "1111 - different encodings can be specified for encoding/nencoding"
        connection = oracledb.connect(test_env.get_main_user(),
                                      test_env.get_main_password(),
                                      test_env.get_connect_string(),
                                      encoding="UTF-8", nencoding="UTF-16")
        value = "\u03b4\u4e2a"
        cursor = connection.cursor()
        nchar_var = cursor.var(oracledb.DB_TYPE_NVARCHAR, 100)
        nchar_var.setvalue(0, value)
        cursor.execute("select :value from dual", value=nchar_var)
        result, = cursor.fetchone()
        self.assertEqual(result, value)

    def test_1112_exception_on_close(self):
        "1112 - confirm an exception is raised after closing a connection"
        connection = test_env.get_connection()
        connection.close()
        self.assertRaises(oracledb.InterfaceError, connection.rollback)

    def test_1113_connect_with_handle(self):
        "1113 - test creating a connection using a handle"
        connection = test_env.get_connection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        int_value = random.randint(1, 32768)
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:val, null)""", val=int_value)
        connection2 = oracledb.connect(handle=connection.handle)
        cursor = connection2.cursor()
        cursor.execute("select IntCol from TestTempTable")
        fetched_int_value, = cursor.fetchone()
        self.assertEqual(fetched_int_value, int_value)
        cursor.close()
        self.assertRaises(oracledb.DatabaseError, connection2.close)
        connection.close()

    def test_1114_make_dsn(self):
        "1114 - test making a data source name from host, port and sid"
        format_string = "(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)" + \
                        "(HOST=%s)(PORT=%d))(CONNECT_DATA=(SID=%s)))"
        args = ("hostname", 1521, "TEST")
        result = oracledb.makedsn(*args)
        self.assertEqual(result, format_string % args)

    def test_1115_single_arg(self):
        "1115 - connection to database with user, password, DSN together"
        arg = "%s/%s@%s" % (test_env.get_main_user(),
                            test_env.get_main_password(),
                            test_env.get_connect_string())
        connection = oracledb.connect(arg)
        self.__verify_args(connection)

    def test_1116_version(self):
        "1116 - connection version is a string"
        connection = test_env.get_connection()
        self.assertTrue(isinstance(connection.version, str))

    def test_1117_rollback_on_close(self):
        "1117 - connection rolls back before close"
        connection = test_env.get_connection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        other_connection = test_env.get_connection()
        other_cursor = other_connection.cursor()
        other_cursor.execute("insert into TestTempTable (IntCol) values (1)")
        other_cursor.close()
        other_connection.close()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def test_1118_rollback_on_del(self):
        "1118 - connection rolls back before destruction"
        connection = test_env.get_connection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        other_connection = test_env.get_connection()
        other_cursor = other_connection.cursor()
        other_cursor.execute("insert into TestTempTable (IntCol) values (1)")
        del other_cursor
        del other_connection
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def test_1119_threading(self):
        "1119 - multiple connections to database with multiple threads"
        threads = []
        for i in range(20):
            thread = threading.Thread(None, self.__connect_and_drop)
            threads.append(thread)
            thread.start()
        for thread in threads:
            thread.join()

    def test_1120_string_format(self):
        "1120 - test string format of connection"
        connection = test_env.get_connection()
        expected_value = "<cx_Oracle.Connection to %s@%s>" % \
                (test_env.get_main_user(), test_env.get_connect_string())
        self.assertEqual(str(connection), expected_value)

    def test_1121_ctx_mgr_close(self):
        "1121 - test context manager - close"
        connection = test_env.get_connection()
        with connection:
            cursor = connection.cursor()
            cursor.execute("truncate table TestTempTable")
            cursor.execute("insert into TestTempTable (IntCol) values (1)")
            connection.commit()
            cursor.execute("insert into TestTempTable (IntCol) values (2)")
        self.assertRaises(oracledb.InterfaceError, connection.ping)
        connection = test_env.get_connection()
        cursor = connection.cursor()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 1)

    def test_1122_connection_attributes(self):
        "1122 - test connection attribute values"
        connection = oracledb.connect(test_env.get_main_user(),
                                      test_env.get_main_password(),
                                      test_env.get_connect_string(),
                                      encoding="ASCII")
        self.assertEqual(connection.maxBytesPerCharacter, 1)
        connection = oracledb.connect(test_env.get_main_user(),
                                      test_env.get_main_password(),
                                      test_env.get_connect_string(),
                                      encoding="UTF-8")
        self.assertEqual(connection.maxBytesPerCharacter, 4)
        if test_env.get_client_version() >= (12, 1):
            self.assertEqual(connection.ltxid, b'')
        self.assertEqual(connection.current_schema, None)
        connection.current_schema = "test_schema"
        self.assertEqual(connection.current_schema, "test_schema")
        self.assertEqual(connection.edition, None)
        connection.external_name = "test_external"
        self.assertEqual(connection.external_name, "test_external")
        connection.internal_name = "test_internal"
        self.assertEqual(connection.internal_name, "test_internal")
        connection.stmtcachesize = 30
        self.assertEqual(connection.stmtcachesize, 30)
        self.assertRaises(TypeError, connection.stmtcachesize, 20.5)
        self.assertRaises(TypeError, connection.stmtcachesize, "value")

    def test_1123_closed_connection_attributes(self):
        "1123 - test closed connection attribute values"
        connection = test_env.get_connection()
        connection.close()
        attr_names = ["current_schema", "edition", "external_name",
                      "internal_name", "stmtcachesize"]
        if test_env.get_client_version() >= (12, 1):
            attr_names.append("ltxid")
        for name in attr_names:
            self.assertRaises(oracledb.InterfaceError, getattr, connection,
                    name)

    def test_1124_ping(self):
        "1124 - test connection ping"
        connection = test_env.get_connection()
        connection.ping()

    def test_1125_transaction_begin(self):
        "1125 - test begin, prepare, cancel transaction"
        connection = test_env.get_connection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        connection.begin(10, 'trxnId', 'branchId')
        self.assertEqual(connection.prepare(), False)
        connection.begin(10, 'trxnId', 'branchId')
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (1, 'tesName')""")
        self.assertEqual(connection.prepare(), True)
        connection.cancel()
        connection.rollback()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def test_1126_multiple_transactions(self):
        "1126 - test multiple transactions on the same connection"
        connection = test_env.get_connection()
        with connection.cursor() as cursor:
            cursor.execute("truncate table TestTempTable")

        id_ = random.randint(0, 2 ** 128)
        xid = (0x1234, "%032x" % id_, "%032x" % 9)
        connection.begin(*xid)
        with connection.cursor() as cursor:
            cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (1, 'tesName')""")
            self.assertEqual(connection.prepare(), True)
            connection.commit()

        for begin_trans in (True, False):
            val = 3
            if begin_trans:
                connection.begin()
                val = 2
            with connection.cursor() as cursor:
                cursor.execute("""
                        insert into TestTempTable (IntCol, StringCol)
                        values (:int_val, 'tesName')""", int_val=val)
                connection.commit()

        expected_rows = [(1, "tesName"), (2, "tesName"), (3, "tesName")]
        with connection.cursor() as cursor:
            cursor.execute("select IntCol, StringCol from TestTempTable")
            self.assertEqual(cursor.fetchall(), expected_rows)

    def test_1127_multiple_global_transactions(self):
        "1127 - test multiple global transactions on the same connection"
        connection = test_env.get_connection()
        with connection.cursor() as cursor:
            cursor.execute("truncate table TestTempTable")

        id_ = random.randint(0, 2 ** 128)
        xid = (0x1234, "%032x" % id_, "%032x" % 9)
        connection.begin(*xid)
        with connection.cursor() as cursor:
            cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (1, 'tesName')""")
            self.assertEqual(connection.prepare(), True)
            connection.commit()

        for begin_trans in (True, False):
            val = 3
            if begin_trans:
                connection.begin()
                val = 2
            with connection.cursor() as cursor:
                cursor.execute("""
                        insert into TestTempTable (IntCol, StringCol)
                        values (:int_val, 'tesName')""", int_val=val)
                connection.commit()

        id2_ = random.randint(0, 2 ** 128)
        xid2 = (0x1234, "%032x" % id2_, "%032x" % 9)
        connection.begin(*xid2)
        with connection.cursor() as cursor:
            cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (4, 'tesName')""")
            self.assertEqual(connection.prepare(), True)
            connection.commit()

        expected_rows = [(1, "tesName"), (2, "tesName"), (3, "tesName"),
                         (4, "tesName")]

        with connection.cursor() as cursor:
            cursor.execute("select IntCol, StringCol from TestTempTable")
            self.assertEqual(cursor.fetchall(), expected_rows)

    def test_1128_exception_creating_global_txn_after_local_txn(self):
        "1128 - test creating global txn after a local txn"
        connection = test_env.get_connection()
        with connection.cursor() as cursor:
            cursor.execute("truncate table TestTempTable")

        val = 2
        with connection.cursor() as cursor:
            cursor.execute("""
                    insert into TestTempTable (IntCol, StringCol)
                    values (:int_val, 'tesName')""", int_val=val)

        id_ = random.randint(0, 2 ** 128)
        xid = (0x1234, "%032x" % id_, "%032x" % 9)
        self.assertRaises(oracledb.DatabaseError, connection.begin, *xid)

    def test_1129_threading_single_connection(self):
        "1129 - single connection to database with multiple threads"
        with test_env.get_connection(threaded=True) as connection:
            threads = [threading.Thread(target=self.__verify_fetched_data,
                                        args=(connection,)) for i in range(3)]
            for t in threads:
                t.start()
            for t in threads:
                t.join()

    def test_1130_cancel(self):
        "1130 - test connection cancel"
        conn = test_env.get_connection()
        def perform_cancel():
            time.sleep(0.1)
            conn.cancel()
        thread = threading.Thread(target=perform_cancel)
        thread.start()
        try:
            with conn.cursor() as cursor:
                self.assertRaises(oracledb.OperationalError, cursor.callproc,
                                  test_env.get_sleep_proc_name(), [2])
        finally:
            thread.join()
        with conn.cursor() as cursor:
            cursor.execute("select user from dual")
            user, = cursor.fetchone()
            self.assertEqual(user, test_env.get_main_user().upper())

    def test_1131_change_password_during_connect(self):
        "1131 - test changing password during connect"
        connection = test_env.get_connection()
        if self.is_on_oracle_cloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        sys_random = random.SystemRandom()
        new_password = "".join(sys_random.choice(string.ascii_letters) \
                       for i in range(20))
        connection = oracledb.connect(dsn=test_env.get_connect_string(),
                                      user=test_env.get_main_user(),
                                      password=test_env.get_main_password(),
                                      newpassword=new_password)
        connection = oracledb.connect(dsn=test_env.get_connect_string(),
                                      user=test_env.get_main_user(),
                                      password=new_password)
        connection.changepassword(new_password, test_env.get_main_password())

    def test_1132_autocommit_during_reexecute(self):
        "1132 - test use of autocommit during reexecute"
        sql = "insert into TestTempTable (IntCol, StringCol) values (:1, :2)"
        data_to_insert = [
            (1, "Test String #1"),
            (2, "Test String #2")
        ]
        connection = test_env.get_connection()
        cursor = connection.cursor()
        other_connection = test_env.get_connection()
        other_cursor = other_connection.cursor()
        cursor.execute("truncate table TestTempTable")
        cursor.execute(sql, data_to_insert[0])
        other_cursor.execute("select IntCol, StringCol from TestTempTable")
        rows = other_cursor.fetchall()
        self.assertEqual(rows, [])
        connection.autocommit = True
        cursor.execute(sql, data_to_insert[1])
        other_cursor.execute("select IntCol, StringCol from TestTempTable")
        rows = other_cursor.fetchall()
        self.assertEqual(rows, data_to_insert)

    def test_1133_current_schema(self):
        "1133 - test current_schame is set properly"
        conn = test_env.get_connection()
        self.assertEqual(conn.current_schema, None)

        user = test_env.get_main_user().upper()
        proxy_user = test_env.get_proxy_user().upper()
        cursor = conn.cursor()
        cursor.execute(f'alter session set current_schema={proxy_user}')
        self.assertEqual(conn.current_schema, proxy_user)

        conn.current_schema = user
        self.assertEqual(conn.current_schema, user)

        cursor.execute("""
                select sys_context('userenv', 'current_schema')
                from dual""")
        result, = cursor.fetchone()
        self.assertEqual(result, user)

    def test_1134_dbms_output(self):
        "1134 - test dbms_output package"
        conn = test_env.get_connection()
        cursor = conn.cursor()
        test_string = "Testing DBMS_OUTPUT package"
        cursor.callproc("dbms_output.enable")
        cursor.execute("""
                begin
                    dbms_output.put_line(:val);
                end; """, val=test_string)
        string_var = cursor.var(str)
        number_var = cursor.var(int)
        cursor.callproc("dbms_output.get_line", (string_var, number_var))
        self.assertEqual(string_var.getvalue(), test_string)

    @unittest.skipIf(test_env.get_client_version() < (18, 1),
                     "unsupported client")
    def test_1135_calltimeout(self):
        "1135 - test connection call_timeout"
        conn = test_env.get_connection()
        conn.call_timeout = 500   # milliseconds
        self.assertEqual(conn.call_timeout, 500)
        self.assertRaises(oracledb.DatabaseError, conn.cursor().callproc,
                          test_env.get_sleep_proc_name(), [2])

if __name__ == "__main__":
    test_env.run_test_cases()
