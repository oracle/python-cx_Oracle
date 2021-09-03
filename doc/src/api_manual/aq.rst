.. _aq:

*********************
Advanced Queuing (AQ)
*********************

See :ref:`aqusermanual` for more information about using AQ in cx_Oracle.

.. note::

    All of these objects are extensions to the DB API.

.. _queue:

------
Queues
------

These objects are created using the :meth:`Connection.queue()` method and are
used to enqueue and dequeue messages.

.. attribute:: Queue.connection

    This read-only attribute returns a reference to the connection object on
    which the queue was created.


.. method:: Queue.deqmany(maxMessages)

    Dequeues up to the specified number of messages from the queue and returns
    a list of these messages. Each element of the returned list is a
    :ref:`message property<msgproperties>` object.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the method was changed from `deqMany()`. The old name will continue to
        work for a period of time.

.. method:: Queue.deqone()

    Dequeues at most one message from the queue. If a message is dequeued, it
    will be a :ref:`message property<msgproperties>` object; otherwise, it will
    be the value None.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the method was changed from `deqOne()`. The old name will continue to
        work for a period of time.

.. attribute:: Queue.deqoptions

    This read-only attribute returns a reference to the :ref:`options
    <deqoptions>` that will be used when dequeuing messages from the queue.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the attribute was changed from `deqOptions`. The old name will continue
        to work for a period of time.

.. method:: Queue.enqmany(messages)

    Enqueues multiple messages into the queue. The messages parameter must be a
    sequence containing :ref:`message property <msgproperties>` objects which
    have all had their payload attribute set to a value that the queue
    supports.

    Warning: calling this function in parallel on different connections
    acquired from the same pool may fail due to Oracle bug 29928074. Ensure
    that this function is not run in parallel, use standalone connections or
    connections from different pools, or make multiple calls to
    :meth:`Queue.enqone()` instead. The function :meth:`Queue.deqmany()`
    call is not affected.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the method was changed from `enqMany()`. The old name will continue
        to work for a period of time.


.. method:: Queue.enqone(message)

    Enqueues a single message into the queue. The message must be a
    :ref:`message property<msgproperties>` object which has had its payload
    attribute set to a value that the queue supports.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the method was changed from `enqOne()`. The old name will continue
        to work for a period of time.

.. attribute:: Queue.enqoptions

    This read-only attribute returns a reference to the :ref:`options
    <enqoptions>` that will be used when enqueuing messages into the queue.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the attribute was changed from `enqOptions`. The old name will continue
        to work for a period of time.


.. attribute:: Queue.name

    This read-only attribute returns the name of the queue.


.. attribute:: Queue.payload_type

    This read-only attribute returns the object type for payloads that can be
    enqueued and dequeued. If using a raw queue, this returns the value None.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the name of
        the attribute was changed from `payloadType`. The old name will
        continue to work for a period of time.


.. _deqoptions:

---------------
Dequeue Options
---------------

.. note::

    These objects are used to configure how messages are dequeued from queues.
    An instance of this object is found in the attribute
    :attr:`Queue.deqOptions`.


.. attribute:: DeqOptions.condition

    This attribute specifies a boolean expression similar to the where clause
    of a SQL query. The boolean expression can include conditions on message
    properties, user data properties and PL/SQL or SQL functions. The default
    is to have no condition specified.


.. attribute:: DeqOptions.consumername

    This attribute specifies the name of the consumer. Only messages matching
    the consumer name will be accessed. If the queue is not set up for multiple
    consumers this attribute should not be set. The default is to have no
    consumer name specified.


.. attribute:: DeqOptions.correlation

    This attribute specifies the correlation identifier of the message to be
    dequeued. Special pattern-matching characters, such as the percent sign (%)
    and the underscore (_), can be used. If multiple messages satisfy the
    pattern, the order of dequeuing is indeterminate. The default is to have no
    correlation specified.


.. attribute:: DeqOptions.deliverymode

    This write-only attribute specifies what types of messages should be
    dequeued. It should be one of the values :data:`~cx_Oracle.MSG_PERSISTENT`
    (default), :data:`~cx_Oracle.MSG_BUFFERED` or
    :data:`~cx_Oracle.MSG_PERSISTENT_OR_BUFFERED`.


.. attribute:: DeqOptions.mode

    This attribute specifies the locking behaviour associated with the dequeue
    operation. It should be one of the values :data:`~cx_Oracle.DEQ_BROWSE`,
    :data:`~cx_Oracle.DEQ_LOCKED`,
    :data:`~cx_Oracle.DEQ_REMOVE` (default), or
    :data:`~cx_Oracle.DEQ_REMOVE_NODATA`.


.. attribute:: DeqOptions.msgid

    This attribute specifies the identifier of the message to be dequeued. The
    default is to have no message identifier specified.


.. attribute:: DeqOptions.navigation

    This attribute specifies the position of the message that is retrieved. It
    should be one of the values :data:`~cx_Oracle.DEQ_FIRST_MSG`,
    :data:`~cx_Oracle.DEQ_NEXT_MSG` (default), or
    :data:`~cx_Oracle.DEQ_NEXT_TRANSACTION`.


.. attribute:: DeqOptions.transformation

    This attribute specifies the name of the transformation that must be
    applied after the message is dequeued from the database but before it is
    returned to the calling application. The transformation must be created
    using dbms_transform. The default is to have no transformation specified.


