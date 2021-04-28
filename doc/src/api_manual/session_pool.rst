.. _sesspool:

******************
SessionPool Object
******************

.. note::

    This object is an extension to the DB API.

    Connection pooling in cx_Oracle is handled by SessionPool objects.

    See :ref:`connpool` for information on connection pooling.


.. method:: SessionPool.acquire(user=None, password=None, cclass=None, \
        purity=cx_Oracle.ATTR_PURITY_DEFAULT, tag=None, matchanytag=False, \
        shardingkey=[], supershardingkey=[])

    Acquire a connection from the session pool and return a
    :ref:`connection object <connobj>`.

    If the pool is homogeneous, the user and password parameters cannot be
    specified. If they are, an exception will be raised.

    The cclass parameter, if specified, should be a string corresponding to the
    connection class for database resident connection pooling (DRCP).

    The purity parameter is expected to be one of
    :data:`~cx_Oracle.ATTR_PURITY_NEW`, :data:`~cx_Oracle.ATTR_PURITY_SELF`, or
    :data:`~cx_Oracle.ATTR_PURITY_DEFAULT`.

    The tag parameter, if specified, is expected to be a string with name=value
    pairs like "k1=v1;k2=v2" and will limit the sessions that can be returned
    from a session pool unless the matchanytag parameter is set to True. In
    that case sessions with the specified tag will be preferred over others,
    but if no such sessions are available a session with a different tag may be
    returned instead. In any case, untagged sessions will always be returned if
    no sessions with the specified tag are available.  Sessions are tagged when
    they are :meth:`released <SessionPool.release>` back to the pool.

    The shardingkey and supershardingkey parameters, if specified, are expected
    to be a sequence of values which will be used to identify the database
    shard to connect to.  The key values can be strings, numbers, bytes or
    dates.


.. attribute:: SessionPool.busy

    This read-only attribute returns the number of sessions currently acquired.


.. method:: SessionPool.close(force=False)

    Close the session pool now, rather than when the last reference to it is
    released, which makes it unusable for further work.

    If any connections have been acquired and not released back to the pool
    this method will fail unless the force parameter is set to True.


.. method:: SessionPool.drop(connection)

    Drop the connection from the pool which is useful if the connection is no
    longer usable (such as when the session is killed).


.. attribute:: SessionPool.dsn

    This read-only attribute returns the TNS entry of the database to which a
    connection has been established.


.. attribute:: SessionPool.homogeneous

    This read-write boolean attribute indicates whether the pool is considered
    homogeneous or not. If the pool is not homogeneous different authentication
    can be used for each connection acquired from the pool.


.. attribute:: SessionPool.increment

    This read-only attribute returns the number of sessions that will be
    established when additional sessions need to be created.


.. attribute:: SessionPool.max

    This read-only attribute returns the maximum number of sessions that the
    session pool can control.


.. attribute:: SessionPool.max_lifetime_session

    This read-write attribute returns the maximum length of time (in seconds)
    that a pooled session may exist. Sessions that are in use will not be
    closed. They become candidates for termination only when they are released
    back to the pool and have existed for longer than max_lifetime_session
    seconds. Note that termination only occurs when the pool is accessed. A
    value of 0 means that there is no maximum length of time that a pooled
    session may exist. This attribute is only available in Oracle Database
    12.1.

    .. versionadded:: 5.3


.. attribute:: SessionPool.min

    This read-only attribute returns the number of sessions with which the
    session pool was created and the minimum number of sessions that will be
    controlled by the session pool.


.. attribute:: SessionPool.name

    This read-only attribute returns the name assigned to the session pool by
    Oracle.


.. attribute:: SessionPool.opened

    This read-only attribute returns the number of sessions currently opened by
    the session pool.


.. attribute:: SessionPool.ping_interval

    This read-write integer attribute specifies the pool ping interval in
    seconds. When a connection is acquired from the pool, a check is first made
    to see how long it has been since the connection was put into the pool. If
    this idle time exceeds ``ping_interval``, then a :ref:`round-trip <roundtrips>`
    ping to the database is performed. If the connection is unusable, it is
    discarded and a different connection is selected to be returned by
    :meth:`SessionPool.acquire()`.  Setting ``ping_interval`` to a negative
    value disables pinging.  Setting it to 0 forces a ping for every
    ``aquire()`` and is not recommended.

    Prior to cx_Oracle 8.2, the ping interval was fixed at 60 seconds.

    .. versionadded:: 8.2


.. method:: SessionPool.release(connection, tag=None)

    Release the connection back to the pool now, rather than whenever __del__
    is called. The connection will be unusable from this point forward; an
    Error exception will be raised if any operation is attempted with the
    connection. Any cursors or LOBs created by the connection will also be
    marked unusable and an Error exception will be raised if any operation is
    attempted with them.

    Internally, references to the connection are held by cursor objects,
    LOB objects, etc. Once all of these references are released, the connection
    itself will be released back to the pool automatically. Either control
    references to these related objects carefully or explicitly release
    connections back to the pool in order to ensure sufficient resources are
    available.

    If the tag is not None, it is expected to be a string with name=value pairs
    like "k1=v1;k2=v2" and will override the value in the property
    :attr:`Connection.tag`. If either :attr:`Connection.tag` or the tag
    parameter are not None, the connection will be retagged when it is released
    back to the pool.


.. attribute:: SessionPool.soda_metadata_cache

    This read-write boolean attribute returns whether the SODA metadata cache
    is enabled or not. Enabling the cache significantly improves the
    performance of methods :meth:`SodaDatabase.createCollection()` (when not
    specifying a value for the metadata parameter) and
    :meth:`SodaDatabase.openCollection()`. Note that the cache can become out
    of date if changes to the metadata of cached collections are made
    externally.

    .. versionadded:: 8.2


.. attribute:: SessionPool.stmtcachesize

    This read-write attribute specifies the size of the statement cache that
    will be used for connections obtained from the pool.

    See :ref:`Statement Caching <stmtcache>` for more information.

    .. versionadded:: 6.0


.. attribute:: SessionPool.timeout

    This read-write attribute specifies the time (in seconds) after which idle
    sessions will be terminated in order to maintain an optimum number of open
    sessions. Note that termination only occurs when the pool is accessed. A
    value of 0 means that no idle sessions are terminated.


.. attribute:: SessionPool.tnsentry

    This read-only attribute returns the TNS entry of the database to which a
    connection has been established.

    .. deprecated:: 8.2

        Use the attribute :attr:`~SessionPool.dsn` instead.


.. attribute:: SessionPool.username

    This read-only attribute returns the name of the user which established the
    connection to the database.


.. attribute:: SessionPool.wait_timeout

    This read-write attribute specifies the time (in milliseconds) that the
    caller should wait for a session to become available in the pool before
    returning with an error. This value is only used if the getmode parameter
    to :meth:`cx_Oracle.SessionPool()` was the value
    :data:`cx_Oracle.SPOOL_ATTRVAL_TIMEDWAIT`.

    .. versionadded:: 6.4
