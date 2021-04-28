.. _connobj:

*****************
Connection Object
*****************

.. note::

    Any outstanding changes will be rolled back when the connection object
    is destroyed or closed.



.. method:: Connection.__enter__()

    The entry point for the connection as a context manager. It returns itself.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.__exit__()

    The exit point for the connection as a context manager. This will close
    the connection and roll back any uncommitted transaction.

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.action

    This write-only attribute sets the action column in the v$session table. It
    is a string attribute and cannot be set to None -- use the empty string
    instead.

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.autocommit

    This read-write attribute determines whether autocommit mode is on or off.
    When autocommit mode is on, all statements are committed as soon as they
    have completed executing.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.begin([formatId, transactionId, branchId])

    Explicitly begin a new transaction. Without parameters, this explicitly
    begins a local transaction; otherwise, this explicitly begins a distributed
    (global) transaction with the given parameters. See the Oracle
    documentation for more details.

    Note that in order to make use of global (distributed) transactions, the
    :attr:`~Connection.internal_name` and :attr:`~Connection.external_name`
    attributes must be set.

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.call_timeout

    This read-write attribute specifies the amount of time (in milliseconds)
    that a single round-trip to the database may take before a timeout will
    occur. A value of 0 means that no timeout will take place.

    .. versionadded:: 7.0

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `callTimeout` was renamed to `call_timeout`. The old name
        will continue to work for a period of time.

    .. note::

        This attribute is an extension to the DB API definition and is only
        available in Oracle Client 18c and higher.


.. method:: Connection.cancel()

    Break a long-running transaction.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.changepassword(oldpassword, newpassword)

    Change the password of the logon.

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.client_identifier

    This write-only attribute sets the client_identifier column in the
    v$session table.

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.clientinfo

    This write-only attribute sets the client_info column in the v$session
    table.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.close()

    Close the connection now, rather than whenever __del__ is called. The
    connection will be unusable from this point forward; an Error exception
    will be raised if any operation is attempted with the connection.

    All open cursors and LOBs created by the connection will be closed and will
    also no longer be usable.

    Internally, references to the connection are held by cursor objects,
    LOB objects, subscription objects, etc. Once all of these references are
    released, the connection itself will be closed automatically. Either
    control references to these related objects carefully or explicitly close
    connections in order to ensure sufficient resources are available.


.. method:: Connection.commit()

    Commit any pending transactions to the database.


.. method:: Connection.createlob(lobType)

    Create and return a new temporary :ref:`LOB object <lobobj>` of the
    specified type. The lobType parameter should be one of
    :data:`cx_Oracle.CLOB`, :data:`cx_Oracle.BLOB` or :data:`cx_Oracle.NCLOB`.

    .. versionadded:: 6.2

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.current_schema

    This read-write attribute sets the current schema attribute for the
    session. Setting this value is the same as executing the SQL statement
    "ALTER SESSION SET CURRENT_SCHEMA". The attribute is set (and verified) on
    the next call that does a round trip to the server. The value is placed
    before unqualified database objects in SQL statements you then execute.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.cursor()

    Return a new :ref:`cursor object <cursorobj>` using the connection.


