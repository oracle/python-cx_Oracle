#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
1100 - Module for testing connections
"""

import TestEnv

import cx_Oracle
import random
import string
import threading

class TestCase(TestEnv.BaseTestCase):

    def __ConnectAndDrop(self):
        """Connect to the database, perform a query and drop the connection."""
        connection = TestEnv.GetConnection(threaded=True)
        cursor = connection.cursor()
        cursor.execute("select count(*) from TestNumbers")
        count, = cursor.fetchone()
        self.assertEqual(count, 10)

    def __VerifyAttributes(self, connection, attrName, value, sql):
        setattr(connection, attrName, value)
        cursor = connection.cursor()
        cursor.execute(sql)
        result, = cursor.fetchone()
        self.assertEqual(result, value, "%s value mismatch" % attrName)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def verifyArgs(self, connection):
        self.assertEqual(connection.username, TestEnv.GetMainUser(),
                "user name differs")
        self.assertEqual(connection.tnsentry, TestEnv.GetConnectString(),
                "tnsentry differs")
        self.assertEqual(connection.dsn, TestEnv.GetConnectString(),
                "dsn differs")

    def test_1100_AllArgs(self):
        "1100 - connection to database with user, password, TNS separate"
        connection = TestEnv.GetConnection()
        self.verifyArgs(connection)

    def test_1101_AppContext(self):
        "1101 - test use of application context"
        namespace = "CLIENTCONTEXT"
        appContextEntries = [
            ( namespace, "ATTR1", "VALUE1" ),
            ( namespace, "ATTR2", "VALUE2" ),
            ( namespace, "ATTR3", "VALUE3" )
        ]
        connection = TestEnv.GetConnection(appcontext=appContextEntries)
        cursor = connection.cursor()
        for namespace, name, value in appContextEntries:
            cursor.execute("select sys_context(:1, :2) from dual",
                    (namespace, name))
            actualValue, = cursor.fetchone()
            self.assertEqual(actualValue, value)

    def test_1102_AppContextNegative(self):
        "1102 - test invalid use of application context"
        self.assertRaises(TypeError, cx_Oracle.connect, TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                appcontext=[('userenv', 'action')])

    def test_1103_Attributes(self):
        "1103 - test connection end-to-end tracing attributes"
        connection = TestEnv.GetConnection()
        if TestEnv.GetClientVersion() >= (12, 1) \
                and not self.isOnOracleCloud(connection):
            self.__VerifyAttributes(connection, "dbop", "cx_OracleTest_DBOP",
                    "select dbop_name from v$sql_monitor "
                    "where sid = sys_context('userenv', 'sid')"
                    "and status = 'EXECUTING'")
        self.__VerifyAttributes(connection, "action", "cx_OracleTest_Action",
                "select sys_context('userenv', 'action') from dual")
        self.__VerifyAttributes(connection, "module", "cx_OracleTest_Module",
                "select sys_context('userenv', 'module') from dual")
        self.__VerifyAttributes(connection, "clientinfo",
                "cx_OracleTest_CInfo",
                "select sys_context('userenv', 'client_info') from dual")
        self.__VerifyAttributes(connection, "client_identifier",
                "cx_OracleTest_CID",
                "select sys_context('userenv', 'client_identifier') from dual")

    def test_1104_AutoCommit(self):
        "1104 - test use of autocommit"
        connection = TestEnv.GetConnection()
        cursor = connection.cursor()
        otherConnection = TestEnv.GetConnection()
        otherCursor = otherConnection.cursor()
        cursor.execute("truncate table TestTempTable")
        cursor.execute("insert into TestTempTable (IntCol) values (1)")
        otherCursor.execute("select IntCol from TestTempTable")
        rows = otherCursor.fetchall()
        self.assertEqual(rows, [])
        connection.autocommit = True
        cursor.execute("insert into TestTempTable (IntCol) values (2)")
        otherCursor.execute("select IntCol from TestTempTable order by IntCol")
        rows = otherCursor.fetchall()
        self.assertEqual(rows, [(1,), (2,)])

    def test_1105_BadConnectString(self):
        "1105 - connection to database with bad connect string"
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                TestEnv.GetMainUser())
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                TestEnv.GetMainUser() + "@" + TestEnv.GetConnectString())
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                TestEnv.GetMainUser() + "@" + \
                TestEnv.GetConnectString() + "/" + TestEnv.GetMainPassword())

    def test_1106_BadPassword(self):
        "1106 - connection to database with bad password"
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                TestEnv.GetMainUser(), TestEnv.GetMainPassword() + "X",
                TestEnv.GetConnectString())

    def test_1107_ChangePassword(self):
        "1107 - test changing password"
        connection = TestEnv.GetConnection()
        if self.isOnOracleCloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        sysRandom = random.SystemRandom()
        newPassword = "".join(sysRandom.choice(string.ascii_letters) \
                for i in range(20))
        connection.changepassword(TestEnv.GetMainPassword(), newPassword)
        cconnection = cx_Oracle.connect(TestEnv.GetMainUser(), newPassword,
                TestEnv.GetConnectString())
        connection.changepassword(newPassword, TestEnv.GetMainPassword())

    def test_1108_ChangePasswordNegative(self):
        "1108 - test changing password to an invalid value"
        connection = TestEnv.GetConnection()
        if self.isOnOracleCloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        newPassword = "1" * 150
        self.assertRaises(cx_Oracle.DatabaseError, connection.changepassword,
                TestEnv.GetMainPassword(), newPassword)

    def test_1109_ParsePassword(self):
        "1109 - test connecting with password containing / and @ symbols"
        connection = TestEnv.GetConnection()
        if self.isOnOracleCloud(connection):
            self.skipTest("passwords on Oracle Cloud are strictly controlled")
        sysRandom = random.SystemRandom()
        chars = list(sysRandom.choice(string.ascii_letters) for i in range(20))
        chars[4] = "/"
        chars[8] = "@"
        newPassword = "".join(chars)
        connection.changepassword(TestEnv.GetMainPassword(), newPassword)
        try:
            arg = "%s/%s@%s" % (TestEnv.GetMainUser(), newPassword,
                    TestEnv.GetConnectString())
            cx_Oracle.connect(arg)
        finally:
            connection.changepassword(newPassword, TestEnv.GetMainPassword())

    def test_1110_Encodings(self):
        "1110 - connection with only encoding/nencoding specified should work"
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString())
        encoding = connection.encoding
        nencoding = connection.nencoding
        altEncoding = "ISO-8859-1"
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                encoding=altEncoding)
        self.assertEqual(connection.encoding, altEncoding)
        self.assertEqual(connection.nencoding, nencoding)
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                nencoding=altEncoding)
        self.assertEqual(connection.encoding, encoding)
        self.assertEqual(connection.nencoding, altEncoding)

    def test_1111_DifferentEncodings(self):
        "1111 - different encodings can be specified for encoding/nencoding"
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                encoding="UTF-8", nencoding="UTF-16")
        value = "\u03b4\u4e2a"
        cursor = connection.cursor()
        ncharVar = cursor.var(cx_Oracle.DB_TYPE_NVARCHAR, 100)
        ncharVar.setvalue(0, value)
        cursor.execute("select :value from dual", value = ncharVar)
        result, = cursor.fetchone()
        self.assertEqual(result, value)

    def test_1112_ExceptionOnClose(self):
        "1112 - confirm an exception is raised after closing a connection"
        connection = TestEnv.GetConnection()
        connection.close()
        self.assertRaises(cx_Oracle.InterfaceError, connection.rollback)

    def test_1113_ConnectWithHandle(self):
        "1113 - test creating a connection using a handle"
        connection = TestEnv.GetConnection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        intValue = random.randint(1, 32768)
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (:val, null)""", val = intValue)
        connection2 = cx_Oracle.connect(handle = connection.handle)
        cursor = connection2.cursor()
        cursor.execute("select IntCol from TestTempTable")
        fetchedIntValue, = cursor.fetchone()
        self.assertEqual(fetchedIntValue, intValue)
        cursor.close()
        self.assertRaises(cx_Oracle.DatabaseError, connection2.close)
        connection.close()
        cursor = connection2.cursor()
        self.assertRaises(cx_Oracle.DatabaseError, cursor.execute,
                "select count(*) from TestTempTable")

    def test_1114_MakeDSN(self):
        "1114 - test making a data source name from host, port and sid"
        formatString = "(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)" + \
                "(HOST=%s)(PORT=%d))(CONNECT_DATA=(SID=%s)))"
        args = ("hostname", 1521, "TEST")
        result = cx_Oracle.makedsn(*args)
        self.assertEqual(result, formatString % args)

    def test_1115_SingleArg(self):
        "1115 - connection to database with user, password, DSN together"
        connection = cx_Oracle.connect("%s/%s@%s" % \
                (TestEnv.GetMainUser(), TestEnv.GetMainPassword(),
                 TestEnv.GetConnectString()))
        self.verifyArgs(connection)

    def test_1116_Version(self):
        "1116 - connection version is a string"
        connection = TestEnv.GetConnection()
        self.assertTrue(isinstance(connection.version, str))

    def test_1117_RollbackOnClose(self):
        "1117 - connection rolls back before close"
        connection = TestEnv.GetConnection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        otherConnection = TestEnv.GetConnection()
        otherCursor = otherConnection.cursor()
        otherCursor.execute("insert into TestTempTable (IntCol) values (1)")
        otherCursor.close()
        otherConnection.close()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def test_1118_RollbackOnDel(self):
        "1118 - connection rolls back before destruction"
        connection = TestEnv.GetConnection()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        otherConnection = TestEnv.GetConnection()
        otherCursor = otherConnection.cursor()
        otherCursor.execute("insert into TestTempTable (IntCol) values (1)")
        del otherCursor
        del otherConnection
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def test_1119_Threading(self):
        "1119 - connection to database with multiple threads"
        threads = []
        for i in range(20):
            thread = threading.Thread(None, self.__ConnectAndDrop)
            threads.append(thread)
            thread.start()
        for thread in threads:
            thread.join()

    def test_1120_StringFormat(self):
        "1120 - test string format of connection"
        connection = TestEnv.GetConnection()
        expectedValue = "<cx_Oracle.Connection to %s@%s>" % \
                (TestEnv.GetMainUser(), TestEnv.GetConnectString())
        self.assertEqual(str(connection), expectedValue)

    def test_1121_CtxMgrClose(self):
        "1121 - test context manager - close"
        connection = TestEnv.GetConnection()
        with connection:
            cursor = connection.cursor()
            cursor.execute("truncate table TestTempTable")
            cursor.execute("insert into TestTempTable (IntCol) values (1)")
            connection.commit()
            cursor.execute("insert into TestTempTable (IntCol) values (2)")
        self.assertRaises(cx_Oracle.InterfaceError, connection.ping)
        connection = TestEnv.GetConnection()
        cursor = connection.cursor()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 1)

    def test_1122_ConnectionAttributes(self):
        "1122 - test connection attribute values"
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                encoding="ASCII")
        self.assertEqual(connection.maxBytesPerCharacter, 1)
        connection = cx_Oracle.connect(TestEnv.GetMainUser(),
                TestEnv.GetMainPassword(), TestEnv.GetConnectString(),
                encoding="UTF-8")
        self.assertEqual(connection.maxBytesPerCharacter, 4)
        if TestEnv.GetClientVersion() >= (12, 1):
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

    def test_1123_ClosedConnectionAttributes(self):
        "1123 - test closed connection attribute values"
        connection = TestEnv.GetConnection()
        connection.close()
        attrNames = ["current_schema", "edition", "external_name",
                "internal_name", "stmtcachesize"]
        if TestEnv.GetClientVersion() >= (12, 1):
            attrNames.append("ltxid")
        for name in attrNames:
            self.assertRaises(cx_Oracle.InterfaceError, getattr, connection,
                    name)

    def test_1124_Ping(self):
        "1124 - test connection ping"
        connection = TestEnv.GetConnection()
        connection.ping()

    def test_1125_TransactionBegin(self):
        "1125 - test begin, prepare, cancel transaction"
        connection = TestEnv.GetConnection()
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

if __name__ == "__main__":
    TestEnv.RunTestCases()
