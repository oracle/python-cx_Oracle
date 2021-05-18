.. _deprecations:

************
Deprecations
************

The following tables contains all of the deprecations in the cx_Oracle API,
when they were first deprecated and a comment on what should be used instead,
if applicable. The most recent deprecations are listed first.

.. list-table:: Deprecated in 8.2
    :header-rows: 1
    :widths: 25 75
    :width: 100%
    :name: _deprecations_8_2

    * - Name
      - Comments
    * - `encoding` parameter to :meth:`cx_Oracle.connect()`
      - No longer needed as the use of encodings other than UTF-8 is
        deprecated. Encoding is handled internally between cx_Oracle and Oracle
        Database.
    * - `nencoding` parameter to :meth:`cx_Oracle.connect()`
      - No longer needed as the use of encodings other than UTF-8 is
        deprecated.
    * - `encoding` parameter to :meth:`cx_Oracle.SessionPool()`
      - No longer needed as the use of encodings other than UTF-8 is
        deprecated.
    * - `nencoding` parameter to :meth:`cx_Oracle.SessionPool()`
      - No longer needed as the use of encodings other than UTF-8 is
        deprecated.
    * - Connection.maxBytesPerCharacter
      - No longer needed as the use of encodings other than UTF-8 is
        deprecated. The constant value 4 can be used instead.
    * - Positional parameters to :meth:`cx_Oracle.connect()`
      - Replace with keyword parameters in order to comply with the Python
        database API.
    * - Positional parameters to :meth:`cx_Oracle.SessionPool()`
      - Replace with keyword parameters in order to comply with the Python
        database API.
    * - `threaded` parameter to :meth:`cx_Oracle.SessionPool()`
      - The value of this parameter is ignored. Threading is now always used.
    * - `waitTimeout` parameter to :meth:`cx_Oracle.SessionPool()`
      - Replace with parameter name `wait_timeout`
    * - `maxLifetimeSession` parameter to :meth:`cx_Oracle.SessionPool()`
      - Replace with parameter name `max_lifetime_session`
    * - `sessionCallback` parameter to :meth:`cx_Oracle.SessionPool()`
      - Replace with parameter name `session_callback`
    * - `maxSessionsPerShard` parameter to :meth:`cx_Oracle.SessionPool()`
      - Replace with parameter name `max_sessions_per_shard`
    * - `SessionPool.tnsentry`
      - Replace with :data:`SessionPool.dsn`
    * - `payloadType` parameter to :meth:`Connection.queue()`
      - Replace with parameter name `payload_type` if using keyword parameters.
    * - `ipAddress` parameter to :meth:`Connection.subscribe()`
      - Replace with parameter name `ip_address`
    * - `groupingClass` parameter to :meth:`Connection.subscribe()`
      - Replace with parameter name `grouping_class`
    * - `groupingValue` parameter to :meth:`Connection.subscribe()`
      - Replace with parameter name `grouping_value`
    * - `groupingType` parameter to :meth:`Connection.subscribe()`
      - Replace with parameter name `grouping_type`
    * - `clientInitiated` parameter to :meth:`Connection.subscribe()`
      - Replace with parameter name `client_initiated`
    * - `Connection.callTimeout`
      - Replace with :data:`Connection.call_timeout`
    * - `Connection.tnsentry`
      - Replace with :data:`Connection.dsn`
    * - `keywordParameters` parameter to :meth:`Cursor.callfunc()`
      - Replace with parameter name `keyword_parameters`
    * - `keywordParameters` parameter to :meth:`Cursor.callproc()`
      - Replace with parameter name `keyword_parameters`
    * - `encodingErrors` parameter to :meth:`Cursor.var()`
      - Replace with parameter name `encoding_errors`
    * - `Cursor.fetchraw()`
      - Replace with :meth:`Cursor.fetchmany()`
    * - `newSize` parameter to :meth:`LOB.trim()`
      - Replace with parameter name `new_size`
    * - `Queue.deqMany`
      - Replace with :meth:`Queue.deqmany()`
    * - `Queue.deqOne`
      - Replace with :meth:`Queue.deqone()`
    * - `Queue.enqMany`
      - Replace with :meth:`Queue.enqmany()`
    * - `Queue.enqOne`
      - Replace with :meth:`Queue.enqone()`
    * - `Queue.deqOptions`
      - Replace with :data:`Queue.deqoptions`
    * - `Queue.enqOptions`
      - Replace with :meth:`Queue.enqoptions`
    * - `Queue.payloadType`
      - Replace with :meth:`Queue.payload_type`
    * - `Subscription.ipAddress`
      - Replace with :attr:`Subscription.ip_address`
    * - `Message.consumerName`
      - Replace with :attr:`Message.consumer_name`
    * - `Message.queueName`
      - Replace with :attr:`Message.queue_name`
    * - `Variable.actualElements`
      - Replace with :attr:`Variable.actual_elements`
    * - `Variable.bufferSize`
      - Replace with :attr:`Variable.buffer_size`
    * - `Variable.numElements`
      - Replace with :attr:`Variable.num_elements`


