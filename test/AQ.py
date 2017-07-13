#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing AQ objects."""

import cx_Oracle

class TestAQ(BaseTestCase):

    def __verifyAttribute(self, obj, attrName, value):
        setattr(obj, attrName, value)
        self.assertEqual(getattr(obj, attrName), value)

    def testDeqOptions(self):
        "test getting/setting dequeue options attributes"
        options = self.connection.deqoptions()
        self.__verifyAttribute(options, "condition", "TEST_CONDITION")
        self.__verifyAttribute(options, "consumername", "TEST_CONSUMERNAME")
        self.__verifyAttribute(options, "correlation", "TEST_CORRELATION")
        self.__verifyAttribute(options, "mode", cx_Oracle.DEQ_LOCKED)
        self.__verifyAttribute(options, "navigation",
                cx_Oracle.DEQ_NEXT_TRANSACTION)
        self.__verifyAttribute(options, "transformation",
                "TEST_TRANSFORMATION")
        self.__verifyAttribute(options, "visibility", cx_Oracle.ENQ_IMMEDIATE)
        self.__verifyAttribute(options, "wait", 1287)

    def testEnqOptions(self):
        "test getting/setting enqueue options attributes"
        options = self.connection.enqoptions()
        self.__verifyAttribute(options, "visibility", cx_Oracle.ENQ_IMMEDIATE)

    def testMsgProps(self):
        "test getting/setting message properties attributes"
        props = self.connection.msgproperties()
        self.__verifyAttribute(props, "correlation", "TEST_CORRELATION")
        self.__verifyAttribute(props, "delay", 60)
        self.__verifyAttribute(props, "exceptionq", "TEST_EXCEPTIONQ")
        self.__verifyAttribute(props, "expiration", 30)
        self.assertEqual(props.attempts, 0)
        self.__verifyAttribute(props, "priority", 1)
        self.assertEqual(props.state, cx_Oracle.MSG_READY)

