.. _sesspool:

******************
SessionPool Object
******************

.. note::

   This object is an extension to the DB API.


.. method:: SessionPool.acquire(user=None, password=None, cclass=None, purity=cx_Oracle.ATTR_PURITY_DEFAULT, tag=None, matchanytag=False)

   Acquire a connection from the session pool and return a
   :ref:`connection object <connobj>`.

   The user and password arguments may not be specified if the pool is
   homogeneous. In that case an exception will be raised.

   The cclass argument, if specified, should be a string corresponding to the
   connection class for database resident connection pooling (DRCP).

   The purity argument is expected to be one of
   :data:`~cx_Oracle.ATTR_PURITY_NEW`, :data:`~cx_Oracle.ATTR_PURITY_SELF`, or
   :data:`~cx_Oracle.ATTR_PURITY_DEFAULT`.

   The tag argument, if specified, is expected to be a string and will limit
   the sessions that can be returned from a session pool unless the matchanytag
   argument is set to True. In that case sessions with the specified tag will
   be preferred over others, but if no such sessions are available a session
   with a different tag may be returned instead. In any case, untagged sessions
   will always be returned if no sessions with the specified tag are available.
   Sessions are tagged when they are :meth:`released <SessionPool.release>`
   back to the pool.


.. attribute:: SessionPool.busy

   This read-only attribute returns the number of sessions currently acquired.


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

   This read-write attribute returns the lifetime (in seconds) for all of the
   sessions in the pool. Sessions in the pool are terminated when they have
   reached their lifetime. If timeout is also set, the session will be
   terminated if either the idle timeout happens or the max lifetime setting
   is exceeded. This attribute is only available in Oracle Database 12.1.

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


.. method:: SessionPool.release(connection, tag=None)

   Release the connection back to the pool now, rather than whenever __del__ is
   called. The connection will be unusable from this point forward; an Error
   exception will be raised if any operation is attempted with the connection.

   Before the connection can be released back to the pool, all cursors created
   by the connection must first be closed or all references released. In
   addition, all LOB objects created by the connection must have their
   references released. If this has not been done, the exception "DPI-1054:
   connection cannot be closed when open statements or LOBs exist" will be
   raised.

   Internally, references to the connection are held by cursor objects,
   LOB objects, subscription objects, etc. Once all of these references are
   released, the connection itself will be released back to the pool
   automatically. Either control references to these related objects carefully
   or explicitly release connections back to the pool in order to ensure
   sufficient resources are available.


.. attribute:: SessionPool.stmtcachesize

   This read-write attribute specifies the size of the statement cache that
   will be used as the starting point for any connections that are created by
   the session pool. Once created, the connection's statement cache size can
   only be changed by setting the stmtcachesize attribute on the connection
   itself.

   .. versionadded:: 6.0


.. attribute:: SessionPool.timeout

   This read-write attribute indicates the time (in seconds) after which idle
   sessions will be terminated in order to maintain an optimum number of open
   sessions.


.. attribute:: SessionPool.tnsentry

   This read-only attribute returns the TNS entry of the database to which a
   connection has been established.


.. attribute:: SessionPool.username

   This read-only attribute returns the name of the user which established the
   connection to the database.

