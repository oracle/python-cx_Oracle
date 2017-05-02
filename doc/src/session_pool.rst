.. _sesspool:

******************
SessionPool Object
******************

.. note::

   This object is an extension to the DB API.


.. method:: SessionPool.acquire(user=None, password=None, cclass=None, purity=cx_Oracle.ATTR_PURITY_DEFAULT)

   Acquire a connection from the session pool and return a
   :ref:`connection object <connobj>`.

   The user and password arguments may not be specified if the pool is
   homogeneous. In that case an exception will be raised.

   The cclass argument, if specified, should be a string corresponding to the
   connection class for database resident connection pooling (DRCP).

   The purity argument is expected to be one of
   :data:`~cx_Oracle.ATTR_PURITY_NEW`, :data:`~cx_Oracle.ATTR_PURITY_SELF`, or
   :data:`~cx_Oracle.ATTR_PURITY_DEFAULT`.


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


.. method:: SessionPool.release(connection)

   Release the connection back to the pool. This will be done automatically as
   well if the connection object is garbage collected.


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

