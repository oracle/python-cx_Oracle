:orphan:

.. _releasenotes:

cx_Oracle Release Notes
=======================

Version 8.2 (TBD)
-----------------

#)  Updated embedded ODPI-C to `version 4.2.0
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-4-2-tbd>`__.
#)  Added :ref:`SODA metadata cache <sodametadatacache>` support to connection
    pools. This significantly improves the performance of methods
    :meth:`SodaDatabase.createCollection()` (when not specifying a value for
    the metadata parameter) and :meth:`SodaDatabase.openCollection()`. Caching
    is available when using Oracle Client version 19.11 and higher.
#)  Added support for supplying hints to SODA operations. A new non-terminal
    method :meth:`~SodaOperation.hint()` was added and a `hint` parameter was
    added to the methods :meth:`SodaCollection.insertOneAndGet()`,
    :meth:`SodaCollection.insertManyAndGet()` and
    :meth:`SodaCollection.saveAndGet()`. All of these require Oracle Client
    21.3 or higher (or Oracle Client 19 from 19.11).
#)  Added parameter `stmtcachesize` to :meth:`cx_Oracle.connect()` and
    :meth:`cx_Oracle.SessionPool()` in order to permit specifying the size of
    the statement cache during the creation of pools and standalone
    connections.
#)  Added parameter `ping_interval` to :meth:`cx_Oracle.SessionPool()` to specify
    the ping interval when acquiring pooled connections. In addition, the
    attribute :data:`SessionPool.ping_interval` was added in order to permit
    making adjustments after the pool has been created.  In previous cx_Oracle
    releases a fixed ping interval of 60 seconds was used.
#)  Added parameter `bypass_decode` to :meth:`Cursor.var()` in order to allow
    the `decode` step to be bypassed when converting data from Oracle Database
    into Python strings
    (`issue 385 <https://github.com/oracle/python-cx_Oracle/issues/385>`__).
    Initial work was done in `PR 549
    <https://github.com/oracle/python-cx_Oracle/pull/549>`__.
#)  Threaded mode is now always enabled when creating connection pools with
    :meth:`cx_Oracle.SessionPool()`. Any `threaded` parameter value is ignored.
#)  Eliminated a memory leak when calling :meth:`SodaOperation.filter()` with a
    dictionary.
#)  The distributed transaction handle assosciated with the connection is now
    cleared on commit or rollback (`issue 530
    <https://github.com/oracle/python-cx_Oracle/issues/530>`__).
#)  Added check to ensure that when setting variables or object attributes, the
    type of the temporary LOB must match the expected type.
#)  A small number of parameter, method, and attribute names were updated to
    follow the PEP 8 style guide. This brings better consistency to the
    cx_Oracle API. The old names are still usable but may be removed in a
    future release of cx_Oracle. See :ref:`_deprecations_8_2` for details.
#)  Improved the test suite.


Version 8.1 (December 2020)
---------------------------

#)  Updated embedded ODPI-C to `version 4.1.0
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-4-1-december-8-2020>`__.
#)  Added support for new JSON data type available in Oracle Client and
    Database 21 and higher.
#)  Dropped support for Python 3.5. Added support for Python 3.9.
#)  Added internal methods for getting/setting OCI attributes that are
    otherwise not supported by cx_Oracle. These methods should only be used as
    directed by Oracle.
#)  Minor code improvement supplied by Alex Henrie
    (`PR 472 <https://github.com/oracle/python-cx_Oracle/pull/472>`__).
#)  Builds are now done with setuptools and most metadata has moved from
    `setup.py` to `setup.cfg` in order to take advantage of Python packaging
    improvements.
#)  The ability to pickle/unpickle Database and API types has been restored.
#)  Tests can now be run with tox in order to automate testing of the different
    environments that are supported.
#)  The value of prefetchrows for REF CURSOR variables is now honored.
#)  Improved documentation, samples and test suite.


Version 8.0.1 (August 2020)
---------------------------

#)  Updated embedded ODPI-C to `version 4.0.2
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-4-0-2-august-31-2020>`__. This includes the fix for binding and
    fetching numbers with 39 or 40 decimal digits
    (`issue 459 <https://github.com/oracle/python-cx_Oracle/issues/459>`__).
#)  Added build metadata specifying that Python 3.5 and higher is required in
    order to avoid downloading and failing to install with Python 2. The
    exception message when running ``setup.py`` directly was updated to inform
    those using Python 2 to use version 7.3 instead.
#)  Documentation improvements.


Version 8.0 (June 2020)
-----------------------

#)  Dropped support for Python 2. For those still requiring Python 2, see
    :ref:`python2`.
#)  Updated embedded ODPI-C to `version 4.0.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-4-0-1-june-26-2020>`__.
#)  Reworked type management to clarify and simplify code

    - Added :ref:`constants <dbtypes>` for all database types. The database
      types :data:`cx_Oracle.DB_TYPE_BINARY_FLOAT`,
      :data:`cx_Oracle.DB_TYPE_INTERVAL_YM`,
      :data:`cx_Oracle.DB_TYPE_TIMESTAMP_LTZ` and
      :data:`cx_Oracle.DB_TYPE_TIMESTAMP_TZ` are completely new. The other
      types were found in earlier releases under a different name. These types
      will be found in :data:`Cursor.description` and passed as the defaultType
      parameter to the :data:`Connection.outputtypehandler` and
      :data:`Cursor.outputtypehandler` functions.
    - Added :ref:`synonyms <dbtypesynonyms>` from the old type names to the new
      type names for backwards compatibility. They are deprecated and will be
      removed in a future version of cx_Oracle.
    - The DB API :ref:`constants <types>` are now a specialized constant that
      matches to the corresponding database types, as recommended by the DB
      API.
    - The variable attribute :data:`~Variable.type` now refers to one of the
      new database type constants if the variable does not contain objects
      (previously it was None in that case).
    - The attribute :data:`~LOB.type` was added to LOB values.
    - The attribute :data:`~ObjectAttribute.type` was added to attributes of
      object types.
    - The attribute :data:`~ObjectType.element_type` was added to object types.
    - :ref:`Object types <objecttype>` now compare equal if they were created
      by the same connection or session pool and their schemas and names match.
    - All variables are now instances of the same class (previously each type
      was an instance of a separate variable type). The attribute
      :data:`~Variable.type` can be examined to determine the database type it
      is associated with.
    - The string representation of variables has changed to include the type
      in addition to the value.

#)  Added function :meth:`cx_Oracle.init_oracle_client()` in order to enable
    programmatic control of the initialization of the Oracle Client library.
#)  The default encoding for all character data is now UTF-8 and any character
    set specified in the environment variable ``NLS_LANG`` is ignored.
#)  Added functions :meth:`SodaCollection.save()`,
    :meth:`SodaCollection.saveAndGet()` and :meth:`SodaCollection.truncate()`
    available in Oracle Client 20 and higher.
#)  Added function :meth:`SodaOperation.fetchArraySize()` available in Oracle
    Client 19.5 and higher.
#)  Added attribute :attr:`Cursor.prefetchrows` to control the number of rows
    that the Oracle Client library fetches into internal buffers when a query
    is executed.
#)  Internally make use of new mode available in Oracle Client 20 and higher in
    order to avoid a round-trip when accessing :attr:`Connection.version` for
    the first time.
#)  Added support for starting up a database using a parameter file (PFILE),
    as requested
    (`issue 295 <https://github.com/oracle/python-cx_Oracle/issues/295>`__).
#)  Fixed overflow issue when calling :meth:`Cursor.getbatcherrors()` with
    row offsets exceeding 65536.
#)  Eliminated spurious error when accessing :attr:`Cursor.lastrowid` after
    executing an INSERT ALL statement.
#)  Miscellaneous improvements supplied by Alex Henrie (pull requests
    `419 <https://github.com/oracle/python-cx_Oracle/pull/419>`__,
    `420 <https://github.com/oracle/python-cx_Oracle/pull/420>`__,
    `421 <https://github.com/oracle/python-cx_Oracle/pull/421>`__,
    `422 <https://github.com/oracle/python-cx_Oracle/pull/422>`__,
    `423 <https://github.com/oracle/python-cx_Oracle/pull/423>`__,
    `437 <https://github.com/oracle/python-cx_Oracle/pull/437>`__ and
    `438 <https://github.com/oracle/python-cx_Oracle/pull/438>`__).
#)  Python objects bound to boolean variables are now converted to True or
    False based on whether they would be considered True or False in a Python
    if statement. Previously, only True was treated as True and all other
    Python values (including 1, 1.0, and "foo") were treated as False
    (pull request
    `435 <https://github.com/oracle/python-cx_Oracle/pull/435>`__).
#)  Documentation, samples and test suite improvements.


Version 7.3 (December 2019)
---------------------------

#)  Added support for Python 3.8.
#)  Updated embedded ODPI-C to `version 3.3
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-3-december-2-2019>`__.
#)  Added support for CQN and other subscription client initiated connections
    to the database (as opposed to the default server initiated connections)
    created by calling :meth:`Connection.subscribe()`.
#)  Added :attr:`support <Cursor.lastrowid>` for returning the rowid of the
    last row modified by an operation on a cursor (or None if no row was
    modified).
#)  Added support for setting the maxSessionsPerShard attribute when
    :meth:`creating session pools <cx_Oracle.SessionPool>`.
#)  Added check to ensure sharding key is specified when a super sharding key
    is specified.
#)  Improved error message when the Oracle Client library is loaded
    successfully but the attempt to detect the version of that library fails,
    either due to the fact that the library is too old or the method could not
    be called for some reason (`node-oracledb issue 1168
    <https://github.com/oracle/node-oracledb/issues/1168>`__).
#)  Adjusted support for creating a connection using an existing OCI service
    context handle. In order to avoid potential memory corruption and
    unsupported behaviors, the connection will now use the same encoding as the
    existing OCI service context handle when it was created.
#)  Added ``ORA-3156: OCI call timed out`` to the list of error messages that
    result in error DPI-1067.
#)  Adjusted samples and the test suite so that they can be run against Oracle
    Cloud databases.
#)  Fixed bug when attempting to create a scrollable cursor on big endian
    platforms like AIX on PPC.
#)  Eliminated reference leak and ensure that memory is properly initialized in
    case of error when using sharding keys.
#)  Eliminated reference leak when splitting the password and DSN components
    out of a full connect string.
#)  Corrected processing of DATE sharding keys (sharding requires a slightly
    different format to be passed to the server).
#)  Eliminated reference leak when
    :meth:`creating message property objects <Connection.msgproperties()>`.
#)  Attempting to use proxy authentication with a homogeneous pool will now
    raise a ``DatabaseError`` exception with the message
    ``DPI-1012: proxy authentication is not possible with homogeneous pools``
    instead of a ``ProgrammingError`` exception with the message
    ``pool is homogeneous. Proxy authentication is not possible.`` since this
    check is done by ODPI-C. An empty string (or None) for the user name will
    no longer generate an exception.
#)  Exception ``InterfaceError: not connected`` is now always raised when an
    operation is attempted with a closed connection. Previously, a number of
    different exceptions were raised depending on the operation.
#)  Added ``ORA-40479: internal JSON serializer error`` to the list of
    exceptions that result in ``cx_Oracle.IntegrityError``.
#)  Improved documentation.


Version 7.2.3 (October 2019)
----------------------------

#)  Updated embedded ODPI-C to `version 3.2.2
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-2-2-october-1-2019>`__.
#)  Restored support for setting numeric bind variables with boolean values.
#)  Ensured that sharding keys are dedicated to the connection that is acquired
    using them in order to avoid possible hangs, crashes or unusual errors.
#)  Corrected support for PLS_INTEGER and BINARY_INTEGER types when used in
    PL/SQL records
    (`ODPI-C issue 112 <https://github.com/oracle/odpi/issues/112>`__).
#)  Improved documentation.


Version 7.2.2 (August 2019)
---------------------------

