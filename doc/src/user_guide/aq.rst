.. _aqusermanual:

****************************
Oracle Advanced Queuing (AQ)
****************************

`Oracle Advanced Queuing
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=ADQUE>`__ is a highly
configurable and scalable messaging feature of Oracle Database.  It has
interfaces in various languages, letting you integrate multiple tools in your
architecture.

cx_Oracle 7.2 introduced an updated interface for Oracle Advanced
Queuing.

There are Advanced Queuing examples in the `GitHub examples
<https://github.com/oracle/python-cx_Oracle/tree/main/samples>`__ directory.


Creating a Queue
================

Before being used, queues need to be created in the database, for example in
SQL*Plus:

.. code-block:: sql

    begin
        dbms_aqadm.create_queue_table('MY_QUEUE_TABLE', 'RAW');
        dbms_aqadm.create_queue('DEMO_RAW_QUEUE', 'MY_QUEUE_TABLE');
        dbms_aqadm.start_queue('DEMO_RAW_QUEUE');
    end;
    /

This examples creates a RAW queue suitable for sending string or raw bytes
messages.


Enqueuing Messages
==================

To send messages in Python you connect and get a :ref:`queue <queue>`. The
queue can be used for enqueuing, dequeuing, or both as needed.

.. code-block:: python

    queue = connection.queue("DEMO_RAW_QUEUE")

Now messages can be queued using :meth:`~Queue.enqone()`.  To send three
messages:

.. code-block:: python

    PAYLOAD_DATA = [
        "The first message",
        "The second message",
        "The third message"
    ]
    for data in PAYLOAD_DATA:
        queue.enqone(connection.msgproperties(payload=data))
    connection.commit()

Since the queue sending the messages is a RAW queue, the strings in this
example will be internally encoded to bytes using :attr:`Connection.encoding`
before being enqueued.


Dequeuing Messages
==================

Dequeuing is performed similarly. To dequeue a message call the method
:meth:`~Queue.deqone()` as shown. Note that if the message is expected to be a
string, the bytes must be decoded using :attr:`Connection.encoding`.

.. code-block:: python

    queue = connection.queue("DEMO_RAW_QUEUE")
    msg = queue.deqOne()
    connection.commit()
    print(msg.payload.decode(connection.encoding))


Using Object Queues
===================

Named Oracle objects can be enqueued and dequeued as well.  Given an object
type called ``UDT_BOOK``:

.. code-block:: sql

    CREATE OR REPLACE TYPE udt_book AS OBJECT (
        Title   VARCHAR2(100),
        Authors VARCHAR2(100),
        Price   NUMBER(5,2)
    );
    /

And a queue that accepts this type:

.. code-block:: sql

    begin
        dbms_aqadm.create_queue_table('BOOK_QUEUE_TAB', 'UDT_BOOK');
        dbms_aqadm.create_queue('DEMO_BOOK_QUEUE', 'BOOK_QUEUE_TAB');
        dbms_aqadm.start_queue('DEMO_BOOK_QUEUE');
    end;
    /

You can queue messages:

.. code-block:: python

    book_type = connection.gettype("UDT_BOOK")
    queue = connection.queue("DEMO_BOOK_QUEUE", book_type)

    book = book_type.newobject()
    book.TITLE = "Quick Brown Fox"
    book.AUTHORS = "The Dog"
    book.PRICE = 123

    queue.enqone(connection.msgproperties(payload=book))
    connection.commit()

Dequeuing is done like this:

.. code-block:: python

    book_type = connection.gettype("UDT_BOOK")
    queue = connection.queue("DEMO_BOOK_QUEUE", book_type)

    msg = queue.deqone()
    connection.commit()
    print(msg.payload.TITLE)        # will print Quick Brown Fox


Changing Queue and Message Options
==================================

Refer to the :ref:`cx_Oracle AQ API <aq>` and
`Oracle Advanced Queuing documentation
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=ADQUE>`__ for details
on all of the enqueue and dequeue options available.

Enqueue options can be set.  For example, to make it so that an explicit
call to :meth:`~Connection.commit()` on the connection is not needed to commit
messages:

.. code-block:: python

    queue = connection.queue("DEMO_RAW_QUEUE")
    queue.enqoptions.visibility = cx_Oracle.ENQ_IMMEDIATE

Dequeue options can also be set.  For example, to specify not to block on
dequeuing if no messages are available:

.. code-block:: python

    queue = connection.queue("DEMO_RAW_QUEUE")
    queue.deqoptions.wait = cx_Oracle.DEQ_NO_WAIT

Message properties can be set when enqueuing.  For example, to set an
expiration of 60 seconds on a message:

.. code-block:: python

    queue.enqone(connection.msgproperties(payload="Message", expiration=60))

This means that if no dequeue operation occurs within 60 seconds that the
message will be dropped from the queue.


Bulk Enqueue and Dequeue
========================

The :meth:`~Queue.enqmany()` and :meth:`~Queue.deqmany()` methods can be used
for efficient bulk message handling.

:meth:`~Queue.enqmany()` is similar to :meth:`~Queue.enqone()` but accepts an
array of messages:

.. code-block:: python

    messages = [
        "The first message",
        "The second message",
        "The third message",
    ]
    queue = connection.queue("DEMO_RAW_QUEUE")
    queue.enqmany(connection.msgproperties(payload=m) for m in messages)
    connection.commit()

.. warning::

    Calling :meth:`~Queue.enqmany()` in parallel on different connections
    acquired from the same pool may fail due to Oracle bug 29928074.  Ensure
    that this function is not run in parallel, use standalone connections or
    connections from different pools, or make multiple calls to
    :meth:`~Queue.enqone()` instead. The function :meth:`~Queue.deqmany()` call
    is not affected.

To dequeue multiple messages at one time, use :meth:`~Queue.deqmany()`.  This
takes an argument specifying the maximum number of messages to dequeue at one
time:

.. code-block:: python

    for m in queue.deqmany(10):
        print(m.payload.decode(connection.encoding))

Depending on the queue properties and the number of messages available to
dequeue, this code will print out from zero to ten messages.
