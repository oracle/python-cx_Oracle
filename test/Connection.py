#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing connections."""

import random
import threading

class TestConnection(TestCase):

    def __ConnectAndDrop(self):
        """Connect to the database, perform a query and drop the connection."""
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry, threaded = True)
        cursor = connection.cursor()
        cursor.execute(u"select count(*) from TestNumbers")
        count, = cursor.fetchone()
        self.assertEqual(count, 10)

    def __VerifyAttributes(self, connection, attrName, value, sql):
        setattr(connection, attrName, value)
        cursor = connection.cursor()
        cursor.execute(sql)
        result, = cursor.fetchone()
        self.assertEqual(result, value, "%s value mismatch" % attrName)

    def setUp(self):
        self.username = USERNAME
        self.password = PASSWORD
        self.tnsentry = TNSENTRY

    def verifyArgs(self, connection):
        self.assertEqual(connection.username, self.username,
                "user name differs")
        self.assertEqual(connection.tnsentry, self.tnsentry,
                "tnsentry differs")
        self.assertEqual(connection.dsn, self.tnsentry, "dsn differs")

    def testAllArgs(self):
        "connection to database with user, password, TNS separate"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        self.verifyArgs(connection)

    def testAttributes(self):
        "test connection end-to-end tracing attributes"
        connection = cx_Oracle.connect(USERNAME, PASSWORD, TNSENTRY)
        self.__VerifyAttributes(connection, "dbop", "cx_OracleTest_DBOP",
                "select dbop_name from v$sql_monitor "
                "where sid = sys_context('userenv', 'sid')")
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

    def testAutoCommit(self):
        "test use of autocommit"
        connection = cx_Oracle.connect(USERNAME, PASSWORD, TNSENTRY)
        cursor = connection.cursor()
        otherConnection = cx_Oracle.connect(USERNAME, PASSWORD, TNSENTRY)
        otherCursor = otherConnection.cursor()
        cursor.execute("truncate table TestTempTable")
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (1, null)""")
        otherCursor.execute("select * from TestTempTable")
        rows = otherCursor.fetchall()
        self.assertEqual(rows, [])
        connection.autocommit = True
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (2, null)""")
        otherCursor.execute("select * from TestTempTable order by IntCol")
        rows = otherCursor.fetchall()
        self.assertEqual(rows, [(1, None), (2, None)])

    def testBadConnectString(self):
        "connection to database with bad connect string"
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                self.username)
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                self.username + u"@" + self.tnsentry)
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                self.username + "@" + self.tnsentry + "/" + self.password)

    def testBadPassword(self):
        "connection to database with bad password"
        self.assertRaises(cx_Oracle.DatabaseError, cx_Oracle.connect,
                self.username, self.password + u"X", self.tnsentry)

    def testChangePassword(self):
        "test changing password"
        newPassword = "NEW_PASSWORD"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        connection.changepassword(self.password, newPassword)
        cconnection = cx_Oracle.connect(self.username, newPassword,
                self.tnsentry)
        connection.changepassword(newPassword, self.password)

    def testEncodings(self):
        "connection with only encoding or nencoding specified should work"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        encoding = connection.encoding
        nencoding = connection.nencoding
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry, encoding = "UTF-8")
        self.assertEqual(connection.encoding, "UTF-8")
        self.assertEqual(connection.nencoding, nencoding)
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry, nencoding = "UTF-8")
        self.assertEqual(connection.encoding, encoding)
        self.assertEqual(connection.nencoding, "UTF-8")

    def testDifferentEncodings(self):
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry, encoding = "UTF-8", nencoding = "UTF-16")
        value = u"\u03b4\u4e2a"
        cursor = connection.cursor()
        ncharVar = cursor.var(cx_Oracle.NCHAR, 100)
        ncharVar.setvalue(0, value)
        cursor.execute("select :value from dual", value = ncharVar)
        result, = cursor.fetchone()
        self.assertEqual(result, value)

    def testExceptionOnClose(self):
        "confirm an exception is raised after closing a connection"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        connection.close()
        self.assertRaises(cx_Oracle.DatabaseError, connection.rollback)

    def testConnectWithHandle(self):
        "test creating a connection using a handle"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        intValue = random.randint(1, 32768)
        cursor.execute("insert into TestTempTable values (:val, null)",
                val = intValue)
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

    def testMakeDSN(self):
        "test making a data source name from host, port and sid"
        formatString = u"(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)" + \
                "(HOST=%s)(PORT=%d))(CONNECT_DATA=(SID=%s)))"
        args = ("hostname", 1521, "TEST")
        result = cx_Oracle.makedsn(*args)
        self.assertEqual(result, formatString % args)
        args = (u"hostname", 1521, u"TEST")
        result = cx_Oracle.makedsn(*args)
        self.assertEqual(result, formatString % args)

    def testSingleArg(self):
        "connection to database with user, password, TNS together"
        connection = cx_Oracle.connect("%s/%s@%s" % \
                (self.username, self.password, self.tnsentry))
        self.verifyArgs(connection)

    def testVersion(self):
        "connection version is a string"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        self.assertTrue(isinstance(connection.version, str))

    def testRollbackOnClose(self):
        "connection rolls back before close"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        otherConnection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        otherCursor = otherConnection.cursor()
        otherCursor.execute("insert into TestTempTable (IntCol) values (1)")
        otherCursor.close()
        otherConnection.close()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def testRollbackOnDel(self):
        "connection rolls back before destruction"
        connection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        otherConnection = cx_Oracle.connect(self.username, self.password,
                self.tnsentry)
        otherCursor = otherConnection.cursor()
        otherCursor.execute("insert into TestTempTable (IntCol) values (1)")
        del otherCursor
        del otherConnection
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)

    def testThreading(self):
        "connection to database with multiple threads"
        threads = []
        for i in range(20):
            thread = threading.Thread(None, self.__ConnectAndDrop)
            threads.append(thread)
            thread.start()
        for thread in threads:
            thread.join()