#)  Updated embedded ODPI-C to `version 3.2.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-2-1-august-12-2019>`__.
#)  A more meaningful error is now returned when calling
    :meth:`SodaCollection.insertMany()` with an empty list.
#)  A more meaningful error is now returned when calling
    :meth:`Subscription.registerquery()` with SQL that is not a SELECT
    statement.
#)  Eliminated segfault when a connection is closed after being created by a
    call to :meth:`cx_Oracle.connect()` with the parameter ``cclass`` set to
    a non-empty string.
#)  Added user guide documentation.
#)  Updated default connect strings to use 19c and XE 18c defaults.


Version 7.2.1 (July 2019)
-------------------------

#)  Resolved ``MemoryError`` exception on Windows when using an output type
    handler
    (`issue 330 <https://github.com/oracle/python-cx_Oracle/issues/330>`__).
#)  Improved test suite and samples.
#)  Improved documentation.


Version 7.2 (July 2019)
-----------------------

#)  Updated embedded ODPI-C to `version 3.2
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-2-july-1-2019>`__.
#)  Improved AQ support

    - added support for enqueue and dequeue of RAW payloads
    - added support for bulk enqueue and dequeue of messages
    - added new method :meth:`Connection.queue()` which creates a new
      :ref:`queue object <queue>` in order to simplify AQ usage
    - enhanced method :meth:`Connection.msgproperties()` to allow the writable
      properties of the newly created object to be initialized.
    - the original methods for enqueueing and dequeuing (Connection.deq(),
      Connection.deqoptions(), Connection.enq() and Connection.enqoptions())
      are now deprecated and will be removed in a future version.

#)  Removed preview status from existing SODA functionality. See
    `this tracking issue
    <https://github.com/oracle/python-cx_Oracle/issues/309>`__ for known issues
    with SODA.
#)  Added support for a preview of SODA bulk insert, available in Oracle Client
    18.5 and higher.
#)  Added support for setting LOB object attributes, as requested
    (`issue 299 <https://github.com/oracle/python-cx_Oracle/issues/299>`__).
#)  Added mode :data:`cx_Oracle.DEFAULT_AUTH` as requested
    (`issue 293 <https://github.com/oracle/python-cx_Oracle/issues/293>`__).
#)  Added support for using the LOB prefetch length indicator in order to
    reduce the number of round trips when fetching LOBs and then subsequently
    calling :meth:`LOB.size()`, :meth:`LOB.getchunksize()` or
    :meth:`LOB.read()`. This is always enabled.
#)  Added support for types BINARY_INTEGER, PLS_INTEGER, ROWID, LONG and LONG
    RAW when used in PL/SQL.
#)  Eliminated deprecation of attribute :attr:`Subscription.id`. It is now
    populated with the value of ``REGID`` found in the database view
    ``USER_CHANGE_NOTIFICATION_REGS`` or the value of ``REG_ID`` found in the
    database view ``USER_SUBSCR_REGISTRATIONS``. For AQ subscriptions, the
    value is 0.
#)  Enabled PY_SSIZE_T_CLEAN, as required by Python 3.8
    (`issue 317 <https://github.com/oracle/python-cx_Oracle/issues/317>`__).
#)  Eliminated memory leak when fetching objects that are atomically null
    (`issue 298 <https://github.com/oracle/python-cx_Oracle/issues/298>`__).
#)  Eliminated bug when processing the string representation of numbers like
    1e-08 and 1e-09 (`issue 300
    <https://github.com/oracle/python-cx_Oracle/issues/300>`__).
#)  Improved error message when the parent cursor is closed before a fetch is
    attempted from an implicit result cursor.
#)  Improved test suite and samples.
#)  Improved documentation.


Version 7.1.3 (April 2019)
--------------------------

#)  Updated to `ODPI-C 3.1.4
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-1-4-april-24-2019>`__.
#)  Added support for getting the row count for PL/SQL statements
    (`issue 285 <https://github.com/oracle/python-cx_Oracle/issues/285>`__).
#)  Corrected parsing of connect string so that the last @ symbol is searched
    for instead of the first @ symbol; otherwise, passwords containing an @
    symbol will result in the incorrect DSN being extracted
    (`issue 290 <https://github.com/oracle/python-cx_Oracle/issues/290>`__).
#)  Adjusted return value of cursor.callproc() to follow documentation (only
    positional arguments are returned since the order of keyword parameters
    cannot be guaranteed in any case)
    (`PR 287 <https://github.com/oracle/python-cx_Oracle/pull/287>`__).
#)  Corrected code getting sample and test parameters by user input when using
    Python 2.7.


Version 7.1.2 (March 2019)
--------------------------

#)  Updated to `ODPI-C 3.1.3
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-1-3-march-12-2019>`__.
#)  Ensured that the strings "-0" and "-0.0" are correctly handled as zero
    values
    (`issue 274 <https://github.com/oracle/python-cx_Oracle/issues/274>`__).
#)  Eliminated error when startup and shutdown events are generated
    (`ODPI-C issue 102 <https://github.com/oracle/odpi/issues/102>`__).
#)  Enabled the types specified in :meth:`Cursor.setinputsizes()` and
    :meth:`Cursor.callfunc()` to be an object type in addition to a Python
    type, just like in :meth:`Cursor.var()`.
#)  Reverted changes to return decimal numbers when the numeric precision was
    too great to be returned accurately as a floating point number. This change
    had too great an impact on existing functionality and an output type
    handler can be used to return decimal numbers where that is desirable
    (`issue 279 <https://github.com/oracle/python-cx_Oracle/issues/279>`__).
#)  Eliminated discrepancies in character sets between an external connection
    handle and the newly created connection handle that references the external
    connection handle
    (`issue 273 <https://github.com/oracle/python-cx_Oracle/issues/273>`__).
#)  Eliminated memory leak when receiving messages received from subscriptions.
#)  Improved test suite and documentation.


Version 7.1.1 (February 2019)
-----------------------------

#)  Updated to `ODPI-C 3.1.2
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-1-2-february-19-2019>`__.
#)  Corrected code for freeing CQN message objects when multiple queries are
    registered
    (`ODPI-C issue 96 <https://github.com/oracle/odpi/issues/96>`__).
#)  Improved error messages and installation documentation.


Version 7.1 (February 2019)
---------------------------

#)  Updated to `ODPI-C 3.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-1-january-21-2019>`__.
#)  Improved support for session tagging in session pools by allowing a
    session callback to be specified when creating a pool via
    :meth:`cx_Oracle.SessionPool()`. Callbacks can be written in Python or in
    PL/SQL and can be used to improve performance by decreasing round trips to
    the database needed to set session state. Callbacks written in Python will
    be invoked for brand new connections (that have never been acquired from
    the pool before) or when the tag assigned to the connection doesn't match
    the one that was requested. Callbacks written in PL/SQL will only be
    invoked when the tag assigned to the connection doesn't match the one that
    was requested.
#)  Added attribute :attr:`Connection.tag` to provide access to the actual tag
    assigned to the connection. Setting this attribute will cause the
    connection to be retagged when it is released back to the pool.
#)  Added support for fetching SYS.XMLTYPE values as strings, as requested
    (`issue 14 <https://github.com/oracle/python-cx_Oracle/issues/14>`__).
    Note that this support is limited to the size of VARCHAR2 columns in the
    database (either 4000 or 32767 bytes).
#)  Added support for allowing the typename parameter in method
    :meth:`Cursor.var()` to be None or a valid object type created by the
    method :meth:`Connection.gettype()`, as requested
    (`issue 231 <https://github.com/oracle/python-cx_Oracle/issues/231>`__).
#)  Added support for getting and setting attributes of type RAW on Oracle
    objects, as requested
    (`ODPI-C issue 72 <https://github.com/oracle/odpi/issues/72>`__).
#)  Added support for performing external authentication with proxy for
    standalone connections.
#)  Added support for mixing integers, floating point and decimal values in
    data passed to :meth:`Cursor.executemany()`
    (`issue 241 <https://github.com/oracle/python-cx_Oracle/issues/241>`__).
    The error message raised when a value cannot be converted to an Oracle
    number was also improved.
#)  Adjusted fetching of numeric values so that no precision is lost. If an
    Oracle number cannot be represented by a Python floating point number a
    decimal value is automatically returned instead.
#)  Corrected handling of multiple calls to method
    :meth:`Cursor.executemany()` where all of the values in one of the columns
    passed to the first call are all None and a subsequent call has a value
    other than None in the same column
    (`issue 236 <https://github.com/oracle/python-cx_Oracle/issues/236>`__).
#)  Added additional check for calling :meth:`Cursor.setinputsizes()` with an
    empty dictionary in order to avoid the error "cx_Oracle.ProgrammingError:
    positional and named binds cannot be intermixed"
    (`issue 199 <https://github.com/oracle/python-cx_Oracle/issues/199>`__).
#)  Corrected handling of values that exceed the maximum value of a plain
    integer object on Python 2 on Windows
    (`issue 257 <https://github.com/oracle/python-cx_Oracle/issues/257>`__).
#)  Added error message when attempting external authentication with proxy
    without placing the user name in [] (proxy authentication was previously
    silently ignored).
#)  Exempted additional error messages from forcing a statement to be dropped
    from the cache
    (`ODPI-C issue 76 <https://github.com/oracle/odpi/issues/76>`__).
#)  Improved dead session detection when using session pools for Oracle Client
    12.2 and higher.
#)  Ensured that the connection returned from a pool after a failed ping (such
    as due to a killed session) is not itself marked as needing to be dropped
    from the pool.
#)  Eliminated memory leak under certain circumstances when pooled connections
    are released back to the pool.
#)  Eliminated memory leak when connections are dropped from the pool.
#)  Eliminated memory leak when calling :meth:`Connection.close()` after
    fetching collections from the database.
#)  Adjusted order in which memory is freed when the last references to SODA
    collections, documents, document cursors and collection cursors are
    released, in order to prevent a segfault under certain circumstances.
#)  Improved code preventing a statement from binding itself, in order to avoid
    a potential segfault under certain circumstances.
#)  Worked around OCI bug when attempting to free objects that are PL/SQL
    records, in order to avoid a potential segfault.
#)  Improved test suite and samples. Note that default passwords are no longer
    supplied. New environment variables can be set to specify passwords if
    desired, or the tests and samples will prompt for the passwords when
    needed. In addition, a Python script is now available to create and drop
    the schemas used for the tests and samples.
#)  Improved documentation.


Version 7.0 (September 2018)
----------------------------

#)  Update to `ODPI-C 3.0
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-3-0-0-september-13-2018>`__.
#)  Added support for Oracle Client 18 libraries.
#)  Added support for SODA (as preview). See :ref:`SODA Database <sodadb>`,
    :ref:`SODA Collection <sodacoll>` and :ref:`SODA Document <sodadoc>` for
    more information.
#)  Added support for call timeouts available in Oracle Client 18.1 and
    higher. See :attr:`Connection.call_timeout`.
#)  Added support for getting the contents of a SQL collection object as a
    dictionary, where the keys are the indices of the collection and the values
    are the elements of the collection. See function :meth:`Object.asdict()`.
#)  Added support for closing a session pool via the function
    :meth:`SessionPool.close()`. Once closed, further attempts to use any
    connection that was acquired from the pool will result in the error
    "DPI-1010: not connected".
#)  Added support for setting a LOB attribute of an object with a string or
    bytes (instead of requiring a temporary LOB to be created).
#)  Added support for the packed decimal type used by object attributes with
    historical types DECIMAL and NUMERIC
    (`issue 212 <https://github.com/oracle/python-cx_Oracle/issues/212>`__).
#)  On Windows, first attempt to load oci.dll from the same directory as
    the cx_Oracle module.
#)  SQL objects that are created or fetched from the database are now tracked
    and marked unusable when a connection is closed. This was done in order
    to avoid a segfault under certain circumstances.
