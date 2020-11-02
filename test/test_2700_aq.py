#------------------------------------------------------------------------------
# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
2700 - Module for testing AQ
"""

import TestEnv

import cx_Oracle
import decimal
import threading

class TestCase(TestEnv.BaseTestCase):
    bookQueueName = "TEST_BOOK_QUEUE"
    bookData = [
            ("Wings of Fire", "A.P.J. Abdul Kalam",
                    decimal.Decimal("15.75")),
            ("The Story of My Life", "Hellen Keller",
                    decimal.Decimal("10.50")),
            ("The Chronicles of Narnia", "C.S. Lewis",
                    decimal.Decimal("25.25"))
    ]

    def __clearBooksQueue(self):
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = self.connection.deqoptions()
        options.wait = cx_Oracle.DEQ_NO_WAIT
        options.deliverymode = cx_Oracle.MSG_PERSISTENT_OR_BUFFERED
        options.visibility = cx_Oracle.ENQ_IMMEDIATE
        props = self.connection.msgproperties()
        while self.connection.deq(self.bookQueueName, options, props, book):
            pass

    def __deqInThread(self, results):
        connection = TestEnv.GetConnection()
        booksType = connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = connection.deqoptions()
        options.wait = 10
        props = connection.msgproperties()
        if connection.deq(self.bookQueueName, options, props, book):
            results.append((book.TITLE, book.AUTHORS, book.PRICE))
        connection.commit()

    def __verifyAttribute(self, obj, attrName, value):
        setattr(obj, attrName, value)
        self.assertEqual(getattr(obj, attrName), value)

    def test_2700_DeqEmpty(self):
        "2700 - test dequeuing an empty queue"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = self.connection.deqoptions()
        options.wait = cx_Oracle.DEQ_NO_WAIT
        props = self.connection.msgproperties()
        messageId = self.connection.deq(self.bookQueueName, options, props,
                book)
        self.assertTrue(messageId is None)

    def test_2701_DeqEnq(self):
        "2701 - test enqueuing and dequeuing multiple messages"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        for title, authors, price in self.bookData:
            book = booksType.newobject()
            book.TITLE = title
            book.AUTHORS = authors
            book.PRICE = price
            self.connection.enq(self.bookQueueName, options, props, book)
        options = self.connection.deqoptions()
        options.navigation = cx_Oracle.DEQ_FIRST_MSG
        options.wait = cx_Oracle.DEQ_NO_WAIT
        results = []
        while self.connection.deq(self.bookQueueName, options, props, book):
            row = (book.TITLE, book.AUTHORS, book.PRICE)
            results.append(row)
        self.connection.commit()
        self.assertEqual(results, self.bookData)

    def test_2702_DeqModeRemoveNoData(self):
        "2702 - test dequeuing with DEQ_REMOVE_NODATA option"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        title, authors, price = self.bookData[1]
        book.TITLE = title
        book.AUTHORS = authors
        book.PRICE = price
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, options, props, book)
        options = self.connection.deqoptions()
        options.navigation = cx_Oracle.DEQ_FIRST_MSG
        options.wait = cx_Oracle.DEQ_NO_WAIT
        options.mode = cx_Oracle.DEQ_REMOVE_NODATA
        book = booksType.newobject()
        messageId = self.connection.deq(self.bookQueueName, options, props,
                book)
        self.connection.commit()
        self.assertTrue(messageId is not None)
        self.assertEqual(book.TITLE, "")

    def test_2703_DeqOptions(self):
        "2703 - test getting/setting dequeue options attributes"
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
        self.__verifyAttribute(options, "msgid", b'mID')

    def test_2704_DeqWithWait(self):
        "2704 - test waiting for dequeue"
        self.__clearBooksQueue()
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
        self.connection.enq(self.bookQueueName, options, props, book)
        self.connection.commit()
        thread.join()
        self.assertEqual(results, [(title, authors, price)])

    def test_2705_EnqOptions(self):
        "2705 - test getting/setting enqueue options attributes"
        options = self.connection.enqoptions()
        self.__verifyAttribute(options, "visibility", cx_Oracle.ENQ_IMMEDIATE)

    def test_2706_ErrorsForInvalidValues(self):
        "2706 - test errors for invalid values for options"
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        options = self.connection.enqoptions()
        props = self.connection.msgproperties()
        self.assertRaises(TypeError, self.connection.deq, self.bookQueueName,
                options, props, book)
        options = self.connection.deqoptions()
        self.assertRaises(TypeError, self.connection.enq, self.bookQueueName,
                options, props, book)

    def test_2707_MsgProps(self):
        "2707 - test getting/setting message properties attributes"
        props = self.connection.msgproperties()
        self.__verifyAttribute(props, "correlation", "TEST_CORRELATION")
        self.__verifyAttribute(props, "delay", 60)
        self.__verifyAttribute(props, "exceptionq", "TEST_EXCEPTIONQ")
        self.__verifyAttribute(props, "expiration", 30)
        self.assertEqual(props.attempts, 0)
        self.__verifyAttribute(props, "priority", 1)
        self.__verifyAttribute(props, "msgid", b'mID')
        self.assertEqual(props.state, cx_Oracle.MSG_READY)
        self.assertEqual(props.deliverymode, 0)

    def test_2708_VisibilityModeCommit(self):
        "2708 - test enqueue visibility option - ENQ_ON_COMMIT"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        enqOptions = self.connection.enqoptions()
        enqOptions.visibility = cx_Oracle.ENQ_ON_COMMIT
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        messageId = otherConnection.deq(self.bookQueueName, deqOptions, props,
                book)
        self.assertTrue(messageId is None)
        self.connection.commit()
        messageId = otherConnection.deq(self.bookQueueName, deqOptions, props,
                book)
        self.assertTrue(messageId is not None)

    def test_2709_VisibilityModeImmediate(self):
        "2709 - test enqueue visibility option - ENQ_IMMEDIATE"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        enqOptions = self.connection.enqoptions()
        enqOptions.visibility = cx_Oracle.ENQ_IMMEDIATE
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_ON_COMMIT
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        otherConnection.deq(self.bookQueueName, deqOptions, props, book)
        results = (book.TITLE, book.AUTHORS, book.PRICE)
        otherConnection.commit()
        self.assertEqual(results, self.bookData[0])

    def test_2710_DeliveryModeSameBuffered(self):
        "2710 - test enqueue/dequeue delivery modes identical - buffered"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        enqOptions = self.connection.enqoptions()
        enqOptions.deliverymode = cx_Oracle.MSG_BUFFERED
        enqOptions.visibility = cx_Oracle.ENQ_IMMEDIATE
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.deliverymode = cx_Oracle.MSG_BUFFERED
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_IMMEDIATE
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        otherConnection.deq(self.bookQueueName, deqOptions, props, book)
        results = (book.TITLE, book.AUTHORS, book.PRICE)
        otherConnection.commit()
        self.assertEqual(results, self.bookData[0])

    def test_2711_DeliveryModeSamePersistent(self):
        "2711 - test enqueue/dequeue delivery modes identical - persistent"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        enqOptions = self.connection.enqoptions()
        enqOptions.deliverymode = cx_Oracle.MSG_PERSISTENT
        enqOptions.visibility = cx_Oracle.ENQ_IMMEDIATE
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.deliverymode = cx_Oracle.MSG_PERSISTENT
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_IMMEDIATE
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        otherConnection.deq(self.bookQueueName, deqOptions, props, book)
        results = (book.TITLE, book.AUTHORS, book.PRICE)
        otherConnection.commit()
        self.assertEqual(results, self.bookData[0])

    def test_2712_DeliveryModeSamePersistentBuffered(self):
        "2712 - test enqueue/dequeue delivery modes the same"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        enqOptions = self.connection.enqoptions()
        enqOptions.deliverymode = cx_Oracle.MSG_PERSISTENT_OR_BUFFERED
        enqOptions.visibility = cx_Oracle.ENQ_IMMEDIATE
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.deliverymode = cx_Oracle.MSG_PERSISTENT_OR_BUFFERED
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_IMMEDIATE
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        otherConnection.deq(self.bookQueueName, deqOptions, props, book)
        results = (book.TITLE, book.AUTHORS, book.PRICE)
        otherConnection.commit()
        self.assertEqual(results, self.bookData[0])

    def test_2713_DeliveryModeDifferent(self):
        "2713 - test enqueue/dequeue delivery modes different"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        enqOptions = self.connection.enqoptions()
        enqOptions.deliverymode = cx_Oracle.MSG_BUFFERED
        enqOptions.visibility = cx_Oracle.ENQ_IMMEDIATE
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.deliverymode = cx_Oracle.MSG_PERSISTENT
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_IMMEDIATE
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        messageId = otherConnection.deq(self.bookQueueName, deqOptions, props,
                book)
        self.assertTrue(messageId is None)

    def test_2714_DequeueTransformation(self):
        "2714 - test dequeue transformation"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        expectedPrice = book.PRICE + 10
        enqOptions = self.connection.enqoptions()
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)
        self.connection.commit()

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_IMMEDIATE
        deqOptions.transformation = "%s.transform2" % self.connection.username
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        otherConnection.deq(self.bookQueueName, deqOptions, props, book)
        otherPrice = book.PRICE
        self.assertEqual(otherPrice, expectedPrice)

    def test_2715_EnqueueTransformation(self):
        "2715 - test enqueue transformation"
        self.__clearBooksQueue()
        booksType = self.connection.gettype("UDT_BOOK")
        book = booksType.newobject()
        book.TITLE, book.AUTHORS, book.PRICE = self.bookData[0]
        expectedPrice = book.PRICE + 5
        enqOptions = self.connection.enqoptions()
        enqOptions.transformation = "%s.transform1" % self.connection.username
        props = self.connection.msgproperties()
        self.connection.enq(self.bookQueueName, enqOptions, props, book)
        self.connection.commit()

        otherConnection = TestEnv.GetConnection()
        deqOptions = otherConnection.deqoptions()
        deqOptions.navigation = cx_Oracle.DEQ_FIRST_MSG
        deqOptions.visibility = cx_Oracle.DEQ_IMMEDIATE
        deqOptions.wait = cx_Oracle.DEQ_NO_WAIT
        booksType = otherConnection.gettype("UDT_BOOK")
        book = booksType.newobject()
        props = otherConnection.msgproperties()
        otherConnection.deq(self.bookQueueName, deqOptions, props, book)
        otherPrice = book.PRICE
        self.assertEqual(otherPrice, expectedPrice)

if __name__ == "__main__":
    TestEnv.RunTestCases()