.. attribute:: Connection.dbop

    This write-only attribute sets the database operation that is to be
    monitored. This can be viewed in the DBOP_NAME column of the V$SQL_MONITOR
    table.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.deq(name, options, msgproperties, payload)

    Returns a message id after successfully dequeuing a message. The options
    object can be created using :meth:`~Connection.deqoptions()` and the
    msgproperties object can be created using
    :meth:`~Connection.msgproperties()`. The payload must be an object created
    using :meth:`ObjectType.newobject()`.

    .. versionadded:: 5.3

    .. deprecated:: 7.2

        Use the methods :meth:`Queue.deqone()` or :meth:`Queue.deqmany()`
        instead.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.deqoptions()

    Returns an object specifying the options to use when dequeuing messages.
    See :ref:`deqoptions` for more information.

    .. versionadded:: 5.3

    .. deprecated:: 7.2

        Use the attribute :attr:`Queue.deqoptions` instead.

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.dsn

    This read-only attribute returns the TNS entry of the database to which a
    connection has been established.

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.edition

    This read-only attribute gets the session edition and is only available in
    Oracle Database 11.2 (both client and server must be at this level or
    higher for this to work).

    .. versionadded:: 5.3

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.encoding

    This read-only attribute returns the IANA character set name of the
    character set in use by the Oracle client for regular strings.

    .. deprecated:: 8.2

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.enq(name, options, msgproperties, payload)

    Returns a message id after successfully enqueuing a message. The options
    object can be created using :meth:`~Connection.enqoptions()` and the
    msgproperties object can be created using
    :meth:`~Connection.msgproperties()`. The payload must be an object created
    using :meth:`ObjectType.newobject()`.

    .. versionadded:: 5.3

    .. deprecated:: 7.2

        Use the methods :meth:`Queue.enqone()` or :meth:`Queue.enqmany()`
        instead.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.enqoptions()

    Returns an object specifying the options to use when enqueuing messages.
    See :ref:`enqoptions` for more information.

    .. versionadded:: 5.3

    .. deprecated:: 7.2

        Use the attribute :attr:`Queue.enqoptions` instead.

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.external_name

    This read-write attribute specifies the external name that is used by the
    connection when logging distributed transactions.

    .. versionadded:: 5.3

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.getSodaDatabase()

    Return a :ref:`SodaDatabase <sodadb>` object for Simple Oracle Document
    Access (SODA). All SODA operations are performed either on the returned
    SodaDatabase object or from objects created by the returned SodaDatabase
    object. See `here <https://www.oracle.com/pls/topic/lookup?
    ctx=dblatest&id=GUID-BE42F8D3-B86B-43B4-B2A3-5760A4DF79FB>`__  for
    additional information on SODA.

    .. versionadded:: 7.0

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.gettype(name)

    Return a :ref:`type object <objecttype>` given its name. This can then be
    used to create objects which can be bound to cursors created by this
    connection.

    .. versionadded:: 5.3

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.handle

    This read-only attribute returns the OCI service context handle for the
    connection. It is primarily provided to facilitate testing the creation of
    a connection using the OCI service context handle.

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.inputtypehandler

    This read-write attribute specifies a method called for each value that is
    bound to a statement executed on any cursor associated with this
    connection.  The method signature is handler(cursor, value, arraysize) and
    the return value is expected to be a variable object or None in which case
    a default variable object will be created. If this attribute is None, the
    default behavior will take place for all values bound to statements.

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.internal_name

    This read-write attribute specifies the internal name that is used by the
    connection when logging distributed transactions.

    .. versionadded:: 5.3

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.ltxid

    This read-only attribute returns the logical transaction id for the
    connection. It is used within Oracle Transaction Guard as a means of
    ensuring that transactions are not duplicated. See the Oracle documentation
    and the provided sample for more information.

    .. versionadded:: 5.3

    .. note:

        This attribute is an extension to the DB API definition. It is only
        available when Oracle Database 12.1 or higher is in use on both the
        server and the client.


.. attribute:: Connection.maxBytesPerCharacter

    This read-only attribute returns the maximum number of bytes each character
    can use for the client character set.

    .. deprecated:: 8.2

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.module

    This write-only attribute sets the module column in the v$session table.
    The maximum length for this string is 48 and if you exceed this length you
    will get ORA-24960.

    .. note:

        This attribute is an extension to the DB API definition.


.. method:: Connection.msgproperties(payload, correlation, delay, exceptionq, \
        expiration, priority)

    Returns an object specifying the properties of messages used in advanced
    queuing. See :ref:`msgproperties` for more information.

    Each of the parameters are optional. If specified, they act as a shortcut
    for setting each of the equivalently named properties.

    .. versionadded:: 5.3

    .. versionchanged:: 7.2 Added parameters

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.nencoding

    This read-only attribute returns the IANA character set name of the
    national character set in use by the Oracle client.

    .. deprecated:: 8.2

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.outputtypehandler

    This read-write attribute specifies a method called for each column that is
    going to be fetched from any cursor associated with this connection. The
    method signature is handler(cursor, name, defaultType, length, precision,
    scale) and the return value is expected to be a variable object or None in
    which case a default variable object will be created. If this attribute is
    None, the default behavior will take place for all columns fetched from
    cursors.

    See :ref:`outputtypehandlers`.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.ping()

    Ping the server which can be used to test if the connection is still
    active.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.prepare()

    Prepare the distributed (global) transaction for commit. Return a boolean
    indicating if a transaction was actually prepared in order to avoid the
    error ORA-24756 (transaction does not exist).

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.queue(name, payload_type=None)

    Creates a :ref:`queue <queue>` which is used to enqueue and dequeue
    messages in Advanced Queueing.

    The name parameter is expected to be a string identifying the queue in
    which messages are to be enqueued or dequeued.

    The payload_type parameter, if specified, is expected to be an
    :ref:`object type <objecttype>` that identifies the type of payload the
    queue expects. If not specified, RAW data is enqueued and dequeued.

    .. versionadded:: 7.2

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        parameter `payloadType` was renamed to `payload_type`. The old name
        will continue to work as a keyword parameter for a period of time.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.rollback()

    Rollback any pending transactions.