#)  Re-enabled dead session detection functionality when using pools for Oracle
    Client 12.2 and higher in order to handle classes of connection errors such
    as resource profile limits.
#)  Improved error messages when the Oracle Client or Oracle Database need to
    be at a minimum version in order to support a particular feature.
#)  When a connection is used as a context manager, the connection is now
    closed when the block ends. Attempts to set
    ``cx_Oracle.__future__.ctx_mgr_close`` are now ignored.
#)  When a DML returning statement is executed, variables bound to it will
    return an array when calling :meth:`Variable.getvalue()`. Attempts to set
    ``cx_Oracle.__future__.dml_ret_array_val`` are now ignored.
#)  Support for Python 3.4 has been dropped.
#)  Added additional test cases.
#)  Improved documentation.


Version 6.4.1 (July 2018)
-------------------------

#)  Update to `ODPI-C 2.4.2
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-4-2-july-9-2018>`__.

    - Avoid buffer overrun due to improper calculation of length byte when
      converting some negative 39 digit numbers from string to the internal
      Oracle number format
      (`ODPI-C issue 67 <https://github.com/oracle/odpi/issues/67>`__).

#)  Prevent error "cx_Oracle.ProgrammingError: positional and named binds
    cannot be intermixed" when calling cursor.setinputsizes() without any
    parameters and then calling cursor.execute() with named bind parameters
    (`issue 199 <https://github.com/oracle/python-cx_Oracle/issues/199>`__).


Version 6.4 (July 2018)
-----------------------

#)  Update to `ODPI-C 2.4.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-4-1-july-2-2018>`__.

    - Added support for grouping subscriptions. See parameters groupingClass,
      groupingValue and groupingType to function
      :meth:`Connection.subscribe()`.
    - Added support for specifying the IP address a subscription should use
      instead of having the Oracle Client library determine the IP address on
      its own. See parameter ipAddress to function
      :meth:`Connection.subscribe()`.
    - Added support for subscribing to notifications when messages are
      available to dequeue in an AQ queue. The new constant
      :data:`cx_Oracle.SUBSCR_NAMESPACE_AQ` should be passed to the namespace
      parameter of function :meth:`Connection.subscribe()` in order to get this
      functionality. Attributes :attr:`Message.queueName` and
      :attr:`Message.consumerName` will be populated in notification messages
      that are received when this namespace is used.
    - Added attribute :attr:`Message.registered` to let the notification
      callback know when the subscription that generated the notification is no
      longer registered with the database.
    - Added support for timed waits when acquiring a session from a session
      pool. Use the new constant :data:`cx_Oracle.SPOOL_ATTRVAL_TIMEDWAIT` in
      the parameter getmode to function :meth:`cx_Oracle.SessionPool` along
      with the new parameter waitTimeout.
    - Added support for specifying the timeout and maximum lifetime session for
      session pools when they are created using function
      :meth:`cx_Oracle.SessionPool`. Previously the pool had to be created
      before these values could be changed.
    - Avoid memory leak when dequeuing from an empty queue.
    - Ensure that the row count for queries is reset to zero when the statement
      is executed
      (`issue 193 <https://github.com/oracle/python-cx_Oracle/issues/193>`__).
    - If the statement should be deleted from the statement cache, first check
      to see that there is a statement cache currently being used; otherwise,
      the error "ORA-24300: bad value for mode" will be raised under certain
      conditions.

#)  Added support for using the cursor as a context manager
    (`issue 190 <https://github.com/oracle/python-cx_Oracle/issues/190>`__).
#)  Added parameter "encodingErrors" to function :meth:`Cursor.var()` in order
    to add support for specifying the "errors" parameter to the decode() that
    takes place internally when fetching strings from the database
    (`issue 162 <https://github.com/oracle/python-cx_Oracle/issues/162>`__).
#)  Added support for specifying an integer for the parameters argument to
    :meth:`Cursor.executemany()`. This allows for batch execution when no
    parameters are required or when parameters have previously been bound. This
    replaces Cursor.executemanyprepared() (which is now deprecated and will be
    removed in cx_Oracle 7).
#)  Adjusted the binding of booleans so that outside of PL/SQL they are bound
    as integers
    (`issue 181 <https://github.com/oracle/python-cx_Oracle/issues/181>`__).
#)  Added support for binding decimal.Decimal values to cx_Oracle.NATIVE_FLOAT
    as requested
    (`issue 184 <https://github.com/oracle/python-cx_Oracle/issues/184>`__).
#)  Added checks on passing invalid type parameters to methods
    :meth:`Cursor.arrayvar()`, :meth:`Cursor.callfunc()` and
    :meth:`Cursor.setinputsizes()`.
#)  Corrected handling of cursors and rowids in DML Returning statements.
#)  Added sample from David Lapp demonstrating the use of GeoPandas with
    SDO_GEOMETRY and a sample for demonstrating the use of REF cursors.
#)  Adjusted samples and documentation for clarity.
#)  Added additional test cases.


Version 6.3.1 (May 2018)
------------------------

#)  Update to `ODPI-C 2.3.2
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-3-2-may-7-2018>`__.

    - Ensure that a call to unregister a subscription only occurs if the
      subscription is still registered.
    - Ensure that before a statement is executed any buffers used for DML
      returning statements are reset.

#)  Ensure that behavior with cx_Oracle.__future__.dml_ret_array_val not
    set or False is the same as the behavior in cx_Oracle 6.2
    (`issue 176 <https://github.com/oracle/python-cx_Oracle/issues/176>`__).


Version 6.3 (April 2018)
------------------------

#)  Update to `ODPI-C 2.3.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-3-1-april-25-2018>`__.

    - Fixed binding of LONG data (values exceeding 32KB) when using the
      function :meth:`Cursor.executemany()`.
    - Added code to verify that a CQN subscription is open before permitting it
      to be used. Error "DPI-1060: subscription was already closed" will now be
      raised if an attempt is made to use a subscription that was closed
      earlier.
    - Stopped attempting to unregister a CQN subscription before it was
      completely registered. This prevents errors encountered during
      registration from being masked by an error stating that the subscription
      has not been registered!
    - Added error "DPI-1061: edition is not supported when a new password is
      specified" to clarify the fact that specifying an edition and a new
      password at the same time is not supported when creating a connection.
      Previously the edition value was simply ignored.
    - Improved error message when older OCI client libraries are being used
      that don't have the method OCIClientVersion().
    - Fixed the handling of ANSI types REAL and DOUBLE PRECISION as
      implemented by Oracle. These types are just subtypes of NUMBER and are
      different from BINARY_FLOAT and BINARY_DOUBLE
      (`issue 163 <https://github.com/oracle/python-cx_Oracle/issues/163>`__).
    - Fixed support for true heterogeneous session pools that use different
      user/password combinations for each session acquired from the pool.
    - Added error message indicating that setting either of the parameters
      arraydmlrowcounts and batcherrors to True in :meth:`Cursor.executemany()`
      is only supported with insert, update, delete and merge statements.

#)  Fixed support for getting the OUT values of bind variables bound to a DML
    Returning statement when calling the function :meth:`Cursor.executemany()`.
    Note that the attribute dml_ret_array_val in :attr:`cx_Oracle.__future__`
    must be set to True first.
#)  Added support for binding integers and floats as cx_Oracle.NATIVE_FLOAT.
#)  A :attr:`cx_Oracle._Error` object is now the value of all cx_Oracle
    exceptions raised by cx_Oracle.
    (`issue 51 <https://github.com/oracle/python-cx_Oracle/issues/51>`__).
#)  Added support for building cx_Oracle with a pre-compiled version of ODPI-C,
    as requested
    (`issue 103 <https://github.com/oracle/python-cx_Oracle/issues/103>`__).
#)  Default values are now provided for all parameters to
    :meth:`cx_Oracle.SessionPool`.
#)  Improved error message when an unsupported Oracle type is encountered.
#)  The Python GIL is now prevented from being held while performing a round
    trip for the call to get the attribute :attr:`Connection.version`
    (`issue 158 <https://github.com/oracle/python-cx_Oracle/issues/158>`__).
#)  Added check for the validity of the year for Python 2.x since it doesn't do
    that itself like Python 3.x does
    (`issue 166 <https://github.com/oracle/python-cx_Oracle/issues/166>`__).
#)  Adjusted documentation to provide additional information on the use of
    :meth:`Cursor.executemany()` as requested
    (`issue 153 <https://github.com/oracle/python-cx_Oracle/issues/153>`__).
#)  Adjusted documentation to state that batch errors and array DML row counts
    can only be used with insert, update, delete and merge statements
    (`issue 31 <https://github.com/oracle/python-cx_Oracle/issues/31>`__).
#)  Updated tutorial to import common connection information from files in
    order to make setup a bit more generic.


Version 6.2.1 (March 2018)
--------------------------

#)  Make sure cxoModule.h is included in the source archive
    (`issue 155 <https://github.com/oracle/python-cx_Oracle/issues/155>`__).


Version 6.2 (March 2018)
------------------------

#)  Update to `ODPI-C 2.2.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-2-1-march-5-2018>`__.

    - eliminate error "DPI-1054: connection cannot be closed when open
      statements or LOBs exist" (`issue 138
      <https://github.com/oracle/python-cx_Oracle/issues/138>`__).
    - avoid a round trip to the database when a connection is released back to
      the pool by preventing a rollback from being called when no transaction
      is in progress.
    - improve error message when the use of bind variables is attempted with
      DLL statements, which is not supported by Oracle.
    - if an Oracle object is retrieved from an attribute of another Oracle
      object or a collection, prevent the "owner" from being destroyed until
      the object that was retrieved has itself been destroyed.
    - correct handling of boundary numbers 1e126 and -1e126
    - eliminate memory leak when calling :meth:`Connection.enq()` and
      :meth:`Connection.deq()`
    - eliminate memory leak when setting NCHAR and NVARCHAR attributes of
      objects.
    - eliminate memory leak when fetching collection objects from the database.

#)  Added support for creating a temporary CLOB, BLOB or NCLOB via the method
    :meth:`Connection.createlob()`.
#)  Added support for binding a LOB value directly to a cursor.
#)  Added support for closing the connection when reaching the end of a
    ``with`` code block controlled by the connection as a context manager, but
    in a backwards compatible way
    (`issue 113 <https://github.com/oracle/python-cx_Oracle/issues/113>`__).
    See :data:`cx_Oracle.__future__` for more information.
#)  Reorganized code to simplify continued maintenance and consolidate
    transformations to/from Python objects.
#)  Ensure that the number of elements in the array is not lost when the
    buffer size is increased to accommodate larger strings.
#)  Corrected support in Python 3.x for cursor.parse() by permitting a string
    to be passed, instead of incorrectly requiring a bytes object.
#)  Eliminate reference leak with LOBs acquired from attributes of objects or
    elements of collections.
#)  Eliminate reference leak when extending an Oracle collection.
#)  Documentation improvements.
#)  Added test cases to the test suite.


Version 6.1 (December 2017)
---------------------------

