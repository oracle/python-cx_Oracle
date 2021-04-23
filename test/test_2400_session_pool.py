#------------------------------------------------------------------------------
# Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""
2400 - Module for testing session pools
"""

import test_env

import cx_Oracle as oracledb
import threading

class TestCase(test_env.BaseTestCase):
    require_connection = False

    def __connect_and_drop(self):
        with self.pool.acquire() as connection:
            cursor = connection.cursor()
            cursor.execute("select count(*) from TestNumbers")
            count, = cursor.fetchone()
            self.assertEqual(count, 10)

    def __connect_and_generate_error(self):
        with self.pool.acquire() as connection:
            cursor = connection.cursor()
            self.assertRaises(oracledb.DatabaseError, cursor.execute,
                            "select 1 / 0 from dual")

    def __verify_connection(self, connection, expected_user,
                            expected_proxy_user=None):
        cursor = connection.cursor()
        cursor.execute("""
                select
                    sys_context('userenv', 'session_user'),
                    sys_context('userenv', 'proxy_user')
                from dual""")
        actual_user, actual_proxy_user = cursor.fetchone()
        self.assertEqual(actual_user, expected_user.upper())
        self.assertEqual(actual_proxy_user,
                         expected_proxy_user and expected_proxy_user.upper())

    def test_2400_pool(self):
        "2400 - test that the pool is created and has the right attributes"
        pool = test_env.get_pool(min=2, max=8, increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        self.assertEqual(pool.username, test_env.get_main_user(),
                         "user name differs")
        self.assertEqual(pool.tnsentry, test_env.get_connect_string(),
                         "tnsentry differs")
        self.assertEqual(pool.dsn, test_env.get_connect_string(),
                         "dsn differs")
        self.assertEqual(pool.max, 8, "max differs")
        self.assertEqual(pool.min, 2, "min differs")
        self.assertEqual(pool.increment, 3, "increment differs")
        self.assertEqual(pool.busy, 0, "busy not 0 at start")
        connection_1 = pool.acquire()
        self.assertEqual(pool.busy, 1, "busy not 1 after acquire")
        connection_2 = pool.acquire()
        self.assertEqual(pool.busy, 2, "busy not 2 after acquire")
        self.assertEqual(pool.opened, 2, "opened differs")
        connection_3 = pool.acquire()
        self.assertEqual(pool.busy, 3, "busy not 3 after acquire")
        pool.release(connection_3)
        self.assertEqual(pool.busy, 2, "busy not 2 after release")
        pool.release(connection_1)
        pool.release(connection_2)
        self.assertEqual(pool.busy, 0, "busy not 0 after release")
        pool.getmode = oracledb.SPOOL_ATTRVAL_NOWAIT
        self.assertEqual(pool.getmode, oracledb.SPOOL_ATTRVAL_NOWAIT)
        if test_env.get_client_version() >= (12, 2):
            pool.getmode = oracledb.SPOOL_ATTRVAL_TIMEDWAIT
            self.assertEqual(pool.getmode, oracledb.SPOOL_ATTRVAL_TIMEDWAIT)
        pool.stmtcachesize = 50
        self.assertEqual(pool.stmtcachesize, 50)
        pool.timeout = 10
        self.assertEqual(pool.timeout, 10)
        if test_env.get_client_version() >= (12, 1):
            pool.max_lifetime_session = 10
            self.assertEqual(pool.max_lifetime_session, 10)

    def test_2401_proxy_auth(self):
        "2401 - test that proxy authentication is possible"
        pool = test_env.get_pool(min=2, max=8, increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        self.assertEqual(pool.homogeneous, True,
                         "homogeneous should be True by default")
        self.assertRaises(oracledb.DatabaseError, pool.acquire,
                          user="missing_proxyuser")
        pool = test_env.get_pool(min=2, max=8, increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT,
                                 homogeneous=False)
        msg = "homogeneous should be False after setting it in the constructor"
        self.assertEqual(pool.homogeneous, False, msg)
        connection = pool.acquire(user=test_env.get_proxy_user())
        cursor = connection.cursor()
        cursor.execute('select user from dual')
        result, = cursor.fetchone()
        self.assertEqual(result, test_env.get_proxy_user().upper())
        connection.close()

    def test_2403_rollback_on_release(self):
        "2403 - connection rolls back before released back to the pool"
        pool = test_env.get_pool(getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        connection = pool.acquire()
        cursor = connection.cursor()
        cursor.execute("truncate table TestTempTable")
        cursor.execute("insert into TestTempTable (IntCol) values (1)")
        cursor.close()
        pool.release(connection)
        pool = test_env.get_pool(getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        connection = pool.acquire()
        cursor = connection.cursor()
        cursor.execute("select count(*) from TestTempTable")
        count, = cursor.fetchone()
        self.assertEqual(count, 0)
        connection.close()

    def test_2404_threading(self):
        "2404 - test session pool with multiple threads"
        self.pool = test_env.get_pool(min=5, max=20, increment=2,
                                      threaded=True,
                                      getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        threads = []
        for i in range(20):
            thread = threading.Thread(None, self.__connect_and_drop)
            threads.append(thread)
            thread.start()
        for thread in threads:
            thread.join()

    def test_2405_threading_with_errors(self):
        "2405 - test session pool with multiple threads (with errors)"
        self.pool = test_env.get_pool(min=5, max=20, increment=2,
                                      threaded=True,
                                      getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        threads = []
        for i in range(20):
            thread = threading.Thread(None, self.__connect_and_generate_error)
            threads.append(thread)
            thread.start()
        for thread in threads:
            thread.join()

    def test_2406_purity(self):
        "2406 - test session pool with various types of purity"
        pool = test_env.get_pool(min=1, max=8, increment=1,
                             getmode=oracledb.SPOOL_ATTRVAL_WAIT)

        # get connection and set the action
        action = "TEST_ACTION"
        connection = pool.acquire()
        connection.action = action
        cursor = connection.cursor()
        cursor.execute("select 1 from dual")
        cursor.close()
        pool.release(connection)
        self.assertEqual(pool.opened, 1, "opened (1)")

        # verify that the connection still has the action set on it
        connection = pool.acquire()
        cursor = connection.cursor()
        cursor.execute("select sys_context('userenv', 'action') from dual")
        result, = cursor.fetchone()
        self.assertEqual(result, action)
        cursor.close()
        pool.release(connection)
        self.assertEqual(pool.opened, 1, "opened (2)")

        # get a new connection with new purity (should not have state)
        connection = pool.acquire(purity=oracledb.ATTR_PURITY_NEW)
        cursor = connection.cursor()
        cursor.execute("select sys_context('userenv', 'action') from dual")
        result, = cursor.fetchone()
        self.assertEqual(result, None)
        cursor.close()
        pool.release(connection)

    def test_2407_heterogeneous(self):
        "2407 - test heterogeneous pool with user and password specified"
        pool = test_env.get_pool(min=2, max=8, increment=3, homogeneous=False,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        self.assertEqual(pool.homogeneous, 0)
        conn = pool.acquire()
        self.__verify_connection(pool.acquire(), test_env.get_main_user())
        conn.close()
        conn = pool.acquire(test_env.get_main_user(),
                            test_env.get_main_password())
        self.__verify_connection(conn, test_env.get_main_user())
        conn.close()
        conn = pool.acquire(test_env.get_proxy_user(),
                            test_env.get_proxy_password())
        self.__verify_connection(conn, test_env.get_proxy_user())
        conn.close()
        user_str = "%s[%s]" % \
                (test_env.get_main_user(), test_env.get_proxy_user())
        conn = pool.acquire(user_str, test_env.get_main_password())
        self.__verify_connection(conn, test_env.get_proxy_user(),
                                 test_env.get_main_user())
        conn.close()

    def test_2408_heterogenous_without_user(self):
        "2408 - test heterogeneous pool without user and password specified"
        pool = test_env.get_pool(user="", password="", min=2, max=8,
                                 increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT,
                                 homogeneous=False)
        conn = pool.acquire(test_env.get_main_user(),
                            test_env.get_main_password())
        self.__verify_connection(conn, test_env.get_main_user())
        conn.close()
        conn = pool.acquire(test_env.get_proxy_user(),
                            test_env.get_proxy_password())
        self.__verify_connection(conn, test_env.get_proxy_user())
        conn.close()
        user_str = "%s[%s]" % \
                (test_env.get_main_user(), test_env.get_proxy_user())
        conn = pool.acquire(user_str, test_env.get_main_password())
        self.__verify_connection(conn, test_env.get_proxy_user(),
                                 test_env.get_main_user())

    def test_2409_heterogeneous_wrong_password(self):
        "2409 - test heterogeneous pool with wrong password specified"
        pool = test_env.get_pool(min=2, max=8, increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT,
                                 homogeneous=False)
        self.assertRaises(oracledb.DatabaseError, pool.acquire,
                          test_env.get_proxy_user(),
                          "this is the wrong password")

    def test_2410_tagging_session(self):
        "2410 - test tagging a session"
        pool = test_env.get_pool(min=2, max=8, increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_NOWAIT)

        tag_mst = "TIME_ZONE=MST"
        tag_utc = "TIME_ZONE=UTC"

        conn = pool.acquire()
        self.assertEqual(conn.tag, None)
        pool.release(conn, tag=tag_mst)

        conn = pool.acquire()
        self.assertEqual(conn.tag, None)
        conn.tag = tag_utc
        conn.close()

        conn = pool.acquire(tag=tag_mst)
        self.assertEqual(conn.tag, tag_mst)
        conn.close()

        conn = pool.acquire(tag=tag_utc)
        self.assertEqual(conn.tag, tag_utc)
        conn.close()

    def test_2411_plsql_session_callbacks(self):
        "2411 - test PL/SQL session callbacks"
        if test_env.get_client_version() < (12, 2):
            self.skipTest("PL/SQL session callbacks not supported before 12.2")
        callback = "pkg_SessionCallback.TheCallback"
        pool = test_env.get_pool(min=2, max=8, increment=3,
                                 getmode=oracledb.SPOOL_ATTRVAL_NOWAIT,
                                 session_callback=callback)
        tags = [
            "NLS_DATE_FORMAT=SIMPLE",
            "NLS_DATE_FORMAT=FULL;TIME_ZONE=UTC",
            "NLS_DATE_FORMAT=FULL;TIME_ZONE=MST"
        ]
        actual_tags = [None, None, "NLS_DATE_FORMAT=FULL;TIME_ZONE=UTC"]

        # truncate PL/SQL session callback log
        conn = pool.acquire()
        cursor = conn.cursor()
        cursor.execute("truncate table PLSQLSessionCallbacks")
        conn.close()

        # request sessions with each of the first two tags
        for tag in tags[:2]:
            conn = pool.acquire(tag=tag)
            conn.close()

        # for the last tag, use the matchanytag flag
        conn = pool.acquire(tag=tags[2], matchanytag=True)
        conn.close()

        # verify the PL/SQL session callback log is accurate
        conn = pool.acquire()
        cursor = conn.cursor()
        cursor.execute("""
                select RequestedTag, ActualTag
                from PLSQLSessionCallbacks
                order by FixupTimestamp""")
        results = cursor.fetchall()
        expected_results = list(zip(tags, actual_tags))
        self.assertEqual(results, expected_results)
        conn.close()

    def test_2412_tagging_invalid_key(self):
        "2412 - testTagging with Invalid key"
        pool = test_env.get_pool(getmode=oracledb.SPOOL_ATTRVAL_NOWAIT)
        conn = pool.acquire()
        self.assertRaises(TypeError, pool.release, conn, tag=12345)
        if test_env.get_client_version() >= (12, 2):
            self.assertRaises(oracledb.DatabaseError, pool.release, conn,
                              tag="INVALID_TAG")

    def test_2413_close_and_drop_connection_from_pool(self):
        "2413 - test dropping/closing a connection from the pool"
        pool = test_env.get_pool(min=1, max=8, increment=1,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        conn = pool.acquire()
        self.assertEqual(pool.busy, 1, "busy (1)")
        self.assertEqual(pool.opened, 1, "opened (1)")
        pool.drop(conn)
        self.assertEqual(pool.busy, 0, "busy (2)")
        self.assertEqual(pool.opened, 0, "opened (2)")
        conn = pool.acquire()
        self.assertEqual(pool.busy, 1, "busy (3)")
        self.assertEqual(pool.opened, 1, "opened (3)")
        conn.close()
        self.assertEqual(pool.busy, 0, "busy (4)")
        self.assertEqual(pool.opened, 1, "opened (4)")

    def test_2414_create_new_pure_connection(self):
        "2414 - test to ensure pure connections and being created correctly"
        pool = test_env.get_pool(min=1, max=2, increment=1,
                                 getmode=oracledb.SPOOL_ATTRVAL_WAIT)
        connection_1 = pool.acquire()
        connection_2 = pool.acquire()
        self.assertEqual(pool.opened, 2, "opened (1)")
        pool.release(connection_1)
        pool.release(connection_2)
        connection_3 = pool.acquire(purity=oracledb.ATTR_PURITY_NEW)
        self.assertEqual(pool.opened, 2, "opened (2)")
        pool.release(connection_3)

if __name__ == "__main__":
    test_env.run_test_cases()
