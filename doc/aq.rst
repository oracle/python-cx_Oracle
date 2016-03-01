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
   :meth:`~Connection.deqoptions()` call and is used in calls to
   :meth:`~Connection.deq()`.


.. attribute:: DeqOptions.condition

   This attribute specifies a boolean expression similar to the where clause
   of a SQL query. The boolean expression can include conditions on message
   properties, user data properties and PL/SQL or SQL functions.


.. attribute:: DeqOptions.consumername

   This attribute specifies the name of the consumer. Only messages matching
   the consumer name will be accessed. If the queue is not set up for multiple
   consumers this attribute should not be set.


.. attribute:: DeqOptions.correlation

   This attribute specifies the correlation identifier of the message to be
   dequeued. Special pattern-matching characters, such as the percent sign (%)
   and the underscore (_), can be used. If multiple messages satisfy the
   pattern, the order of dequeuing is indeterminate.


.. attribute:: DeqOptions.deliverymode

   This write-only attribute specifies whether persistent
   (:data:`cx_Oracle.MSG_PERSISTENT`), buffered
   (:data:`cx_Oracle.MSG_BUFFERED`) or both
   (:data:`cx_Oracle.MSG_PERSISTENT_OR_BUFFERED`) types of messages should
   be dequeued.


.. attribute:: DeqOptions.mode

   This attribute specifies the locking behaviour associated with the dequeue
   operation. The valid values are :data:`cx_Oracle.DEQ_BROWSE` (the message is
   read without acquiring any lock on the message),
   :data:`cx_Oracle.DEQ_LOCKED` (the message is read and a write lock is
   obtained), :data:`cx_Oracle.DEQ_REMOVE` (the message is read and updated
   or deleted, which is also the default value) or
   :data:`cx_Oracle.DEQ_REMOVE_NODATA` (which confirms receipt of the message
   but does not deliver the actual message content).


.. attribute:: DeqOptions.msgid

   This attribute specifies the identifier of the message to be dequeued.


.. attribute:: DeqOptions.navigation

   This attribute specifies the position of the message that is retrieved.
   The valid values are :data:`cx_Oracle.DEQ_FIRST_MSG` (retrieves the first
   available message that matches the search criteria),
   :data:`cx_Oracle.DEQ_NEXT_MSG` (retrieves the next available message that
   matches the search criteria, which is also the default value), or
   :data:`cx_Oracle.DEQ_NEXT_TRANSACTION` (skips the remainder of the current
   transaction group and retrieves the first message of the next transaction
   group).


.. attribute:: DeqOptions.transformation

   This attribute specifies the name of the transformation that must be applied
   after the message is enqueued from the database but before it is returned to
   the calling application. The transformation must be created using
   dbms_transform.


.. attribute:: DeqOptions.visibility

   This attribute specifies the transactional behavior of the enqueue request.
   It must be one of :data:`cx_Oracle.ENQ_ON_COMMIT` indicating that the
   enqueue is part of the current transaction (the default) or
   :data:`cx_Oracle.ENQ_IMMEDIATE` indicating that the enqueue operation
   constitutes a transaction of its own. This parameter is ignored when using
   the DEQ_BROWSE mode.


.. _enqoptions:

---------------
Enqueue Options
---------------

.. note::

   This object is an extension to the DB API. It is returned by the
   :meth:`~Connection.enqoptions()` call and is used in calls to
   :meth:`~Connection.enq()`.


.. attribute:: EnqOptions.deliverymode

   This write-only attribute specifies whether persistent
   (:data:`cx_Oracle.MSG_PERSISTENT`) or buffered
   (:data:`cx_Oracle.MSG_BUFFERED`) messages should be enqueued.


.. attribute:: EnqOptions.transformation

   This attribute specifies the name of the transformation that must be applied
   before the message is enqueued into the database. The transformation must
   be created using dbms_transform.


.. attribute:: EnqOptions.visibility

   This attribute specifies the transactional behavior of the enqueue request.
   It must be one of :data:`cx_Oracle.ENQ_ON_COMMIT` indicating that the
   enqueue is part of the current transaction (the default) or
   :data:`cx_Oracle.ENQ_IMMEDIATE` indicating that the enqueue operation
   constitutes a transaction of its own.


.. _msgproperties:

------------------
Message Properties
------------------

.. note::

   This object is an extension to the DB API. It is returned by the
   :meth:`~Connection.msgproperties()` call and is used in calls to
   :meth:`~Connection.deq` and :meth:`~Connection.enq()`.


.. attribute:: MessageProperties.attempts

   This read-only attribute specifies the number of attempts that have been
   made to dequeue the message.


.. attribute:: MessageProperties.correlation

   This attribute specifies the correlation used when the message was enqueued.


.. attribute:: MessageProperties.delay

   This attribute specifies the number of seconds to delay an enqueued message.
   Any integer is acceptable but the constant :data:`cx_Oracle.MSG_NO_DELAY`
   can also be used indicating that the message is available for immediate
   dequeuing.


.. attribute:: EnqOptions.deliverymode

   This read-only attribute specifies whether a persistent
   (:data:`cx_Oracle.MSG_PERSISTENT`) or buffered
   (:data:`cx_Oracle.MSG_BUFFERED`) message was dequeued.


.. attribute:: EnqOptions.enqtime

   This read-only attribute specifies the time that the message was enqueued.


.. attribute:: EnqOptions.exceptionq

   This attribute specifies the name of the queue to which the message is
   moved if it cannot be processed successfully. Messages are moved if the
   number of unsuccessful dequeue attempts has exceeded the maximum number of
   retries or if the message has expired. All messages in the exception queue
   are in the EXPIRED state. The default value is the name of the exception
   queue associated with the queue table.


.. attribute:: EnqOptions.expiration

   This attribute specifies, in seconds, how long the message is available for
   dequeuing. This attribute is an offset from the delay attribute. Expiration
   processing requires the queue monitor to be running. Any integer is accepted
   but the constant :data:`cx_Oracle.MSG_NO_EXPIRATION` can also be used
   indicating that the message never expires.


.. attribute:: EnqOptions.msgid

   This attribute specifies the id of the message in the last queue that
   generated this message.


.. attribute:: EnqOptions.priority

   This attribute specifies the priority of the message. A smaller number
   indicates a higher priority. The priority can be any integer, including
   negative numbers. The default value is zero.


.. attribute:: EnqOptions.state

   This read-only attribute specifies the state of the message at the time of
   the dequeue. It will be one of :data:`cx_Oracle.MSG_WAITING` (the message
   delay has not yet been reached), :data:`cx_Oracle.MSG_READY` (the message
   is ready to be processed), :data:`cx_Oracle.MSG_PROCESSED` (the message
   has been processed and is retained) or :data:`cx_Oracle.MSG_EXPIRED`
   (the message has been moved to the exception queue).