#)  Update to `ODPI-C 2.1
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-1-december-12-2017>`__.

    - Support was added for accessing sharded databases via sharding keys (new
      in Oracle 12.2). NOTE: the underlying OCI library has a bug when using
      standalone connections. There is a small memory leak proportional to the
      number of connections created/dropped. There is no memory leak when using
      session pools, which is recommended.
    - Added options for authentication with SYSBACKUP, SYSDG, SYSKM and SYSRAC,
      as requested (`issue 101
      <https://github.com/oracle/python-cx_Oracle/issues/101>`__).
    - Attempts to release statements or free LOBs after the connection has been
      closed (by, for example, killing the session) are now prevented.
    - An error message was added when specifying an edition and a connection
      class since this combination is not supported.
    - Attempts to close the session for connections created with an external
      handle are now prevented.
    - Attempting to ping a database earlier than 10g results in ORA-1010:
      invalid OCI operation, but that implies a response from the database and
      therefore a successful ping, so treat it that way!
      (see `<https://github.com/rana/ora/issues/224>`__ for more information).
    - Support was added for converting numeric values in an object type
      attribute to integer and text, as requested (`ODPI-C issue 35
      <https://github.com/oracle/odpi/issues/35>`__).
    - Setting attributes :attr:`DeqOptions.msgId` and
      :attr:`MessageProperties.msgId` now works as expected.
    - The overflow check when using double values (Python floats) as input
      to float attributes of objects or elements of collections was removed as
      it didn't work anyway and is a well-known issue that cannot be prevented
      without removing desired functionality. The developer should ensure that
      the source value falls within the limits of floats, understand the
      consequent precision loss or use a different data type.
    - Variables of string/raw types are restricted to 2 bytes less than 1 GB
      (1,073,741,822 bytes), since OCI cannot handle more than that currently.
    - Support was added for identifying the id of the transaction which spawned
      a CQN subscription message, as requested
      (`ODPI-C issue 32 <https://github.com/oracle/odpi/issues/32>`__).
    - Corrected use of subscription port number (`issue 115
      <https://github.com/oracle/python-cx_Oracle/issues/115>`__).
    - Problems reported with the usage of FormatMessage() on Windows were
      addressed (`ODPI-C issue 47
      <https://github.com/oracle/odpi/issues/47>`__).
    - On Windows, if oci.dll cannot be loaded because it is the wrong
      architecture (32-bit vs 64-bit), attempt to find the offending DLL and
      include the full path of the DLL in the message, as suggested.
      (`ODPI-C issue 49 <https://github.com/oracle/odpi/issues/49>`__).
    - Force OCI prefetch to always use the value 2; the OCI default is 1 but
      setting the ODPI-C default to 2 ensures that single row fetches don't
      require an extra round trip to determine if there are more rows to fetch;
      this change also reduces the potential memory consumption when
      fetchArraySize was set to a large value and also avoids performance
      issues discovered with larger values of prefetch.

#)  Fix build with PyPy 5.9.0-alpha0 in libpython mode
    (`PR 54 <https://github.com/oracle/python-cx_Oracle/pull/54>`__).
#)  Ensure that the edition is passed through to the database when a session
    pool is created.
#)  Corrected handling of Python object references when an invalid keyword
    parameter is passed to :meth:`cx_Oracle.SessionPool`.
#)  Corrected handling of :attr:`Connection.handle` and the handle parameter
    to :meth:`cx_Oracle.connect` on Windows.
#)  Documentation improvements.
#)  Added test cases to the test suite.


Version 6.0.3 (November 2017)
-----------------------------

#)  Update to `ODPI-C 2.0.3
    <https://oracle.github.io/odpi/doc/releasenotes.html#
    version-2-0-3-november-6-2017>`__.

    - Prevent use of uninitialized data in certain cases (`issue 77
      <https://github.com/oracle/python-cx_Oracle/issues/77>`__).
    - Attempting to ping a database earlier than 10g results in error
      "ORA-1010: invalid OCI operation", but that implies a response from the
      database and therefore a successful ping, so treat it that way!
    - Correct handling of conversion of some numbers to NATIVE_FLOAT.
    - Prevent use of NaN with Oracle numbers since it produces corrupt data
      (`issue 91 <https://github.com/oracle/python-cx_Oracle/issues/91>`__).
    - Verify that Oracle objects bound to cursors, fetched from cursors, set in
      object attributes or appended to collection objects are of the correct
      type.
    - Correct handling of NVARCHAR2 when used as attributes of Oracle objects
      or as elements of collections.

#)  Ensure that a call to setinputsizes() with an invalid type prior to a call
    to executemany() does not result in a type error, but instead gracefully
    ignores the call to setinputsizes() as required by the DB API
    (`issue 75 <https://github.com/oracle/python-cx_Oracle/issues/75>`__).
#)  Check variable array size when setting variable values and raise
    IndexError, as is already done for getting variable values.


Version 6.0.2 (August 2017)
---------------------------

#)  Update to `ODPI-C 2.0.2
    <https://oracle.github.io/odpi/doc/releasenotes.html
    #version-2-0-2-august-30-2017>`__.

    - Don't prevent connection from being explicitly closed when a fatal error
      has taken place (`issue 67
      <https://github.com/oracle/python-cx_Oracle/issues/67>`__).
    - Correct handling of objects when dynamic binding is performed.
    - Process deregistration events without an error.
    - Eliminate memory leak when creating objects.

#)  Added missing type check to prevent coercion of decimal to float
    (`issue 68 <https://github.com/oracle/python-cx_Oracle/issues/68>`__).
#)  On Windows, sizeof(long) = 4, not 8, which meant that integers between 10
    and 18 digits were not converted to Python correctly
    (`issue 70 <https://github.com/oracle/python-cx_Oracle/issues/70>`__).
#)  Eliminate memory leak when repeatedly executing the same query.
#)  Eliminate segfault when attempting to reuse a REF cursor that has been
    closed.
#)  Updated documentation.


Version 6.0.1 (August 2017)
---------------------------

#)  Update to `ODPI-C 2.0.1
    <https://oracle.github.io/odpi/doc/releasenotes.html
    #version-2-0-1-august-18-2017>`__.

    - Ensure that queries registered via :meth:`Subscription.registerquery()`
      do not prevent the associated connection from being closed
      (`ODPI-C issue 27 <https://github.com/oracle/odpi/issues/27>`__).
    - Deprecated attribute :attr:`Subscription.id` as it was never intended to
      be exposed (`ODPI-C issue 28
      <https://github.com/oracle/odpi/issues/28>`__). It will be dropped in
      version 6.1.
    - Add support for DML Returning statements that require dynamically
      allocated variable data (such as CLOBs being returned as strings).

#)  Correct packaging of Python 2.7 UCS4 wheels on Linux
    (`issue 64 <https://github.com/oracle/python-cx_Oracle/issues/64>`__).
#)  Updated documentation.


Version 6.0 (August 2017)
-------------------------

#)  Update to `ODPI-C 2.0 <https://oracle.github.io/odpi/doc/releasenotes.html
    #version-2-0-august-14-2017>`__.

    -   Prevent closing the connection when there are any open statements or
        LOBs and add new error "DPI-1054: connection cannot be closed when open
        statements or LOBs exist" when this situation is detected; this is
        needed to prevent crashes under certain conditions when statements or
        LOBs are being acted upon while at the same time (in another thread) a
        connection is being closed; it also prevents leaks of statements and
        LOBs when a connection is returned to a session pool.
    -   On platforms other than Windows, if the regular method for loading the
        Oracle Client libraries fails, try using $ORACLE_HOME/lib/libclntsh.so
        (`ODPI-C issue 20 <https://github.com/oracle/odpi/issues/20>`__).
    -   Use the environment variable DPI_DEBUG_LEVEL at runtime, not compile
        time.
    -   Added support for DPI_DEBUG_LEVEL_ERRORS (reports errors and has the
        value 8) and DPI_DEBUG_LEVEL_SQL (reports prepared SQL statement text
        and has the value 16) in order to further improve the ability to debug
        issues.
    -   Correct processing of :meth:`Cursor.scroll()` in some circumstances.

#)  Delay initialization of the ODPI-C library until the first standalone
    connection or session pool is created so that manipulation of the
    environment variable NLS_LANG can be performed after the module has been
    imported; this also has the added benefit of reducing the number of errors
    that can take place when the module is imported.
#)  Prevent binding of null values from generating the exception "ORA-24816:
    Expanded non LONG bind data supplied after actual LONG or LOB column" in
    certain circumstances
    (`issue 50 <https://github.com/oracle/python-cx_Oracle/issues/50>`__).
#)  Added information on how to run the test suite
    (`issue 33 <https://github.com/oracle/python-cx_Oracle/issues/33>`__).
#)  Documentation improvements.


Version 6.0 rc 2 (July 2017)
----------------------------

#)  Update to `ODPI-C rc 2 <https://oracle.github.io/odpi/doc/releasenotes.html
    #version-2-0-0-rc-2-july-20-2017>`__.

    -   Provide improved error message when OCI environment cannot be created,
        such as when the oraaccess.xml file cannot be processed properly.
    -   On Windows, convert system message to Unicode first, then to UTF-8;
        otherwise, the error message returned could be in a mix of encodings
        (`issue 40 <https://github.com/oracle/python-cx_Oracle/issues/40>`__).
    -   Corrected support for binding decimal values in object attribute values
        and collection element values.
    -   Corrected support for binding PL/SQL boolean values to PL/SQL
        procedures with Oracle client 11.2.

#)  Define exception classes on the connection object in addition to at module
    scope in order to simplify error handling in multi-connection environments,
    as specified in the Python DB API.
#)  Ensure the correct encoding is used for setting variable values.
#)  Corrected handling of CLOB/NCLOB when using different encodings.
#)  Corrected handling of TIMESTAMP WITH TIME ZONE attributes on objects.
#)  Ensure that the array position passed to var.getvalue() does not exceed the
    number of elements allocated in the array.
#)  Reworked test suite and samples so that they are independent of each other
    and so that the SQL scripts used to create/drop schemas are easily adjusted
    to use different schema names, if desired.
#)  Updated DB API test suite stub to support Python 3.
#)  Added additional test cases and samples.
#)  Documentation improvements.


Version 6.0 rc 1 (June 2017)
----------------------------

#)  Update to `ODPI-C rc 1 <https://oracle.github.io/odpi/doc/releasenotes.html
    #version-2-0-0-rc-1-june-16-2017>`__.
#)  The method :meth:`Cursor.setoutputsize` no longer needs to do anything,
    since ODPI-C automatically manages buffer sizes of LONG and LONG RAW
    columns.
#)  Handle case when both precision and scale are zero, as occurs when
    retrieving numeric expressions (`issue 34
    <https://github.com/oracle/python-cx_Oracle/issues/34>`__).
#)  OCI requires that both encoding and nencoding have values or that both
    encoding and encoding do not have values. These parameters are used in
    functions :meth:`cx_Oracle.connect` and :meth:`cx_Oracle.SessionPool`. The
    missing value is set to its default value if one of the values is set and
    the other is not (`issue 36
    <https://github.com/oracle/python-cx_Oracle/issues/36>`__).
#)  Permit use of both string and unicode for Python 2.7 for creating session
    pools and for changing passwords (`issue 23
    <https://github.com/oracle/python-cx_Oracle/issues/23>`__).
#)  Corrected handling of BFILE LOBs.
#)  Add script for dropping test schemas.
#)  Documentation improvements.


Version 6.0 beta 2 (May 2017)
-----------------------------

#)  Added support for getting/setting attributes of objects or element values
    in collections that contain LOBs, BINARY_FLOAT values, BINARY_DOUBLE values
    and NCHAR and NVARCHAR2 values. The error message for any types that are
    not supported has been improved as well.
#)  Enable temporary LOB caching in order to avoid disk I/O as
    `suggested <https://github.com/oracle/odpi/issues/10>`__.
#)  Added support for setting the debug level in ODPI-C, if desirable, by
    setting environment variable DPI_DEBUG_LEVEL prior to building cx_Oracle.
#)  Correct processing of strings in :meth:`Cursor.executemany` when a
    larger string is found after a shorter string in the list of data bound to
    the statement.
#)  Correct handling of long Python integers that cannot fit inside a 64-bit C
    integer (`issue 18
    <https://github.com/oracle/python-cx_Oracle/issues/18>`__).
#)  Correct creation of pool using external authentication.
#)  Handle edge case when an odd number of zeroes trail the decimal point in a
    value that is effectively zero (`issue 22
    <https://github.com/oracle/python-cx_Oracle/issues/22>`__).
