.. _subscrobj:

*******************
Subscription Object
*******************

.. note::

    This object is an extension the DB API.


.. attribute:: Subscription.callback

    This read-only attribute returns the callback that was registered when the
    subscription was created.


.. attribute:: Subscription.connection

    This read-only attribute returns the connection that was used to register
    the subscription when it was created.


.. attribute:: Subscription.id

    This read-only attribute returns the value of ``REGID`` found in the
    database view ``USER_CHANGE_NOTIFICATION_REGS`` or the value of ``REG_ID``
    found in the database view ``USER_SUBSCR_REGISTRATIONS``. For AQ
    subscriptions, the value is 0.


.. attribute:: Subscription.ip_address

    This read-only attribute returns the IP address used for callback
    notifications from the database server. If not set during construction,
    this value is None.

    .. versionadded:: 6.4

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `ipAddress` was renamed to `ip_address`. The old name will
        continue to work for a period of time.


.. attribute:: Subscription.name

    This read-only attribute returns the name used to register the subscription
    when it was created.

    .. versionadded:: 6.4


.. attribute:: Subscription.namespace

    This read-only attribute returns the namespace used to register the
    subscription when it was created.


.. attribute:: Subscription.operations

    This read-only attribute returns the operations that will send
    notifications for each table or query that is registered using this
    subscription.


.. attribute:: Subscription.port

    This read-only attribute returns the port used for callback notifications
    from the database server. If not set during construction, this value is
    zero.


.. attribute:: Subscription.protocol

    This read-only attribute returns the protocol used to register the
    subscription when it was created.


.. attribute:: Subscription.qos

    This read-only attribute returns the quality of service flags used to
    register the subscription when it was created.


.. method:: Subscription.registerquery(statement, [args])

    Register the query for subsequent notification when tables referenced by
    the query are changed. This behaves similarly to cursor.execute() but only
    queries are permitted and the args parameter must be a sequence or
    dictionary.  If the qos parameter included the flag
    cx_Oracle.SUBSCR_QOS_QUERY when the subscription was created, then the ID
    for the registered query is returned; otherwise, None is returned.


.. attribute:: Subscription.timeout

    This read-only attribute returns the timeout (in seconds) that was
    specified when the subscription was created. A value of 0 indicates that
    there is no timeout.


.. _msgobjects:

Message Objects
---------------

.. note::

    This object is created internally when notification is received and passed
    to the callback procedure specified when a subscription is created.


.. attribute:: Message.consumer_name

    This read-only attribute returns the name of the consumer which generated
    the notification. It will be populated if the subscription was created with
    the namespace :data:`cx_Oracle.SUBSCR_NAMESPACE_AQ` and the queue is a
    multiple consumer queue.

    .. versionadded:: 6.4

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `consumerName` was renamed to `consumer_name`. The old name
        will continue to work for a period of time.


.. attribute:: Message.dbname

    This read-only attribute returns the name of the database that generated
    the notification.


.. attribute:: Message.queries

    This read-only attribute returns a list of message query objects that give
    information about query result sets changed for this notification. This
    attribute will be None if the qos parameter did not include the flag
    :data:`~cx_Oracle.SUBSCR_QOS_QUERY` when the subscription was created.


.. attribute:: Message.queue_name

    This read-only attribute returns the name of the queue which generated the
    notification. It will only be populated if the subscription was created
    with the namespace :data:`cx_Oracle.SUBSCR_NAMESPACE_AQ`.

    .. versionadded:: 6.4

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `queueName` was renamed to `queue_name`. The old name will
        continue to work for a period of time.


.. attribute:: Message.registered

    This read-only attribute returns whether the subscription which generated
    this notification is still registered with the database. The subscription
    is automatically deregistered with the database when the subscription
    timeout value is reached or when the first notification is sent (when the
    quality of service flag :data:`cx_Oracle.SUBSCR_QOS_DEREG_NFY` is used).

    .. versionadded:: 6.4


.. attribute:: Message.subscription

    This read-only attribute returns the subscription object for which this
    notification was generated.


.. attribute:: Message.tables

    This read-only attribute returns a list of message table objects that give
    information about the tables changed for this notification. This
    attribute will be None if the qos parameter included the flag
    :data:`~cx_Oracle.SUBSCR_QOS_QUERY` when the subscription was created.


.. attribute:: Message.txid

    This read-only attribute returns the id of the transaction that generated
    the notification.


.. attribute:: Message.type

    This read-only attribute returns the type of message that has been sent.
    See the constants section on event types for additional information.


Message Table Objects
---------------------

.. note::

    This object is created internally for each table changed when notification
    is received and is found in the tables attribute of message objects, and
    the tables attribute of message query objects.


.. attribute:: MessageTable.name

    This read-only attribute returns the name of the table that was changed.


.. attribute:: MessageTable.operation

    This read-only attribute returns the operation that took place on the table
    that was changed.


.. attribute:: MessageTable.rows

    This read-only attribute returns a list of message row objects that give
    information about the rows changed on the table. This value is only filled
    in if the qos parameter to the :meth:`Connection.subscribe()` method
    included the flag :data:`~cx_Oracle.SUBSCR_QOS_ROWIDS`.


Message Row Objects
-------------------

.. note::

    This object is created internally for each row changed on a table when
    notification is received and is found in the rows attribute of message
    table objects.


.. attribute:: MessageRow.operation

    This read-only attribute returns the operation that took place on the row
    that was changed.


.. attribute:: MessageRow.rowid

    This read-only attribute returns the rowid of the row that was changed.


Message Query Objects
---------------------

.. note::

    This object is created internally for each query result set changed when
    notification is received and is found in the queries attribute of message
    objects.


.. attribute:: MessageQuery.id

    This read-only attribute returns the query id of the query for which the
    result set changed. The value will match the value returned by
    Subscription.registerquery when the related query was registered.


.. attribute:: MessageQuery.operation

    This read-only attribute returns the operation that took place on the query
    result set that was changed. Valid values for this attribute are
    :data:`~cx_Oracle.EVENT_DEREG` and :data:`~cx_Oracle.EVENT_QUERYCHANGE`.


.. attribute:: MessageQuery.tables

    This read-only attribute returns a list of message table objects that give
    information about the table changes that caused the query result set to
    change for this notification.