.. attribute:: DeqOptions.visibility

    This attribute specifies the transactional behavior of the dequeue request.
    It should be one of the values :data:`~cx_Oracle.DEQ_ON_COMMIT` (default)
    or :data:`~cx_Oracle.DEQ_IMMEDIATE`. This attribute is ignored when using
    the :data:`~cx_Oracle.DEQ_BROWSE` mode. Note the value of
    :attr:`~Connection.autocommit` is always ignored.


.. attribute:: DeqOptions.wait

    This attribute specifies the time to wait, in seconds, for a message
    matching the search criteria to become available for dequeuing. One of the
    values :data:`~cx_Oracle.DEQ_NO_WAIT` or
    :data:`~cx_Oracle.DEQ_WAIT_FOREVER` can also be used. The default is
    :data:`~cx_Oracle.DEQ_WAIT_FOREVER`.


.. _enqoptions:

---------------
Enqueue Options
---------------

.. note::

    These objects are used to configure how messages are enqueued into queues.
    An instance of this object is found in the attribute
    :attr:`Queue.enqOptions`.


.. attribute:: EnqOptions.deliverymode

    This write-only attribute specifies what type of messages should be
    enqueued. It should be one of the values :data:`~cx_Oracle.MSG_PERSISTENT`
    (default) or :data:`~cx_Oracle.MSG_BUFFERED`.


.. attribute:: EnqOptions.transformation

    This attribute specifies the name of the transformation that must be
    applied before the message is enqueued into the database. The
    transformation must be created using dbms_transform. The default is to have
    no transformation specified.


.. attribute:: EnqOptions.visibility

    This attribute specifies the transactional behavior of the enqueue request.
    It should be one of the values :data:`~cx_Oracle.ENQ_ON_COMMIT` (default)
    or :data:`~cx_Oracle.ENQ_IMMEDIATE`. Note the value of
    :attr:`~Connection.autocommit` is ignored.


.. _msgproperties:

------------------
Message Properties
------------------

.. note::

    These objects are used to identify the properties of messages that are
    enqueued and dequeued in queues. They are created by the method
    :meth:`Connection.msgproperties()`.  They are used by the methods
    :meth:`Queue.enqone()` and :meth:`Queue.enqmany()` and
    returned by the methods :meth:`Queue.deqone()` and :meth:`Queue.deqmany()`.


.. attribute:: MessageProperties.attempts

    This read-only attribute specifies the number of attempts that have been
    made to dequeue the message.


.. attribute:: MessageProperties.correlation

    This attribute specifies the correlation used when the message was
    enqueued.


.. attribute:: MessageProperties.delay

    This attribute specifies the number of seconds to delay an enqueued
    message. Any integer is acceptable but the constant
    :data:`~cx_Oracle.MSG_NO_DELAY` can also be used indicating that the
    message is available for immediate dequeuing.


.. attribute:: MessageProperties.deliverymode

    This read-only attribute specifies the type of message that was dequeued.
    It will be one of the values :data:`~cx_Oracle.MSG_PERSISTENT` or
    :data:`~cx_Oracle.MSG_BUFFERED`.


.. attribute:: MessageProperties.enqtime

    This read-only attribute specifies the time that the message was enqueued.


.. attribute:: MessageProperties.exceptionq

    This attribute specifies the name of the queue to which the message is
    moved if it cannot be processed successfully. Messages are moved if the
    number of unsuccessful dequeue attempts has exceeded the maximum number of
    retries or if the message has expired. All messages in the exception queue
    are in the :data:`~cx_Oracle.MSG_EXPIRED` state. The default value is the
    name of the exception queue associated with the queue table.


.. attribute:: MessageProperties.expiration

    This attribute specifies, in seconds, how long the message is available for
    dequeuing. This attribute is an offset from the delay attribute. Expiration
    processing requires the queue monitor to be running. Any integer is
    accepted but the constant :data:`~cx_Oracle.MSG_NO_EXPIRATION` can also be
    used indicating that the message never expires.


.. attribute:: MessageProperties.msgid

    This read-only attribute specifies the id of the message in the last queue
    that enqueued or dequeued the message. If the message has never been
    dequeued or enqueued, the value will be `None`.


.. attribute:: MessageProperties.payload

    This attribute identifies the payload that will be enqueued or the payload
    that was dequeued when using a :ref:`queue <queue>`. When enqueuing, the
    value is checked to ensure that it conforms to the type expected by that
    queue. For RAW queues, the value can be a bytes object or a string. If the
    value is a string it will first be converted to bytes by encoding in the
    encoding identified by the attribute :attr:`Connection.encoding`.


.. attribute:: MessageProperties.priority

    This attribute specifies the priority of the message. A smaller number
    indicates a higher priority. The priority can be any integer, including
    negative numbers. The default value is zero.


.. attribute:: MessageProperties.state

    This read-only attribute specifies the state of the message at the time of
    the dequeue. It will be one of the values :data:`~cx_Oracle.MSG_WAITING`,
    :data:`~cx_Oracle.MSG_READY`, :data:`~cx_Oracle.MSG_PROCESSED` or
    :data:`~cx_Oracle.MSG_EXPIRED`.