#)  Prevent segfault under load when the attempt to create an error fails.
#)  Eliminate resource leak when a standalone connection or pool is freed.
#)  Correct `typo <https://github.com/oracle/python-cx_Oracle/issues/24>`__.
#)  Correct handling of REF cursors when the array size is manipulated.
#)  Prevent attempts from binding the cursor being executed to itself.
#)  Correct reference count handling of parameters when creating a cursor.
#)  Correct determination of the names of the bind variables in prepared SQL
    statements (which behaves a little differently from PL/SQL statements).


Version 6.0 beta 1 (April 2017)
-------------------------------

#)  Simplify building cx_Oracle considerably by use of
    `ODPI-C <https://oracle.github.io/odpi>`__. This means that cx_Oracle can
    now be built without Oracle Client header files or libraries and that at
    runtime cx_Oracle can adapt to Oracle Client 11.2, 12.1 or 12.2 libraries
    without needing to be rebuilt. This also means that wheels can now be
    produced and installed via pip.
#)  Added attribute :attr:`SessionPool.stmtcachesize` to support getting and
    setting the default statement cache size for connections in the pool.
#)  Added attribute :attr:`Connection.dbop` to support setting the database
    operation that is to be monitored.
#)  Added attribute :attr:`Connection.handle` to facilitate testing the
    creation of a connection using a OCI service context handle.
#)  Added parameters tag and matchanytag to the :meth:`cx_Oracle.connect`
    and :meth:`SessionPool.acquire` methods and added parameters tag and retag
    to the :meth:`SessionPool.release` method in order to support session
    tagging.
#)  Added parameter edition to the :meth:`cx_Oracle.SessionPool` method.
#)  Added support for
    `universal rowids <https://github.com/oracle/python-cx_Oracle/blob/master/
    samples/universal_rowids.py>`__.
#)  Added support for `DML Returning of multiple rows
    <https://github.com/oracle/python-cx_Oracle/blob/master/samples/
    dml_returning_multiple_rows.py>`__.
#)  Added attributes :attr:`Variable.actualElements` and
    :attr:`Variable.values` to variables.
#)  Added parameters region, sharding_key and super_sharding_key to the
    :meth:`cx_Oracle.makedsn()` method to support connecting to a sharded
    database (new in Oracle Database 12.2).
#)  Added support for smallint and float data types in Oracle objects, as
    `requested <https://github.com/oracle/python-cx_Oracle/issues/4>`__.
#)  An exception is no longer raised when a collection is empty for methods
    :meth:`Object.first()` and :meth:`Object.last()`. Instead, the value None
    is returned to be consistent with the methods :meth:`Object.next()` and
    :meth:`Object.prev()`.
#)  If the environment variables NLS_LANG and NLS_NCHAR are being used, they
    must be set before the module is imported. Using the encoding and nencoding
    parameters to the :meth:`cx_Oracle.connect` and
    :meth:`cx_Oracle.SessionPool` methods is a simpler alternative to setting
    these environment variables.
#)  Removed restriction on fetching LOBs across round trips to the database
    (eliminates error "LOB variable no longer valid after subsequent fetch").
#)  Removed requirement for specifying a maximum size when fetching LONG or
    LONG raw columns. This also allows CLOB, NCLOB, BLOB and BFILE columns to
    be fetched as strings or bytes without needing to specify a maximum size.
#)  Dropped deprecated parameter twophase from the :meth:`cx_Oracle.connect`
    method. Applications should set the :attr:`Connection.internal_name` and
    :attr:`Connection.external_name` attributes instead to a value appropriate
    to the application.
#)  Dropped deprecated parameters action, module and clientinfo from the
    :meth:`cx_Oracle.connect` method. The appcontext parameter should be used
    instead as shown in this `sample <https://github.com/oracle/
    python-cx_Oracle/blob/master/samples/app_context.py>`__.
#)  Dropped deprecated attribute numbersAsString from
    :ref:`cursor objects <cursorobj>`. Use an output type handler instead as
    shown in this `sample <https://github.com/oracle/python-cx_Oracle/blob/
    master/samples/return_numbers_as_decimals.py>`__.
#)  Dropped deprecated attributes cqqos and rowids from
    :ref:`subscription objects <subscrobj>`. Use the qos attribute instead as
    shown in this `sample <https://github.com/oracle/python-cx_Oracle/blob/
    master/samples/cqn.py>`__.
#)  Dropped deprecated parameters cqqos and rowids from the
    :meth:`Connection.subscribe()` method. Use the qos parameter instead as
    shown in this `sample <https://github.com/oracle/python-cx_Oracle/blob/
    master/samples/cqn.py>`__.


Version 5.3 (March 2017)
------------------------

#)  Added support for Python 3.6.
#)  Dropped support for Python versions earlier than 2.6.
#)  Dropped support for Oracle clients earlier than 11.2.
#)  Added support for
    :meth:`fetching implicit results<Cursor.getimplicitresults()>`
    (available in Oracle 12.1)
#)  Added support for :attr:`Transaction Guard <Connection.ltxid>` (available
    in Oracle 12.1).
#)  Added support for setting the
    :attr:`maximum lifetime <SessionPool.max_lifetime_session>` of pool
    connections (available in Oracle 12.1).
#)  Added support for large row counts (larger than 2 ** 32, available in
    Oracle 12.1)
#)  Added support for :meth:`advanced queuing <Connection.deq()>`.
#)  Added support for :meth:`scrollable cursors <Cursor.scroll()>`.
#)  Added support for :attr:`edition based redefinition <Connection.edition>`.
#)  Added support for :meth:`creating <ObjectType.newobject()>`, modifying and
    binding user defined types and collections.
#)  Added support for creating, modifying and binding PL/SQL records and
    collections (available in Oracle 12.1).
#)  Added support for binding :data:`native integers <cx_Oracle.NATIVE_INT>`.
#)  Enabled statement caching.
#)  Removed deprecated variable attributes maxlength and allocelems.
#)  Corrected support for setting the encoding and nencoding parameters when
    :meth:`creating a connection <cx_Oracle.Connection>` and added support for
    setting these when creating a session pool. These can now be used instead
    of setting the environment variables NLS_LANG and NLS_NCHAR.
#)  Use None instead of 0 for items in the :attr:`Cursor.description` attribute
    that do not have any validity.
#)  Changed driver name to match informal driver name standard used by Oracle
    for other drivers.
#)  Add check for maximum of 10,000 parameters when calling a stored procedure
    or function in order to prevent a possible improper memory access from
    taking place.
#)  Removed -mno-cygwin compile flag since it is no longer used in newer
    versions of the gcc compiler for Cygwin.
#)  Simplified test suite by combining Python 2 and 3 scripts into one script
    and separated out 12.1 features into a single script.
#)  Updated samples to use code that works on both Python 2 and 3
#)  Added support for pickling/unpickling error objects
    (`Issue #23 <https://bitbucket.org/anthony_tuininga/cx_oracle/issues/23>`__)
#)  Dropped support for callbacks on OCI functions.
#)  Removed deprecated types UNICODE, FIXED_UNICODE and LONG_UNICODE (use
    NCHAR, FIXED_NCHAR and LONG_NCHAR instead).
#)  Increased default array size to 100 (from 50) to match other drivers.
#)  Added support for setting the :attr:`~Connection.internal_name` and
    :attr:`~Connection.external_name` on the connection directly. The use of
    the twophase parameter is now deprecated.  Applications should set the
    internal_name and external_name attributes directly to a value appropriate
    to the application.
#)  Added support for using application context when
    :meth:`creating a connection <cx_Oracle.Connection>`. This should be used
    in preference to the module, action and clientinfo parameters which are now
    deprecated.
#)  Reworked database change notification and continuous query notification to
    more closely align with the PL/SQL implementation and prepare for sending
    notifications for AQ messages. The following changes were made:

    - added constant :data:`~cx_Oracle.SUBSCR_QOS_BEST_EFFORT` to replace
      deprecated constant SUBSCR_CQ_QOS_BEST_EFFORT
    - added constant :data:`~cx_Oracle.SUBSCR_QOS_QUERY` to replace
      deprecated constant SUBSCR_CQ_QOS_QUERY
    - added constant :data:`~cx_Oracle.SUBSCR_QOS_DEREG_NFY` to replace
      deprecated constant SUBSCR_QOS_PURGE_ON_NTFN
    - added constant :data:`~cx_Oracle.SUBSCR_QOS_ROWIDS` to replace parameter
      rowids for method :meth:`Connection.subscribe()`
    - deprecated parameter cqqos for method :meth:`Connection.subscribe()`. The
      qos parameter should be used instead.
    - dropped constants SUBSCR_CQ_QOS_CLQRYCACHE, SUBSCR_QOS_HAREG,
      SUBSCR_QOS_MULTICBK, SUBSCR_QOS_PAYLOAD, SUBSCR_QOS_REPLICATE, and
      SUBSCR_QOS_SECURE since they were never actually used
#)  Deprecated use of the numbersAsStrings attribute on cursors. An output type
    handler should be used instead.


Version 5.2.1 (January 2016)
----------------------------

#)  Added support for Python 3.5.
#)  Removed password attribute from connection and session pool objects in
    order to promote best security practices (if stored in RAM in cleartext it
    can be read in process dumps, for example). For those who would like to
    retain this feature, a subclass of Connection could be used to store the
    password.
#)  Added optional parameter externalauth to SessionPool() which enables wallet
    based or other external authentication mechanisms to be used.
#)  Use the national character set encoding when required (when char set form
    is SQLCS_NCHAR); otherwise, the wrong encoding would be used if the
    environment variable NLS_NCHAR is set.
#)  Added support for binding boolean values to PL/SQL blocks and stored
    procedures (available in Oracle 12.1).


Version 5.2 (June 2015)
-----------------------

#)  Added support for strings up to 32k characters (new in Oracle 12c).
#)  Added support for getting array DML row counts (new in Oracle 12c).
#)  Added support for fetching batch errors.
#)  Added support for LOB values larger than 4 GB.
#)  Added support for connections as SYSASM.
#)  Added support for building without any configuration changes to the machine
    when using instant client RPMs on Linux.
#)  Added types NCHAR, FIXED_NCHAR and LONG_NCHAR to replace the types UNICODE,
    FIXED_UNICODE and LONG_UNICODE (which are now deprecated). These types are
    available in Python 3 as well so they can be used to specify the use of
    NCHAR type fields when binding or using setinputsizes().
#)  Fixed binding of booleans in Python 3.x.
#)  Test suite now sets NLS_LANG if not already set.
#)  Enhanced documentation for connection.action attribute and added note
    on cursor.parse() method to make clear that DDL statements are executed
    when parsed.
#)  Removed remaining remnants of support Oracle 9i.
#)  Added __version__ attribute to conform with PEP 396.
#)  Ensure that sessions are released to the pool when calling
    connection.close()
    (`Issue #2 <https://bitbucket.org/anthony_tuininga/cx_oracle/issue/2>`__)
#)  Fixed handling of datetime intervals
    (`Issue #7 <https://bitbucket.org/anthony_tuininga/cx_oracle/issue/7>`__)


Version 5.1.3 (May 2014)
------------------------

#)  Added support for Oracle 12c.
#)  Added support for Python 3.4.
#)  Added support for query result set change notification. Thanks to Glen
    Walker for the patch.
#)  Ensure that in Python 3.x that NCHAR and NVARCHAR2 and NCLOB columns are
    retrieved properly without conversion issues. Thanks to Joakim Andersson
    for pointing out the issue and the possible solution.
#)  Fix bug when an exception is caught and then another exception is raised
    while handling that exception in Python 3.x. Thanks to Boris Dzuba for
    pointing out the issue and providing a test case.
#)  Enhance performance returning integers between 10 and 18 digits on 64-bit
    platforms that support it. Thanks for Shai Berger for the initial patch.
#)  Fixed two memory leaks.
#)  Fix to stop current_schema from throwing a MemoryError on 64-bit platforms
    on occasion. Thanks to Andrew Horton for the fix.
#)  Class name of cursors changed to real name cx_Oracle.Cursor.