.. list-table:: Deprecated in 8.0
    :header-rows: 1
    :widths: 25 75
    :width: 100%
    :name: _deprecations_8_0

    * - Name
      - Comments
    * - cx_Oracle.BFILE
      - Replace with :data:`cx_Oracle.DB_TYPE_BFILE`
    * - cx_Oracle.BLOB
      - Replace with :data:`cx_Oracle.DB_TYPE_BLOB`
    * - cx_Oracle.BOOLEAN
      - Replace with :data:`cx_Oracle.DB_TYPE_BOOLEAN`
    * - cx_Oracle.CLOB
      - Replace with :data:`cx_Oracle.DB_TYPE_CLOB`
    * - cx_Oracle.CURSOR
      - Replace with :data:`cx_Oracle.DB_TYPE_CURSOR`
    * - cx_Oracle.FIXED_CHAR
      - Replace with :data:`cx_Oracle.DB_TYPE_CHAR`
    * - cx_Oracle.FIXED_NCHAR
      - Replace with :data:`cx_Oracle.DB_TYPE_NCHAR`
    * - cx_Oracle.INTERVAL
      - Replace with :data:`cx_Oracle.DB_TYPE_INTERVAL_DS`
    * - cx_Oracle.LONG_BINARY
      - Replace with :data:`cx_Oracle.DB_TYPE_LONG_RAW`
    * - cx_Oracle.LONG_STRING
      - Replace with :data:`cx_Oracle.DB_TYPE_LONG`
    * - cx_Oracle.NATIVE_FLOAT
      - Replace with :data:`cx_Oracle.DB_TYPE_BINARY_DOUBLE`
    * - cx_Oracle.NATIVE_INT
      - Replace with :data:`cx_Oracle.DB_TYPE_BINARY_INTEGER`
    * - cx_Oracle.NCHAR
      - Replace with :data:`cx_Oracle.DB_TYPE_NVARCHAR`
    * - cx_Oracle.NCLOB
      - Replace with :data:`cx_Oracle.DB_TYPE_NCLOB`
    * - cx_Oracle.OBJECT
      - Replace with :data:`cx_Oracle.DB_TYPE_OBJECT`
    * - cx_Oracle.TIMESTAMP
      - Replace with :data:`cx_Oracle.DB_TYPE_TIMESTAMP`


.. list-table:: Deprecated in 7.2
    :header-rows: 1
    :widths: 25 75
    :width: 100%
    :name: _deprecations_7_2

    * - Name
      - Comments
    * - Connection.deq()
      - Replace with :meth:`Queue.deqone()` or :meth:`Queue.deqmany()`.
    * - Connection.deqoptions()
      - Replace with attribute :attr:`Queue.deqoptions`.
    * - Connection.enq()
      - Replace with :meth:`Queue.enqone()` or :meth:`Queue.enqmany()`.
    * - Connection.enqoptions()
      - Replace with attribute :attr:`Queue.enqoptions`.


.. list-table:: Deprecated in 6.4
    :header-rows: 1
    :widths: 25 75
    :width: 100%
    :name: _deprecations_6_4

    * - Name
      - Comments
    * - Cursor.executemanyprepared()
      - Replace with :meth:`~Cursor.executemany()` with None for the statement
        argument and an integer for the parameters argument.
