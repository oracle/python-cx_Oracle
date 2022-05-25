#------------------------------------------------------------------------------
# Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
3000 - Module for testing subscriptions
"""

import threading
import unittest

import cx_Oracle as oracledb
import test_env

class SubscriptionData:

    def __init__(self, num_messages_expected):
        self.condition = threading.Condition()
        self.num_messages_expected = num_messages_expected
        self.num_messages_received = 0

    def _process_message(self, message):
        pass

    def callback_handler(self, message):
        if message.type != oracledb.EVENT_DEREG:
            self._process_message(message)
            self.num_messages_received += 1
        if message.type == oracledb.EVENT_DEREG or \
                self.num_messages_received == self.num_messages_expected:
            with self.condition:
                self.condition.notify()

    def wait_for_messages(self):
        if self.num_messages_received < self.num_messages_expected:
            with self.condition:
                self.condition.wait(10)


class AQSubscriptionData(SubscriptionData):
    pass


class DMLSubscriptionData(SubscriptionData):

    def __init__(self, num_messages_expected):
        super().__init__(num_messages_expected)
        self.table_operations = []
        self.row_operations = []
        self.rowids = []

    def _process_message(self, message):
        table, = message.tables
        self.table_operations.append(table.operation)
        for row in table.rows:
            self.row_operations.append(row.operation)
            self.rowids.append(row.rowid)


class TestCase(test_env.BaseTestCase):

    def test_3000_dml_subscription(self):
        "3000 - test subscription for insert, update, delete and truncate"

        # skip if running on the Oracle Cloud, which does not support
        # subscriptions currently
        if self.is_on_oracle_cloud():
            message = "Oracle Cloud does not support subscriptions currently"
            self.skipTest(message)

        # truncate table in order to run test in known state
        self.cursor.execute("truncate table TestTempTable")

        # expected values
        table_operations = [
            oracledb.OPCODE_INSERT,
            oracledb.OPCODE_UPDATE,
            oracledb.OPCODE_INSERT,
            oracledb.OPCODE_DELETE,
            oracledb.OPCODE_ALTER | oracledb.OPCODE_ALLROWS
        ]
        row_operations = [
            oracledb.OPCODE_INSERT,
            oracledb.OPCODE_UPDATE,
            oracledb.OPCODE_INSERT,
            oracledb.OPCODE_DELETE
        ]
        rowids = []

        # set up subscription
        data = DMLSubscriptionData(5)
        connection = test_env.get_connection(threaded=True, events=True)
        sub = connection.subscribe(callback=data.callback_handler,
                                   timeout=10, qos=oracledb.SUBSCR_QOS_ROWIDS)
        sub.registerquery("select * from TestTempTable")
        connection.autocommit = True
        cursor = connection.cursor()

        # insert statement
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (1, 'test')""")
        cursor.execute("select rowid from TestTempTable where IntCol = 1")
        rowids.extend(r for r, in cursor)

        # update statement
        cursor.execute("""
                update TestTempTable set
                    StringCol = 'update'
                where IntCol = 1""")
        cursor.execute("select rowid from TestTempTable where IntCol = 1")
        rowids.extend(r for r, in cursor)

        # second insert statement
        cursor.execute("""
                insert into TestTempTable (IntCol, StringCol)
                values (2, 'test2')""")
        cursor.execute("select rowid from TestTempTable where IntCol = 2")
        rowids.extend(r for r, in cursor)

        # delete statement
        cursor.execute("delete TestTempTable where IntCol = 2")
        rowids.append(rowids[-1])

        # truncate table
        cursor.execute("truncate table TestTempTable")

        # wait for all messages to be sent
        data.wait_for_messages()

        # verify the correct messages were sent
        self.assertEqual(data.table_operations, table_operations)
        self.assertEqual(data.row_operations, row_operations)
        self.assertEqual(data.rowids, rowids)

        # test string format of subscription object is as expected
        fmt = "<cx_Oracle.Subscription on <cx_Oracle.Connection to %s@%s>>"
        expected = fmt % \
                (test_env.get_main_user(), test_env.get_connect_string())
        self.assertEqual(str(sub), expected)

    def test_3001_deprecations(self):
        "3001 - test to verify deprecations"
        connection = test_env.get_connection(threaded=True, events=True)
        self.assertRaises(oracledb.ProgrammingError, connection.subscribe,
                          ip_address='www.oracle.in',
                          ipAddress='www.oracle.in')
        self.assertRaises(oracledb.ProgrammingError, connection.subscribe,
                          grouping_class=1, groupingClass=1)
        self.assertRaises(oracledb.ProgrammingError, connection.subscribe,
                          grouping_value=3, groupingValue=3)
        self.assertRaises(oracledb.ProgrammingError, connection.subscribe,
                          grouping_type=2, groupingType=2)
        self.assertRaises(oracledb.ProgrammingError, connection.subscribe,
                          client_initiated=True, clientInitiated=True)

    @unittest.skip("multiple subscriptions cannot be created simultaneously")
    def test_3002_aq_subscription(self):
        "3002 - test subscription for AQ"

        # create queue and clear it of all messages
        queue = self.connection.queue("TEST_RAW_QUEUE")
        queue.deqoptions.wait = oracledb.DEQ_NO_WAIT
        while queue.deqone():
            pass
        self.connection.commit()

        # set up subscription
        data = AQSubscriptionData(1)
        connection = test_env.get_connection(events=True)
        sub = connection.subscribe(namespace=oracledb.SUBSCR_NAMESPACE_AQ,
                                   name=queue.name, timeout=10,
                                   callback=data.callback_handler)

        # enqueue a message
        queue.enqone(self.connection.msgproperties(payload="Some data"))
        self.connection.commit()

        # wait for all messages to be sent
        data.wait_for_messages()

if __name__ == "__main__":
    test_env.run_test_cases()
