.. _aq:

****************
Advanced Queuing
****************

.. _deqoptions:

---------------
Dequeue Options
---------------

.. note::

    This object is an extension to the DB API. It is returned by the
    :meth:`Connection.deqoptions()` call and is used in calls to
    :meth:`Connection.deq()`.


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

    This object is an extension to the DB API. It is returned by the
    :meth:`Connection.enqoptions()` call and is used in calls to
    :meth:`Connection.enq()`.


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

    This object is an extension to the DB API. It is returned by the
    :meth:`Connection.msgproperties()` call and is used in calls to
    :meth:`Connection.deq()` and :meth:`Connection.enq()`.


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

    This attribute specifies the id of the message in the last queue that
    generated this message.


.. attribute:: MessageProperties.priority

    This attribute specifies the priority of the message. A smaller number
    indicates a higher priority. The priority can be any integer, including
    negative numbers. The default value is zero.


.. attribute:: MessageProperties.state

    This read-only attribute specifies the state of the message at the time of
    the dequeue. It will be one of the values :data:`~cx_Oracle.MSG_WAITING`,
    :data:`~cx_Oracle.MSG_READY`, :data:`~cx_Oracle.MSG_PROCESSED` or
    :data:`~cx_Oracle.MSG_EXPIRED`.