Version 5.1.2 (July 2012)
-------------------------

#)  Added support for LONG_UNICODE which is a type used to handle long unicode
    strings. These are not explicitly supported in Oracle but can be used to
    bind to NCLOB, for example, without getting the error "unimplemented or
    unreasonable conversion requested".
#)  Set the row number in a cursor when executing PL/SQL blocks as requested
    by Robert Ritchie.
#)  Added support for setting the module, action and client_info attributes
    during connection so that logon triggers will see the supplied values, as
    requested by Rodney Barnett.


Version 5.1.1 (October 2011)
----------------------------

#)  Simplify management of threads for callbacks performed by database change
    notification and eliminate a crash that occurred under high load in
    certain situations. Thanks to Calvin S. for noting the issue and suggesting
    a solution and testing the patch.
#)  Force server detach on close so that the connection is completely closed
    and not just the session as before.
#)  Force use of OCI_UTF16ID for NCLOBs as using the default character set
    would result in ORA-03127 with Oracle 11.2.0.2 and UTF8 character set.
#)  Avoid attempting to clear temporary LOBs a second time when destroying the
    variable as in certain situations this results in spurious errors.
#)  Added additional parameter service_name to makedsn() which can be used to
    use the service_name rather than the SID in the DSN string that is
    generated.
#)  Fix cursor description in test suite to take into account the number of
    bytes per character.
#)  Added tests for NCLOBS to the test suite.
#)  Removed redundant code in setup.py for calculating the library path.


Version 5.1 (March 2011)
------------------------

#)  Remove support for UNICODE mode and permit Unicode to be passed through in
    everywhere a string may be passed in. This means that strings will be
    passed through to Oracle using the value of the NLS_LANG environment
    variable in Python 3.x as well. Doing this eliminated a bunch of problems
    that were discovered by using UNICODE mode and also removed an unnecessary
    restriction in Python 2.x that Unicode could not be used in connect strings
    or SQL statements, for example.
#)  Added support for creating an empty object variable via a named type, the
    first step to adding full object support.
#)  Added support for Python 3.2.
#)  Account for lib64 used on x86_64 systems. Thanks to Alex Wood for supplying
    the patch.
#)  Clear up potential problems when calling cursor.close() ahead of the
    cursor being freed by going out of scope.
#)  Avoid compilation difficulties on AIX5 as OCIPing does not appear to be
    available on that platform under Oracle 10g Release 2. Thanks to
    Pierre-Yves Fontaniere for the patch.
#)  Free temporary LOBs prior to each fetch in order to avoid leaking them.
    Thanks to Uwe Hoffmann for the initial patch.


Version 5.0.4 (July 2010)
-------------------------

#)  Added support for Python 2.7.
#)  Added support for new parameter (port) for subscription() call which allows
    the client to specify the listening port for callback notifications from
    the database server. Thanks to Geoffrey Weber for the initial patch.
#)  Fixed compilation under Oracle 9i.
#)  Fixed a few error messages.


Version 5.0.3 (February 2010)
-----------------------------

#)  Added support for 64-bit Windows.
#)  Added support for Python 3.1 and dropped support for Python 3.0.
#)  Added support for keyword parameters in cursor.callproc() and
    cursor.callfunc().
#)  Added documentation for the UNICODE and FIXED_UNICODE variable types.
#)  Added extra link arguments required for Mac OS X as suggested by Jason
    Woodward.
#)  Added additional error codes to the list of error codes that raise
    OperationalError rather than DatabaseError.
#)  Fixed calculation of display size for strings with national database
    character sets that are not the default AL16UTF16.
#)  Moved the resetting of the setinputsizes flag before the binding takes
    place so that if an error takes place and a new statement is prepared
    subsequently, spurious errors will not occur.
#)  Fixed compilation with Oracle 10g Release 1.
#)  Tweaked documentation based on feedback from a number of people.
#)  Added support for running the test suite using "python setup.py test"
#)  Added support for setting the CLIENT_IDENTIFIER value in the v$session
    table for connections.
#)  Added exception when attempting to call executemany() with arrays which is
    not supported by the OCI.
#)  Fixed bug when converting from decimal would result in OCI-22062 because
    the locale decimal point was not a period. Thanks to Amaury Forgeot d'Arc
    for the solution to this problem.


Version 5.0.2 (May 2009)
------------------------

#)  Fix creation of temporary NCLOB values and the writing of NCLOB values in
    non Unicode mode.
#)  Re-enabled parsing of non select statements as requested by Roy Terrill.
#)  Implemented a parse error offset as requested by Catherine Devlin.
#)  Removed lib subdirectory when forcing RPATH now that the library directory
    is being calculated exactly in setup.py.
#)  Added an additional cast in order to support compiling by Microsoft
    Visual C++ 2008 as requested by Marco de Paoli.
#)  Added additional include directory to setup.py in order to support
    compiling by Microsoft Visual Studio was requested by Jason Coombs.
#)  Fixed a few documentation issues.


Version 5.0.1 (February 2009)
-----------------------------

#)  Added support for database change notification available in Oracle 10g
    Release 2 and higher.
#)  Fix bug where NCLOB data would be corrupted upon retrieval (non Unicode
    mode) or would generate exception ORA-24806 (LOB form mismatch). Oracle
    insists upon differentiating between CLOB and NCLOB no matter which
    character set is being used for retrieval.
#)  Add new attributes size, bufferSize and numElements to variable objects,
    deprecating allocelems (replaced by numElements) and maxlength (replaced
    by bufferSize)
#)  Avoid increasing memory allocation for strings when using variable width
    character sets and increasing the number of elements in a variable during
    executemany().
#)  Tweaked code in order to ensure that cx_Oracle can compile with Python
    3.0.1.


Version 5.0 (December 2008)
---------------------------

#)  Added support for Python 3.0 with much help from Amaury Forgeot d'Arc.
#)  Removed support for Python 2.3 and Oracle 8i.
#)  Added support for full unicode mode in Python 2.x where all strings are
    passed in and returned as unicode (module must be built in this mode)
    rather than encoded strings
#)  nchar and nvarchar columns now return unicode instead of encoded strings
#)  Added support for an output type handler and/or an input type handler to be
    specified at the connection and cursor levels.
#)  Added support for specifying both input and output converters for variables
#)  Added support for specifying the array size of variables that are created
    using the cursor.var() method
#)  Added support for events mode and database resident connection pooling
    (DRCP) in Oracle 11g.
#)  Added support for changing the password during construction of a new
    connection object as well as after the connection object has been created
#)  Added support for the interval day to second data type in Oracle,
    represented as datetime.timedelta objects in Python.
#)  Added support for getting and setting the current_schema attribute for a
    session
#)  Added support for proxy authentication in session pools as requested by
    Michael Wegrzynek (and thanks for the initial patch as well).
#)  Modified connection.prepare() to return a boolean indicating if a
    transaction was actually prepared in order to avoid the error ORA-24756
    (transaction does not exist).
#)  Raise a cx_Oracle.Error instance rather than a string for column
    truncation errors as requested by Helge Tesdal.
#)  Fixed handling of environment handles in session pools in order to allow
    session pools to fetch objects without exceptions taking place.


Version 4.4.1 (October 2008)
----------------------------

#)  Make the bind variables and fetch variables accessible although they need
    to be treated carefully since they are used internally; support added for
    forward compatibility with version 5.x.
#)  Include the "cannot insert null value" in the list of errors that are
    treated as integrity errors as requested by Matt Boersma.
#)  Use a cx_Oracle.Error instance rather than a string to hold the error when
    truncation (ORA-1406) takes place as requested by Helge Tesdal.
#)  Added support for fixed char, old style varchar and timestamp attribute
    values in objects.
#)  Tweaked setup.py to check for the Oracle version up front rather than
    during the build in order to produce more meaningful errors and simplify
    the code.
#)  In setup.py added proper detection for the instant client on Mac OS X as
    recommended by Martijn Pieters.
#)  In setup.py, avoided resetting the extraLinkArgs on Mac OS X as doing so
    prevents simple modification where desired as expressed by Christian
    Zagrodnick.
#)  Added documentation on exception handling as requested by Andreas Mock, who
    also graciously provided an initial patch.
#)  Modified documentation indicating that the password attribute on connection
    objects can be written.
#)  Added documentation warning that parameters not passed in during subsequent
    executions of a statement will retain their original values as requested by
    Harald Armin Massa.
#)  Added comments indicating that an Oracle client is required since so many
    people find this surprising.
#)  Removed all references to Oracle 8i from the documentation and version 5.x
    will eliminate all vestiges of support for this version of the Oracle
    client.
#)  Added additional link arguments for Cygwin as requested by Rob Gillen.


Version 4.4 (June 2008)
-----------------------

#)  Fix setup.py to handle the Oracle instant client and Oracle XE on both
    Linux and Windows as pointed out by many. Thanks also to the many people
    who also provided patches.
#)  Set the default array size to 50 instead of 1 as the DB API suggests
    because the performance difference is so drastic and many people have
    recommended that the default be changed.
#)  Added Py_BEGIN_ALLOW_THREADS and Py_END_ALLOW_THREADS around each blocking
    call for LOBs as requested by Jason Conroy who also provided an initial
    patch and performed a number of tests that demonstrate the new code is much
    more responsive.
#)  Add support for acquiring cursor.description after a parse.
#)  Defer type assignment when performing executemany() until the last possible
    moment if the value being bound in is null as suggested by Dragos Dociu.
#)  When dropping a connection from the pool, ignore any errors that occur
    during the rollback; unfortunately, Oracle decides to commit data even when
    dropping a connection from the pool instead of rolling it back so the
    attempt still has to be made.
#)  Added support for setting CLIENT_DRIVER in V$SESSION_CONNECT_INFO in Oracle
    11g and higher.
#)  Use cx_Oracle.InterfaceError rather than the builtin RuntimeError when
    unable to create the Oracle environment object as requested by Luke Mewburn
    since the error is specific to Oracle and someone attempting to catch any
    exception cannot simply use cx_Oracle.Error.
#)  Translated some error codes to OperationalError as requested by Matthew
    Harriger; translated if/elseif/else logic to switch statement to make it
    more readable and to allow for additional translation if desired.
#)  Transformed documentation to new format using restructured text. Thanks to
    Waldemar Osuch for contributing the initial draft of the new documentation.
#)  Allow the password to be overwritten by a new value as requested by Alex
    VanderWoude; this value is retained as a convenience to the user and not
    used by anything in the module; if changed externally it may be convenient
    to keep this copy up to date.
#)  Cygwin is on Windows so should be treated in the same way as noted by
    Matthew Cahn.
#)  Add support for using setuptools if so desired as requested by Shreya
    Bhatt.
#)  Specify that the version of Oracle 10 that is now primarily used is 10.2,
    not 10.1.


Version 4.3.3 (October 2007)
----------------------------

#)  Added method ping() on connections which can be used to test whether or not
    a connection is still active (available in Oracle 10g R2).
#)  Added method cx_Oracle.clientversion() which returns a 5-tuple giving the
    version of the client that is in use (available in Oracle 10g R2).
#)  Added methods startup() and shutdown() on connections which can be used to
    startup and shutdown databases (available in Oracle 10g R2).
#)  Added support for Oracle 11g.
#)  Added samples directory which contains a handful of scripts containing
    sample code for more advanced techniques. More will follow in future
    releases.
#)  Prevent error "ORA-24333: zero iteration count" when calling executemany()
    with zero rows as requested by Andreas Mock.
#)  Added methods __enter__() and __exit__() on connections to support using
    connections as context managers in Python 2.5 and higher. The context
    managed is the transaction state. Upon exit the transaction is either
    rolled back or committed depending on whether an exception took place or
    not.
#)  Make the search for the lib32 and lib64 directories automatic for all
    platforms.
#)  Tweak the setup configuration script to include all of the metadata and
    allow for building the module within another setup configuration script
