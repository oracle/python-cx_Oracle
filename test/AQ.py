#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing AQ objects."""

import cx_Oracle
import decimal
import threading

class TestAQ(BaseTestCase):
    bookData = [
            ("Wings of Fire", "A.P.J. Abdul Kalam",
                    decimal.Decimal("15.75")),
            ("The Story of My Life", "Hellen Keller",
                    decimal.Decimal("10.50")),
            ("The Chronicles of Narnia", "C.S. Lewis",
                    decimal.Decimal("25.25"))
    ]

    def __deqInThread(self, results):
        connection = cx_Oracle.connect(USERNAME, PASSWORD, TNSENTRY)
        booksType = connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = connection.deqoptions()
        options.wait = 10
        props = connection.msgproperties()
        if connection.deq("BOOKS", options, props, book):
            results.append((book.TITLE, book.AUTHORS, book.PRICE))
        connection.commit()

    def __verifyAttribute(self, obj, attrName, value):
        setattr(obj, attrName, value)
        self.assertEqual(getattr(obj, attrName), value)

    def testDeqEmpty(self):
        "test dequeuing an empty queue"
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = self.connection.deqoptions()
        options.wait = cx_Oracle.DEQ_NO_WAIT
        props = self.connection.msgproperties()
        messageId = self.connection.deq("BOOKS", options, props, book)
        self.assertEqual(messageId, None)

    def testDeqEnq(self):
        "test enqueuing and dequeuing multiple messages"
        booksType = self.connection.gettype("UDT_BOOK")
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        for title, authors, price in self.bookData:
            book = booksType.newobject()
            book.TITLE = title
            book.AUTHORS = authors
            book.PRICE = price
            self.connection.enq("BOOKS", options, props, book)
        options = self.connection.deqoptions()
        options.navigation = cx_Oracle.DEQ_FIRST_MSG
        options.wait = cx_Oracle.DEQ_NO_WAIT
        results = []
        while self.connection.deq("BOOKS", options, props, book):
            row = (book.TITLE, book.AUTHORS, book.PRICE)
            results.append(row)
        self.connection.commit()
        self.assertEqual(results, self.bookData)

    def testDeqModeRemoveNoData(self):
        "test dequeuing with DEQ_REMOVE_NODATA option"
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        title, authors, price = self.bookData[1]
        book.TITLE = title
        book.AUTHORS = authors
        book.PRICE = price
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        self.connection.enq("BOOKS", options, props, book)
        options = self.connection.deqoptions()
        options.navigation = cx_Oracle.DEQ_FIRST_MSG
        options.wait = cx_Oracle.DEQ_NO_WAIT
        options.mode = cx_Oracle.DEQ_REMOVE_NODATA
        book = booksType.newobject()
        messageId = self.connection.deq("BOOKS", options, props, book)
        self.connection.commit()
        self.assertTrue(messageId is not None)
        self.assertEqual(book.TITLE, "")

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

    def testDeqWithWait(self):
        "test waiting for dequeue"
        results = []
        thread = threading.Thread(target = self.__deqInThread,
                args = (results,))
        thread.start()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        title, authors, price = self.bookData[0]
        book.TITLE = title
        book.AUTHORS = authors
        book.PRICE = price
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        self.connection.enq("BOOKS", options, props, book)
        self.connection.commit()
        thread.join()
        self.assertEqual(results, [(title, authors, price)])

    def testEnqOptions(self):
        "test getting/setting enqueue options attributes"
        options = self.connection.enqoptions()
        self.__verifyAttribute(options, "visibility", cx_Oracle.ENQ_IMMEDIATE)

    def testErrorsForInvalidValues(self):
        "test errors for invalid values for options"
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        self.assertRaises(TypeError, self.connection.deq, "BOOKS", options,
                props, book)
        options = self.connection.deqoptions()
        self.assertRaises(TypeError, self.connection.enq, "BOOKS", options,
                props, book)

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