.. method:: Connection.shutdown([mode])

    Shutdown the database. In order to do this the connection must be connected
    as :data:`~cx_Oracle.SYSDBA` or :data:`~cx_Oracle.SYSOPER`. Two calls must
    be made unless the mode specified is :data:`~cx_Oracle.DBSHUTDOWN_ABORT`.
    An example is shown below:

    ::

        import cx_Oracle

        connection = cx_Oracle.connect(mode = cx_Oracle.SYSDBA)
        connection.shutdown(mode = cx_Oracle.DBSHUTDOWN_IMMEDIATE)
        cursor = connection.cursor()
        cursor.execute("alter database close normal")
        cursor.execute("alter database dismount")
        connection.shutdown(mode = cx_Oracle.DBSHUTDOWN_FINAL)

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.startup(force=False, restrict=False, pfile=None)

    Startup the database. This is equivalent to the SQL\*Plus command "startup
    nomount". The connection must be connected as :data:`~cx_Oracle.SYSDBA` or
    :data:`~cx_Oracle.SYSOPER` with the :data:`~cx_Oracle.PRELIM_AUTH` option
    specified for this to work.

    The pfile parameter, if specified, is expected to be a string identifying
    the location of the parameter file (PFILE) which will be used instead of
    the stored parameter file (SPFILE).

    An example is shown below:

    ::

        import cx_Oracle

        connection = cx_Oracle.connect(
                mode=cx_Oracle.SYSDBA | cx_Oracle.PRELIM_AUTH)
        connection.startup()
        connection = cx_Oracle.connect(mode=cx_Oracle.SYSDBA)
        cursor = connection.cursor()
        cursor.execute("alter database mount")
        cursor.execute("alter database open")

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.stmtcachesize

    This read-write attribute specifies the size of the statement cache. This
    value can make a significant difference in performance if you have a small
    number of statements that you execute repeatedly.

    The default value is 20.

    See :ref:`Statement Caching <stmtcache>` for more information.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.subscribe(namespace=cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE, protocol=cx_Oracle.SUBSCR_PROTO_OCI, callback=None, timeout=0, operations=OPCODE_ALLOPS, port=0, qos=0, ip_address=None, grouping_class=0, grouping_value=0, grouping_type=cx_Oracle.SUBSCR_GROUPING_TYPE_SUMMARY, name=None, client_initiated=False)

    Return a new :ref:`subscription object <subscrobj>` that receives
    notifications for events that take place in the database that match the
    given parameters.

    The namespace parameter specifies the namespace the subscription uses. It
    can be one of :data:`cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE` or
    :data:`cx_Oracle.SUBSCR_NAMESPACE_AQ`.

    The protocol parameter specifies the protocol to use when notifications are
    sent. Currently the only valid value is :data:`cx_Oracle.SUBSCR_PROTO_OCI`.

    The callback is expected to be a callable that accepts a single parameter.
    A :ref:`message object <msgobjects>` is passed to this callback whenever a
    notification is received.

    The timeout value specifies that the subscription expires after the given
    time in seconds. The default value of 0 indicates that the subscription
    never expires.

    The operations parameter enables filtering of the messages that are sent
    (insert, update, delete). The default value will send notifications for all
    operations. This parameter is only used when the namespace is set to
    :data:`cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE`.

    The port parameter specifies the listening port for callback notifications
    from the database server. If not specified, an unused port will be selected
    by the Oracle Client libraries.

    The qos parameter specifies quality of service options. It should be one or
    more of the following flags, OR'ed together:
    :data:`cx_Oracle.SUBSCR_QOS_RELIABLE`,
    :data:`cx_Oracle.SUBSCR_QOS_DEREG_NFY`,
    :data:`cx_Oracle.SUBSCR_QOS_ROWIDS`,
    :data:`cx_Oracle.SUBSCR_QOS_QUERY`,
    :data:`cx_Oracle.SUBSCR_QOS_BEST_EFFORT`.

    The ip_address parameter specifies the IP address (IPv4 or IPv6) in
    standard string notation to bind for callback notifications from the
    database server. If not specified, the client IP address will be determined
    by the Oracle Client libraries.

    The grouping_class parameter specifies what type of grouping of
    notifications should take place. Currently, if set, this value can only be
    set to the value :data:`cx_Oracle.SUBSCR_GROUPING_CLASS_TIME`, which
    will group notifications by the number of seconds specified in the
    grouping_value parameter. The grouping_type parameter should be one of the
    values :data:`cx_Oracle.SUBSCR_GROUPING_TYPE_SUMMARY` (the default) or
    :data:`cx_Oracle.SUBSCR_GROUPING_TYPE_LAST`.

    The name parameter is used to identify the subscription and is specific to
    the selected namespace. If the namespace parameter is
    :data:`cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE` then the name is optional and
    can be any value. If the namespace parameter is
    :data:`cx_Oracle.SUBSCR_NAMESPACE_AQ`, however, the name must be in the
    format '<QUEUE_NAME>' for single consumer queues and
    '<QUEUE_NAME>:<CONSUMER_NAME>' for multiple consumer queues, and identifies
    the queue that will be monitored for messages. The queue name may include
    the schema, if needed.

    The client_initiated parameter is used to determine if client initiated
    connections or server initiated connections (the default) will be
    established. Client initiated connections are only available in Oracle
    Client 19.4 and Oracle Database 19.4 and higher.

    .. versionadded:: 6.4

        The parameters ipAddress, groupingClass, groupingValue, groupingType
        and name were added.

    .. versionadded:: 7.3

        The parameter clientInitiated was added.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        parameter `ipAddress` was renamed to `ip_address`, the parameter
        `groupingClass` was renamed to `grouping_class`, the parameter
        `groupingValue` was renamed to `grouping_value`, the parameter
        `groupingType` was renamed to `grouping_type` and the parameter
        `clientInitiated` was renamed to `client_initiated`. The old names will
        continue to work as keyword parameters for a period of time.

    .. note::

        This method is an extension to the DB API definition.

    .. note::

        The subscription can be deregistered in the database by calling the
        function :meth:`~Connection.unsubscribe()`. If this method is not
        called and the connection that was used to create the subscription is
        explicitly closed using the function :meth:`~Connection.close()`, the
        subscription will not be deregistered in the database.