#)  Include the Oracle version in addition to the Python version in the build
    directories that are created and in the names of the binary packages that
    are created.
#)  Remove unnecessary dependency on win32api to build module on Windows.


Version 4.3.2 (August 2007)
---------------------------

#)  Added methods open(), close(), isopen() and getchunksize() in order to
    improve performance of reading/writing LOB values in chunks.
#)  Fixed support for native doubles and floats in Oracle 10g; added new type
    NATIVE_FLOAT to allow specification of a variable of that specific type
    where desired. Thanks to D.R. Boxhoorn for pointing out the fact that this
    was not working properly when the arraysize was anything other than 1.
#)  When calling connection.begin(), only create a new transaction handle if
    one is not already associated with the connection. Thanks to Andreas Mock
    for discovering this and for Amaury Forgeot d'Arc for diagnosing the
    problem and pointing the way to a solution.
#)  Added attribute cursor.rowfactory which allows a method to be called for
    each row that is returned; this is about 20% faster than calling the method
    in Python using the idiom [method(\*r) for r in cursor].
#)  Attempt to locate an Oracle installation by looking at the PATH if the
    environment variable ORACLE_HOME is not set; this is of primary use on
    Windows where this variable should not normally be set.
#)  Added support for autocommit mode as requested by Ian Kelly.
#)  Added support for connection.stmtcachesize which allows for both reading
    and writing the size of the statement cache size. This parameter can make a
    huge difference with the length of time taken to prepare statements. Added
    support for setting the statement tag when preparing a statement. Both of
    these were requested by Bjorn Sandberg who also provided an initial patch.
#)  When copying the value of a variable, copy the return code as well.


Version 4.3.1 (April 2007)
--------------------------

#)  Ensure that if the client buffer size exceeds 4000 bytes that the server
    buffer size does not as strings may only contain 4000 bytes; this allows
    handling of multibyte character sets on the server as well as the client.
#)  Added support for using buffer objects to populate binary data and made the
    Binary() constructor the buffer type as requested by Ken Mason.
#)  Fix potential crash when using full optimization with some compilers.
    Thanks to Aris Motas for noticing this and providing the initial patch and
    to Amaury Forgeot d'Arc for providing an even simpler solution.
#)  Pass the correct charset form in to the write call in order to support
    writing to national character set LOB values properly. Thanks to Ian Kelly
    for noticing this discrepancy.


Version 4.3 (March 2007)
------------------------

#)  Added preliminary support for fetching Oracle objects (SQL types) as
    requested by Kristof Beyls (who kindly provided an initial patch).
    Additional work needs to be done to support binding and updating objects
    but the basic structure is now in place.
#)  Added connection.maxBytesPerCharacter which indicates the maximum number of
    bytes each character can use; use this value to also determine the size of
    local buffers in order to handle discrepancies between the client character
    set and the server character set. Thanks to Andreas Mock for providing the
    initial patch and working with me to resolve this issue.
#)  Added support for querying native floats in Oracle 10g as requested by
    Danny Boxhoorn.
#)  Add support for temporary LOB variables created via PL/SQL instead of only
    directly by cx_Oracle; thanks to Henning von Bargen for discovering this
    problem.
#)  Added support for specifying variable types using the builtin types int,
    float, str and datetime.date which allows for finer control of what type of
    Python object is returned from cursor.callfunc() for example.
#)  Added support for passing booleans to callproc() and callfunc() as
    requested by Anana Aiyer.
#)  Fixed support for 64-bit environments in Python 2.5.
#)  Thanks to Filip Ballegeer and a number of his co-workers, an intermittent
    crash was tracked down; specifically, if a connection is closed, then the
    call to OCIStmtRelease() will free memory twice. Preventing the call when
    the connection is closed solves the problem.


Version 4.2.1 (September 2006)
------------------------------

#)  Added additional type (NCLOB) to handle CLOBs that use the national
    character set as requested by Chris Dunscombe.
#)  Added support for returning cursors from functions as requested by Daniel
    Steinmann.
#)  Added support for getting/setting the "get" mode on session pools as
    requested by Anand Aiyer.
#)  Added support for binding subclassed cursors.
#)  Fixed binding of decimal objects with absolute values less than 0.1.


Version 4.2 (July 2006)
-----------------------

#)  Added support for parsing an Oracle statement as requested by Patrick
    Blackwill.
#)  Added support for BFILEs at the request of Matthew Cahn.
#)  Added support for binding decimal.Decimal objects to cursors.
#)  Added support for reading from NCLOBs as requested by Chris Dunscombe.
#)  Added connection attributes encoding and nencoding which return the IANA
    character set name for the character set and national character set in use
    by the client.
#)  Rework module initialization to use the techniques recommended by the
    Python documentation as one user was experiencing random segfaults due
    to the use of the module dictionary after the initialization was complete.
#)  Removed support for the OPT_Threading attribute. Use the threaded keyword
    when creating connections and session pools instead.
#)  Removed support for the OPT_NumbersAsStrings attribute. Use the
    numbersAsStrings attribute on cursors instead.
#)  Use type long rather than type int in order to support long integers on
    64-bit machines as reported by Uwe Hoffmann.
#)  Add cursor attribute "bindarraysize" which is defaulted to 1 and is used
    to determine the size of the arrays created for bind variables.
#)  Added repr() methods to provide something a little more useful than the
    standard type name and memory address.
#)  Added keyword parameter support to the functions that imply such in the
    documentation as requested by Harald Armin Massa.
#)  Treat an empty dictionary passed through to cursor.execute() as keyword
    parameters the same as if no keyword parameters were specified at all, as
    requested by Fabien Grumelard.
#)  Fixed memory leak when a LOB read would fail.
#)  Set the LDFLAGS value in the environment rather than directly in the
    setup.py file in order to satisfy those who wish to enable the use of
    debugging symbols.
#)  Use __DATE__ and __TIME__ to determine the date and time of the build
    rather than passing it through directly.
#)  Use Oracle types and add casts to reduce warnings as requested by Amaury
    Forgeot d'Arc.
#)  Fixed typo in error message.


Version 4.1.2 (December 2005)
-----------------------------

#)  Restore support of Oracle 9i features when using the Oracle 10g client.


Version 4.1.1 (December 2005)
-----------------------------

#)  Add support for dropping a connection from a session pool.
#)  Add support for write only attributes "module", "action" and "clientinfo"
    which work only in Oracle 10g as requested by Egor Starostin.
#)  Add support for pickling database errors.
#)  Use the previously created bind variable as a template if available when
    creating a new variable of a larger size. Thanks to Ted Skolnick for the
    initial patch.
#)  Fixed tests to work properly in the Python 2.4 environment where dates and
    timestamps are different Python types. Thanks to Henning von Bargen for
    pointing this out.
#)  Added additional directories to search for include files and libraries in
    order to better support the Oracle 10g instant client.
#)  Set the internal fetch number to 0 in order to satisfy very picky source
    analysis tools as requested by Amaury Fogeot d'Arc.
#)  Improve the documentation for building and installing the module from
    source as some people are unaware of the standard methods for building
    Python modules using distutils.
#)  Added note in the documentation indicating that the arraysize attribute
    can drastically affect performance of queries since this seems to be a
    common misunderstanding of first time users of cx_Oracle.
#)  Add a comment indicating that on HP-UX Itanium with Oracle 10g the library
    ttsh10 must also be linked against. Thanks to Bernard Delmee for the
    information.


Version 4.1 (January 2005)
--------------------------

#)  Fixed bug where subclasses of Cursor do not pass the connection in the
    constructor causing a segfault.
#)  DDL statements must be reparsed before execution as noted by Mihai
    Ibanescu.
#)  Add support for setting input sizes by position.
#)  Fixed problem with catching an exception during execute and then still
    attempting to perform a fetch afterwards as noted by Leith Parkin.
#)  Rename the types so that they can be pickled and unpickled. Thanks to Harri
    Pasanen for pointing out the problem.
#)  Handle invalid NLS_LANG setting properly (Oracle seems to like to provide a
    handle back even though it is invalid) and determine the number of bytes
    per character in order to allow for proper support in the future of
    multibyte and variable width character sets.
#)  Remove date checking from the native case since Python already checks that
    dates are valid; enhance error message when invalid dates are encountered
    so that additional processing can be done.
#)  Fix bug executing SQL using numeric parameter names with predefined
    variables (such as what takes place when calling stored procedures with out
    parameters).
#)  Add support for reading CLOB values using multibyte or variable length
    character sets.


Version 4.1 beta 1 (September 2004)
-----------------------------------

#)  Added support for Python 2.4. In Python 2.4, the datetime module is used
    for both binding and fetching of date and timestamp data. In Python 2.3,
    objects from the datetime module can be bound but the internal datetime
    objects will be returned from queries.
#)  Added pickling support for LOB and datetime data.
#)  Fully qualified the table name that was missing in an alter table
    statement in the setup test script as noted by Marc Gehling.
#)  Added a section allowing for the setting of the RPATH linker directive in
    setup.py as requested by Iustin Pop.
#)  Added code to raise a programming error exception when an attempt is made
    to access a LOB locator variable in a subsequent fetch.
#)  The username, password and dsn (tnsentry) are stored on the connection
    object when specified, regardless of whether or not a standard connection
    takes place.
#)  Added additional module level constant called "LOB" as requested by Joseph
    Canedo.
#)  Changed exception type to IntegrityError for constraint violations as
    requested by Joseph Canedo.
#)  If scale and precision are not specified, an attempt is made to return a
    long integer as requested by Joseph Canedo.
#)  Added workaround for Oracle bug which returns an invalid handle when the
    prepare call fails. Thanks to alantam@hsbc.com for providing the code that
    demonstrated the problem.
#)  The cursor method arrayvar() will now accept the actual list so that it is
    not necessary to call cursor.arrayvar() followed immediately by
    var.setvalue().
#)  Fixed bug where attempts to execute the statement "None" with bind
    variables would cause a segmentation fault.
#)  Added support for binding by position (paramstyle = "numeric").
#)  Removed memory leak created by calls to OCIParamGet() which were not
    mirrored by calls to OCIDescriptorFree(). Thanks to Mihai Ibanescu for
    pointing this out and providing a patch.
#)  Added support for calling cursor.executemany() with statement None
    implying that the previously prepared statement ought to be executed.
    Thanks to Mihai Ibanescu for providing a patch.
#)  Added support for rebinding variables when a subsequent call to
    cursor.executemany() uses a different number of rows. Thanks to Mihai
    Ibanescu for supplying a patch.
#)  The microseconds are now displayed in datetime variables when nonzero
    similar to method used in the datetime module.
#)  Added support for binary_float and binary_double columns in Oracle 10g.


Version 4.0.1 (February 2004)
-----------------------------

#)  Fixed bugs on 64-bit platforms that caused segmentation faults and bus
    errors in session pooling and determining the bind variables associated
    with a statement.
#)  Modified test suite so that 64-bit platforms are tested properly.
#)  Added missing commit statements in the test setup scripts. Thanks to Keith
    Lyon for pointing this out.
#)  Fix setup.py for Cygwin environments. Thanks to Doug Henderson for
    providing the necessary fix.
#)  Added support for compiling cx_Oracle without thread support. Thanks to
    Andre Reitz for pointing this out.
#)  Added support for a new keyword parameter called threaded on connections
    and session pools. This parameter defaults to False and indicates whether
    threaded mode ought to be used. It replaces the module level attribute
    OPT_Threading although examining the attribute will be retained until the
    next release at least.
#)  Added support for a new keyword parameter called twophase on connections.
    This parameter defaults to False and indicates whether support for two
    phase (distributed or global) transactions ought to be present. Note that
    support for distributed transactions is buggy when crossing major version
    boundaries (Oracle 8i to Oracle 9i for example).
#)  Ensure that the rowcount attribute is set properly when an exception is
    raised during execution. Thanks to Gary Aviv for pointing out this problem
    and its solution.


