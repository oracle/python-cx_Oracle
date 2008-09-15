.. _sesspool:

******************
SessionPool Object
******************

.. note::

   This object is an extension the DB API and is only available in Oracle 9i.


.. method:: SessionPool.acquire()

   Acquire a connection from the session pool and return a connection object
   (:ref:`connobj`).


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


.. attribute:: SessionPool.password

   This read-only attribute returns the password of the user which established
   the connection to the database.


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