.. attribute:: Connection.tag

    This read-write attribute initially contains the actual tag of the session
    that was acquired from a pool by :meth:`SessionPool.acquire()`. If the
    connection was not acquired from a pool or no tagging parameters were
    specified (tag and matchanytag) when the connection was acquired from the
    pool, this value will be None. If the value is changed, it must be a string
    containing name=value pairs like "k1=v1;k2=v2".

    If this value is not None when the connection is released back to the pool
    it will be used to retag the session. This value can be overridden in the
    call to :meth:`SessionPool.release()`.

    .. note::

        This attribute is an extension to the DB API definition.

    .. versionadded:: 7.1


.. attribute:: Connection.tnsentry

    This read-only attribute returns the TNS entry of the database to which a
    connection has been established.

    .. deprecated:: 8.2

        Use the attribute :attr:`~Connection.dsn` instead.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.unsubscribe(subscr)

    Unsubscribe from events in the database that were originally subscribed to
    using :meth:`~Connection.subscribe()`. The connection used to unsubscribe
    should be the same one used to create the subscription, or should access
    the same database and be connected as the same user name.

    .. versionadded:: 6.4


.. attribute:: Connection.username

    This read-only attribute returns the name of the user which established the
    connection to the database.

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.version

    This read-only attribute returns the version of the database to which a
    connection has been established.

    .. note::

        This attribute is an extension to the DB API definition.

    .. note::

        If you connect to Oracle Database 18 or higher with client libraries
        12.2 or lower that you will only receive the base version (such as
        18.0.0.0.0) instead of the full version (18.3.0.0.0).