Version 4.0 (December 2003)
---------------------------

#)  Added support for subclassing connections, cursors and session pools. The
    changes involved made it necessary to drop support for Python 2.1 and
    earlier although a branch exists in CVS to allow for support of Python 2.1
    and earlier if needed.
#)  Connections and session pools can now be created with keyword parameters,
    not just sequential parameters.
#)  Queries now return integers whenever possible and long integers if the
    number will overflow a simple integer. Floats are only returned when it is
    known that the number is a floating point number or the integer conversion
    fails.
#)  Added initial support for user callbacks on OCI functions. See the
    documentation for more details.
#)  Add support for retrieving the bind variable names associated with a
    cursor with a new method bindnames().
#)  Add support for temporary LOB variables. This means that setinputsizes()
    can be used with the values CLOB and BLOB to create these temporary LOB
    variables and allow for the equivalent of empty_clob() and empty_blob()
    since otherwise Oracle will treat empty strings as NULL values.
#)  Automatically switch to long strings when the data size exceeds the
    maximum string size that Oracle allows (4000 characters) and raise an
    error if an attempt is made to set a string variable to a size that it
    does not support. This avoids truncation errors as reported by Jon Franz.
#)  Add support for global (distributed) transactions and two phase commit.
#)  Force the NLS settings for the session so that test tables are populated
    correctly in all circumstances; problems were noted by Ralf Braun and
    Allan Poulsen.
#)  Display error messages using the environment handle when the error handle
    has not yet been created; this provides better error messages during this
    rather rare situation.
#)  Removed memory leak in callproc() that was reported by Todd Whiteman.
#)  Make consistent the calls to manipulate memory; otherwise segfaults can
    occur when the pymalloc option is used, as reported by Matt Hoskins.
#)  Force a rollback when a session is released back to the session pool.
    Apparently the connections are not as stateless as Oracle's documentation
    suggests and this makes the logic consistent with normal connections.
#)  Removed module method attach(). This can be replaced with a call to
    Connection(handle=) if needed.


Version 3.1 (August 2003)
-------------------------

#)  Added support for connecting with SYSDBA and SYSOPER access which is
    needed for connecting as sys in Oracle 9i.
#)  Only check the dictionary size if the variable is not NULL; otherwise, an
    error takes place which is not caught or cleared; this eliminates a
    spurious "Objects/dictobject.c:1258: bad argument to internal function" in
    Python 2.3.
#)  Add support for session pooling. This is only support for Oracle 9i but
    is amazingly fast -- about 100 times faster than connecting.
#)  Add support for statement caching when pooling sessions, this reduces the
    parse time considerably. Unfortunately, the Oracle OCI does not allow this
    to be easily turned on for normal sessions.
#)  Add method trim() on CLOB and BLOB variables for trimming the size.
#)  Add support for externally identified users; to use this feature leave the
    username and password fields empty when connecting.
#)  Add method cancel() on connection objects to cancel long running queries.
    Note that this only works on non-Windows platforms.
#)  Add method callfunc() on cursor objects to allow calling a function
    without using an anonymous PL/SQL block.
#)  Added documentation on objects that were not documented. At this point all
    objects, methods and constants in cx_Oracle have been documented.
#)  Added support for timestamp columns in Oracle 9i.
#)  Added module level method makedsn() which creates a data source name given
    the host, port and SID.
#)  Added constant "buildtime" which is the time when the module was built as
    an additional means of identifying the build that is in use.
#)  Binding a value that is incompatible to the previous value that was bound
    (data types do not match or array size is larger) will now result in a
    new bind taking place. This is more consistent with the DB API although
    it does imply a performance penalty when used.


Version 3.0a (June 2003)
------------------------

#)  Fixed bug where zero length PL/SQL arrays were being mishandled
#)  Fixed support for the data type "float" in Oracle; added one to the
    display size to allow for the sign of the number, if necessary; changed
    the display size of unconstrained numbers to 127, which is the largest
    number that Oracle can handle
#)  Added support for retrieving the description of a bound cursor before
    fetching it
#)  Fixed a couple of build issues on Mac OS X, AIX and Solaris (64-bit)
#)  Modified documentation slightly based on comments from several people
#)  Included files in MANIFEST that are needed to generate the binaries
#)  Modified test suite to work within the test environment at Computronix
    as well as within the packages that are distributed


Version 3.0 (March 2003)
------------------------

#)  Removed support for connection to Oracle7 databases; it is entirely
    possible that it will still work but I no longer have any way of testing
    and Oracle has dropped any meaningful support for Oracle7 anyway
#)  Fetching of strings is now done with predefined memory areas rather than
    dynamic memory areas; dynamic fetching of strings was causing problems
    with Oracle 9i in some instances and databases using a different character
    set other than US ASCII
#)  Fixed bug where segfault would occur if the '/' character preceded the '@'
    character in a connect string
#)  Added two new cursor methods var() and arrayvar() in order to eliminate
    the need for setinputsizes() when defining PL/SQL arrays and as a generic
    method of acquiring bind variables directly when needed
#)  Fixed support for binding cursors and added support for fetching cursors
    (these are known as ref cursors in PL/SQL).
#)  Eliminated discrepancy between the array size used internally and the
    array size specified by the interface user; this was done earlier to avoid
    bus errors on 64-bit platforms but another way has been found to get
    around that issue and a number of people were getting confused because of
    the discrepancy
#)  Added support for the attribute "connection" on cursors, an optional
    DB API extension
#)  Added support for passing a dictionary as the second parameter for the
    cursor.execute() method in order to comply with the DB API more closely;
    the method of passing parameters with keyword parameters is still supported
    and is in fact preferred
#)  Added support for the attribute "statement" on cursors which is a
    reference to the last SQL statement prepared or executed
#)  Added support for passing any sequence to callproc() rather than just
    lists as before
#)  Fixed bug where segfault would occur if the array size was changed after
    the cursor was executed but before it was fetched
#)  Ignore array size when performing executemany() and use the length of the
    list of parameters instead
#)  Rollback when connection is closed or destroyed to follow DB API rather
    than use the Oracle default (which is commit)
#)  Added check for array size too large causing an integer overflow
#)  Added support for iterators for Python 2.2 and above
#)  Added test suite based on PyUnitTest
#)  Added documentation in HTML format similar to the documentation for the
    core Python library


Version 2.5a (August 2002)
--------------------------

#)  Fix problem with Oracle 9i and retrieving strings; it seems that Oracle 9i
    uses the correct method for dynamic callback but Oracle 8i will not work
    with that method so an #ifdef was added to check for the existence of an
    Oracle 9i feature; thanks to Paul Denize for discovering this problem


Version 2.5 (July 2002)
-----------------------

#)  Added flag OPT_NoOracle7 which, if set, assumes that connections are being
    made to Oracle8 or higher databases; this allows for eliminating the
    overhead in performing this check at connect time
#)  Added flag OPT_NumbersAsStrings which, if set, returns all numbers as
    strings rather than integers or floats; this flag is used when defined
    variables are created (during select statements only)
#)  Added flag OPT_Threading which, if set, uses OCI threading mode; there is a
    significant performance degradation in this mode (about 15-20%) but it does
    allow threads to share connections (threadsafety level 2 according to the
    Python Database API 2.0); note that in order to support this, Oracle 8i or
    higher is now required
#)  Added Py_BEGIN_ALLOW_THREADS and Py_END_ALLOW_THREADS pairs where
    applicable to support threading during blocking OCI calls
#)  Added global method attach() to cx_Oracle to support attaching to an
    existing database handle (as provided by PowerBuilder, for example)
#)  Eliminated the cursor method fetchbinds() which was used for returning the
    list of bind variables after execution to get the values of out variables;
    the cursor method setinputsizes() was modified to return the list of bind
    variables and the cursor method execute() was modified to return the list
    of defined variables in the case of a select statement being executed;
    these variables have three methods available to them: getvalue([<pos>]) to
    get the value of a variable, setvalue(<pos>, <value>) to set its value and
    copy(<var>, <src_pos>, <targ_pos>) to copy the value from a variable in a
    more efficient manner than setvalue(getvalue())
#)  Implemented cursor method executemany() which expects a list of
    dictionaries for the parameters
#)  Implemented cursor method callproc()
#)  Added cursor method prepare() which parses (prepares) the statement for
    execution; subsequent execute() or executemany() calls can pass None as the
    statement which will imply use of the previously prepared statement; used
    for high performance only
#)  Added cursor method fetchraw() which will perform a raw fetch of the cursor
    returning the number of rows thus fetched; this is used to avoid the
    overhead of generating result sets; used for high performance only
#)  Added cursor method executemanyprepared() which is identical to the method
    executemany() except that it takes a single parameter which is the number
    of times to execute a previously prepared statement and it assumes that the
    bind variables already have their values set; used for high performance
    only
#)  Added support for rowid being returned in a select statement
#)  Added support for comparing dates returned by cx_Oracle
#)  Integrated patch from Andre Reitz to set the null ok flag in the
    description attribute of the cursor
#)  Integrated patch from Andre Reitz to setup.py to support compilation with
    Python 1.5
#)  Integrated patch from Benjamin Kearns to setup.py to support compilation
    on Cygwin


Version 2.4 (January 2002)
--------------------------

#)  String variables can now be made any length (previously restricted to the
    64K limit imposed by Oracle for default binding); use the type
    cx_Oracle.LONG_STRING as the parameter to setinputsizes() for binding in
    string values larger than 4000 bytes.
#)  Raw and long raw columns are now supported; use the types cx_Oracle.BINARY
    and cx_Oracle.LONG_BINARY as the parameter to setinputsizes() for binding
    in values of these types.
#)  Functions DateFromTicks(), TimeFromTicks() and TimestampFromTicks()
    are now implemented.
#)  Function cursor.setoutputsize() implemented
#)  Added the ability to bind arrays as out parameters to procedures; use the
    format [cx_Oracle.<DataType>, <NumElems>] as the input to the function
    setinputsizes() for binding arrays
#)  Discovered from the Oracle 8.1.6 version of the documentation of the OCI
    libraries, that the size of the memory location required for the precision
    variable is larger than the printed documentation says; this was causing a
    problem with the code on the Sun platform.
#)  Now support building RPMs for Linux.


Version 2.3 (October 2001)
--------------------------

#)  Incremental performance enhancements (dealing with reusing cursors and
    bind handles)
#)  Ensured that arrays of integers with a single float in them are all
    treated as floats, as suggested by Martin Koch.
#)  Fixed code dealing with scale and precision for both defining a numeric
    variable and for providing the cursor description; this eliminates the
    problem of an underflow error (OCI-22054) when retrieving data with
    non-zero scale.


Version 2.2 (July 2001)
-----------------------

#)  Upgraded thread safety to level 1 (according to the Python DB API 2.0) as
    an internal project required the ability to share the module between
    threads.
#)  Added ability to bind ref cursors to PL/SQL blocks as requested by
    Brad Powell.
#)  Added function write(Value, [Offset]) to LOB variables as requested by
    Matthias Kirst.
#)  Procedure execute() on Cursor objects now permits a value None for the
    statement which means that the previously prepared statement will be
    executed and any input sizes set earlier will be retained. This was done to
    improve the performance of scripts that execute one statement many times.
#)  Modified module global constants BINARY and DATETIME to point to the
    external representations of those types so that the expression
    type(var) == cx_Oracle.DATETIME will work as expected.
#)  Added global constant version to provide means of determining the current
    version of the module.
#)  Modified error checking routine to distinguish between an Oracle error and
    invalid handles.
#)  Added error checking to avoid setting the value of a bind variable to a
    value that it cannot support and raised an exception to indicate this fact.
#)  Added extra compile arguments for the AIX platform as suggested by Jehwan
    Ryu.
#)  Added section to the README to indicate the method for a binary
    installation as suggested by Steve Holden.
#)  Added simple usage example as requested by many people.
#)  Added HISTORY file to the distribution.
