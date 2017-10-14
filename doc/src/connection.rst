.. _connobj:

*****************
Connection Object
*****************

.. note::

    Any outstanding changes will be rolled back when the connection object
    is destroyed or closed.



.. method:: Connection.__enter__()

    The entry point for the connection as a context manager, a feature
    available in Python 2.5 and higher. It returns itself.

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.__exit__()

    The exit point for the connection as a context manager, a feature available
    in Python 2.5 and higher. In the event of an exception, the transaction is
    rolled back; otherwise, the transaction is committed.

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


.. method:: Connection.cancel()

    Cancel a long-running transaction.

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

    Before the connection can be closed, all cursors created by the connection
    must first be closed or all references released. In addition, all LOB
    objects created by the connection must have their references released. If
    this has not been done, the exception "DPI-1054: connection cannot be
    closed when open statements or LOBs exist" will be raised.

    Internally, references to the connection are held by cursor objects,
    LOB objects, subscription objects, etc. Once all of these references are
    released, the connection itself will be closed automatically. Either
    control references to these related objects carefully or explicitly close
    connections in order to ensure sufficient resources are available.


.. method:: Connection.commit()

    Commit any pending transactions to the database.


.. attribute:: Connection.current_schema

    This read-write attribute sets the current schema attribute for the
    session.

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

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.deqoptions()

    Returns an object specifying the options to use when dequeuing messages.
    See :ref:`deqoptions` for more information.

    .. versionadded:: 5.3

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

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.enq(name, options, msgproperties, payload)

    Returns a message id after successfully enqueuing a message. The options
    object can be created using :meth:`~Connection.enqoptions()` and the
    msgproperties object can be created using
    :meth:`~Connection.msgproperties()`. The payload must be an object created
    using :meth:`ObjectType.newobject()`.

    .. versionadded:: 5.3

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.enqoptions()

    Returns an object specifying the options to use when enqueuing messages.
    See :ref:`enqoptions` for more information.

    .. versionadded:: 5.3

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.external_name

    This read-write attribute specifies the external name that is used by the
    connection when logging distributed transactions.

    .. versionadded:: 5.3

    .. note::

        This attribute is an extension to the DB API definition.


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

    .. note::

        This attribute is an extension to the DB API definition.


.. attribute:: Connection.module

    This write-only attribute sets the module column in the v$session table.
    The maximum length for this string is 48 and if you exceed this length you
    will get ORA-24960.

    .. note:

        This attribute is an extension to the DB API definition.


.. method:: Connection.msgproperties()

    Returns an object specifying the properties of messages used in advanced
    queuing. See :ref:`msgproperties` for more information.

    .. versionadded:: 5.3

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.nencoding

    This read-only attribute returns the IANA character set name of the
    national character set in use by the Oracle client.

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


.. method:: Connection.rollback()

    Rollback any pending transactions.


.. method:: Connection.shutdown([mode])

    Shutdown the database. In order to do this the connection must be connected
    as :data:`~cx_Oracle.SYSDBA` or :data:`~cx_Oracle.SYSOPER`. Two calls must
    be made unless the mode specified is :data:`~cx_Oracle.DBSHUTDOWN_ABORT`.
    An example is shown below:

    ::

        import cx_Oracle

        connection = cx_Oracle.Connection(mode = cx_Oracle.SYSDBA)
        connection.shutdown(mode = cx_Oracle.DBSHUTDOWN_IMMEDIATE)
        cursor = connection.cursor()
        cursor.execute("alter database close normal")
        cursor.execute("alter database dismount")
        connection.shutdown(mode = cx_Oracle.DBSHUTDOWN_FINAL)

    .. note::

        This method is an extension to the DB API definition.


.. method:: Connection.startup(force=False, restrict=False)

    Startup the database. This is equivalent to the SQL\*Plus command "startup
    nomount". The connection must be connected as :data:`~cx_Oracle.SYSDBA` or
    :data:`~cx_Oracle.SYSOPER` with the :data:`~cx_Oracle.PRELIM_AUTH` option
    specified for this to work. An example is shown below:

    ::

        import cx_Oracle

        connection = cx_Oracle.Connection(
                mode = cx_Oracle.SYSDBA | cx_Oracle.PRELIM_AUTH)
        connection.startup()
        connection = cx_Oracle.connect(mode = cx_Oracle.SYSDBA)
        cursor = connection.cursor()
        cursor.execute("alter database mount")
        cursor.execute("alter database open")

    .. note::

        This method is an extension to the DB API definition.


.. attribute:: Connection.stmtcachesize

    This read-write attribute specifies the size of the statement cache. This
    value can make a significant difference in performance (up to 100x) if you
    have a small number of statements that you execute repeatedly.

    .. note::

        This attribute is an extension to the DB API definition.


.. method:: Connection.subscribe(namespace=cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE, protocol=cx_Oracle.SUBSCR_PROTO_OCI, callback=None, timeout=0, operations=OPCODE_ALLOPS, port=0, qos=0)

    Return a new :ref:`subscription object <subscrobj>` using the connection.
    Currently the namespace and protocol parameters cannot have any other
    meaningful values.

    The callback is expected to be a callable that accepts a single parameter.
    A :ref:`message object <msgobjects>` is passed to this callback whenever a
    notification is received.

    The timeout value specifies that the subscription expires after the given
    time in seconds. The default value of 0 indicates that the subscription
    never expires.

    The operations parameter enables filtering of the messages that are sent
    (insert, update, delete). The default value will send notifications for all
    operations.

    The port specifies the listening port for callback notifications from the
    database server. If not specified, an unused port will be selected by the
    database.

    The qos parameter specifies quality of service options. It should be one or
    more of the following flags, OR'ed together:
    :data:`cx_Oracle.SUBSCR_QOS_RELIABLE`,
    :data:`cx_Oracle.SUBSCR_QOS_DEREG_NFY`,
    :data:`cx_Oracle.SUBSCR_QOS_ROWIDS`,
    :data:`cx_Oracle.SUBSCR_QOS_QUERY`,
    :data:`cx_Oracle.SUBSCR_QOS_BEST_EFFORT`.

    .. note::

        This method is an extension to the DB API definition.

    .. note::

        Do not close the connection before the subscription object is deleted
        or the subscription object will not be deregistered in the database.
        This is done automatically if connection.close() is never called.


.. attribute:: Connection.tnsentry

    This read-only attribute returns the TNS entry of the database to which a
    connection has been established.

    .. note::

        This attribute is an extension to the DB API definition.


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

