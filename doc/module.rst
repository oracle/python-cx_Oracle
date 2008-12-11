.. module:: cx_Oracle

.. _module:

****************
Module Interface
****************

.. function:: Binary(string)

   Construct an object holding a binary (long) string value.


.. function:: clientversion()

   Return the version of the client library being used as a 5-tuple. The five
   values are the major version, minor version, update number, patch number and
   port update number.

   .. note::

      This method is an extension to the DB API definition and is only
      available in Oracle 10g Release 2 and higher.


.. function:: Connection([user, password, dsn, mode, handle, pool, threaded, twophase, events, cclass, purity, newpassword])
              connect([user, password, dsn, mode, handle, pool, threaded, twophase, events, cclass, purity, newpassword])

   Constructor for creating a connection to the database. Return a Connection
   object (:ref:`connobj`). All arguments are optional and can be specified as
   keyword parameters.
  
   The dsn (data source name) is the TNS entry (from the Oracle names server or
   tnsnames.ora file) or is a string like the one returned from makedsn(). If
   only one parameter is passed, a connect string is assumed which is to be of
   the format ``user/password@dsn``, the same format accepted by Oracle
   applications such as SQL\*Plus.
  
   If the mode is specified, it must be one of :data:`SYSDBA` or
   :data:`SYSOPER` which are defined at the module level; otherwise it defaults
   to the normal mode of connecting.
  
   If the handle is specified, it must be of type OCISvcCtx\* and is only of
   use when embedding Python in an application (like PowerBuilder) which has
   already made the connection.
  
   The pool argument is expected to be a session pool object (:ref:`sesspool`)
   and the use of this argument is the equivalent of calling pool.acquire().
  
   The threaded argument is expected to be a boolean expression which
   indicates whether or not Oracle should use the mode OCI_THREADED to wrap
   accesses to connections with a mutex. Doing so in single threaded
   applications imposes a performance penalty of about 10-15% which is why the
   default is False.
  
   The twophase argument is expected to be a boolean expression which
   indicates whether or not the attributes should be set on the connection
   object to allow for two phase commit. The default for this value is also
   False because of bugs in Oracle prior to Oracle 10g.

   The events argument is expected to be a boolean expression which indicates
   whether or not to initialize Oracle in events mode (only available in Oracle
   11g and higher).

   The cclass argument is expected to be a string and defines the connection
   class for database resident connection pooling (DRCP) in Oracle 11g and
   higher.

   The purity argument is expected to be one of :data:`ATTR_PURITY_NEW` (the
   session must be new without any prior session state),
   :data:`ATTR_PURITY_NEW` (the session may have been used before) or
   :data:`ATTR_PURITY_DEFAULT` (the default behavior which is defined by Oracle
   in its documentation). This argument is only relevant in Oracle 11g and
   higher.

   The newpassword argument is expected to be a string if specified and sets
   the password for the logon during the connection process.


.. function:: Cursor(connection)

   Constructor for creating a cursor.  Return a new Cursor object
   (:ref:`cursorobj`) using the connection.

   .. note::

      This method is an extension to the DB API definition.


.. function:: Date(year, month, day)

   Construct an object holding a date value.


.. function:: DateFromTicks(ticks)

   Construct an object holding a date value from the given ticks value (number
   of seconds since the epoch; see the documentation of the standard Python
   time module for details).


.. function:: makedsn(host, port, sid)

   Return a string suitable for use as the dsn for the connect() method. This
   string is identical to the strings that are defined by the Oracle names
   server or defined in the tnsnames.ora file.

   .. note::

      This method is an extension to the DB API definition.


.. function:: SessionPool(user, password, database, min, max, increment, [connectiontype, threaded, getmode=cx_Oracle.SPOOL_ATTRVAL_NOWAIT, homogeneous=True])

   Create a session pool (see Oracle 9i documentation for more information) and
   return a session pool object (:ref:`sesspool`). This allows for very fast
   connections to the database and is of primary use in a server where the same
   connection is being made multiple times in rapid succession (a web server,
   for example). If the connection type is specified, all calls to acquire()
   will create connection objects of that type, rather than the base type
   defined at the module level. The threaded attribute is expected to be a
   boolean expression which indicates whether or not Oracle should use the mode
   OCI_THREADED to wrap accesses to connections with a mutex. Doing so in
   single threaded applications imposes a performance penalty of about 10-15%
   which is why the default is False.

   .. note::

      This method is an extension to the DB API definition and is only
      available in Oracle 9i.


.. function:: Time(hour, minute, second)

   Construct an object holding a time value.


.. function:: TimeFromTicks(ticks)

   Construct an object holding a time value from the given ticks value (number
   of seconds since the epoch; see the documentation of the standard Python
   time module for details).


.. function:: Timestamp(year, month, day, hour, minute, second)

   Construct an object holding a time stamp value.


.. function:: TimestampFromTicks(ticks)

   Construct an object holding a time stamp value from the given ticks value
   (number of seconds since the epoch; see the documentation of the standard
   Python time module for details).



.. _constants:

Constants
=========


.. data:: apilevel

   String constant stating the supported DB API level. Currently '2.0'.


.. data:: ATTR_PURITY_DEFAULT

   This constant is used when using database resident connection pooling (DRCP)
   and specifies that the purity of the session is the default value used by
   Oracle (see Oracle's documentation for more information).

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: ATTR_PURITY_NEW

   This constant is used when using database resident connection pooling (DRCP)
   and specifies that the session acquired from the pool should be new and not
   have any prior session state.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: ATTR_PURITY_SELF

   This constant is used when using database resident connection pooling (DRCP)
   and specifies that the session acquired from the pool need not be new and
   may have prior session state.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: buildtime

   String constant stating the time when the binary was built.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: BINARY

   This type object is used to describe columns in a database that are binary
   (in Oracle this is RAW columns).


.. data:: BFILE

   This type object is used to describe columns in a database that are BFILEs.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: BLOB

   This type object is used to describe columns in a database that are BLOBs.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: CLOB

   This type object is used to describe columns in a database that are CLOBs.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: CURSOR

   This type object is used to describe columns in a database that are cursors
   (in PL/SQL these are known as ref cursors).

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: DATETIME

   This type object is used to describe columns in a database that are dates.


.. data:: DBSHUTDOWN_ABORT

   This constant is used in database shutdown to indicate that the program
   should not wait for current calls to complete or for users to disconnect
   from the database. Use only in unusual circumstances since database recovery
   may be necessary upon next startup.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: DBSHUTDOWN_FINAL

   This constant is used in database shutdown to indicate that the instance can
   be truly halted. This should only be done after the database has been shut
   down in one of the other modes (except abort) and the database has been
   closed and dismounted using the appropriate SQL commands. See the method
   :meth:`~Connection.shutdown()` in the section on connections
   (:ref:`connobj`).

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: DBSHUTDOWN_IMMEDIATE

   This constant is used in database shutdown to indicate that all uncommitted
   transactions should be rolled back and any connected users should be
   disconnected.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: DBSHUTDOWN_TRANSACTIONAL

   This constant is used in database shutdown to indicate that further
   connections should be prohibited and no new transactions should be allowed.
   It then waits for active transactions to complete.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: DBSHUTDOWN_TRANSACTIONAL_LOCAL

   This constant is used in database shutdown to indicate that further
   connections should be prohibited and no new transactions should be allowed.
   It then waits for only local active transactions to complete.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FIXED_CHAR

   This type object is used to describe columns in a database that are fixed
   length strings (in Oracle this is CHAR columns); these behave differently in
   Oracle than varchar2 so they are differentiated here even though the DB API
   does not differentiate them.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FNCODE_BINDBYNAME

   This constant is used to register callbacks on the OCIBindByName() function
   of the OCI.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FNCODE_BINDBYPOS

   This constant is used to register callbacks on the OCIBindByPos() function
   of the OCI.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FNCODE_DEFINEBYPOS

   This constant is used to register callbacks on the OCIDefineByPos() function
   of the OCI.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FNCODE_STMTEXECUTE

   This constant is used to register callbacks on the OCIStmtExecute() function
   of the OCI.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FNCODE_STMTFETCH

   This constant is used to register callbacks on the OCIStmtFetch() function
   of the OCI.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: FNCODE_STMTPREPARE

   This constant is used to register callbacks on the OCIStmtPrepare() function
   of the OCI.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: INTERVAL

   This type object is used to describe columns in a database that are of type
   interval day to second.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: LOB

   This type object is the Python type of :data:`BLOB` and :data:`CLOB` data
   that is returned from cursors.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: LONG_BINARY

   This type object is used to describe columns in a database that are long
   binary (in Oracle these are LONG RAW columns).

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: LONG_STRING

   This type object is used to describe columns in a database that are long
   strings (in Oracle these are LONG columns).

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: NATIVE_FLOAT

   This type object is used to describe columns in a database that are of type
   binary_double or binary_float and is only available in Oracle 10g.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: NCLOB

   This type object is used to describe columns in a database that are NCLOBs.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: NUMBER

   This type object is used to describe columns in a database that are numbers.


.. data:: OBJECT

   This type object is used to describe columns in a database that are objects.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: paramstyle

   String constant stating the type of parameter marker formatting expected by
   the interface. Currently 'named' as in 'where name = :name'.


.. data:: ROWID

   This type object is used to describe the pseudo column "rowid".


.. data:: PRELIM_AUTH

   This constant is used to define the preliminary authentication mode required
   for performing database startup and shutdown.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: SPOOL_ATTRVAL_FORCEGET

   This constant is used to define the "get" mode on session pools and
   indicates that a new connection will be returned if there are no free
   sessions available in the pool.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: SPOOL_ATTRVAL_NOWAIT

   This constant is used to define the "get" mode on session pools and
   indicates that an exception is raised if there are no free sessions
   available in the pool.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: SPOOL_ATTRVAL_WAIT

   This constant is used to define the "get" mode on session pools and
   indicates that the acquisition of a connection waits until a session is
   freed if there are no free sessions available in the pool.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: STRING

   This type object is used to describe columns in a database that are strings
   (in Oracle this is VARCHAR2 columns).


.. data:: SYSDBA

   Value to be passed to the connect() method which indicates that SYSDBA
   access is to be acquired. See the Oracle documentation for more details.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: SYSOPER

   Value to be passed to the connect() method which indicates that SYSOPER
   access is to be acquired. See the Oracle documentation for more details.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: threadsafety

   Integer constant stating the level of thread safety that the interface
   supports.  Currently 2, which means that threads may share the module and
   connections, but not cursors. Sharing means that a thread may use a
   resource without wrapping it using a mutex semaphore to implement resource
   locking.

   Note that in order to make use of multiple threads in a program which
   intends to connect and disconnect in different threads, the threaded
   argument to the Connection constructor must be a true value. See the
   comments on the Connection constructor for more information (:ref:`module`).


.. data:: TIMESTAMP

   This type object is used to describe columns in a database that are
   timestamps.

   .. note::

      This attribute is an extension to the DB API definition and is only
      available in Oracle 9i.


.. data:: UCBTYPE_ENTRY

   This constant is used to register callbacks on entry to the function of the
   OCI.  In other words, the callback will be called prior to the execution of
   the OCI function.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: UCBTYPE_EXIT

   This constant is used to register callbacks on exit from the function of the
   OCI. In other words, the callback will be called after the execution of the
   OCI function.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: UCBTYPE_REPLACE

   This constant is used to register callbacks that completely replace the call
   to the OCI function.

   .. note::

      This attribute is an extension to the DB API definition.


.. data:: version

   String constant stating the version of the module. Currently '|release|'.

   .. note::

      This attribute is an extension to the DB API definition.



.. _exceptions:

Exceptions
==========

.. exception:: Warning

   Exception raised for important warnings and defined by the DB API but not
   actually used by cx_Oracle.


.. exception:: Error

   Exception that is the base class of all other exceptions defined by
   cx_Oracle and is a subclass of the Python StandardError exception (defined
   in the module exceptions).


.. exception:: InterfaceError

   Exception raised for errors that are related to the database interface
   rather than the database itself. It is a subclass of Error.


.. exception:: DatabaseError

   Exception raised for errors that are related to the database. It is a
   subclass of Error.


.. exception:: DataError

   Exception raised for errors that are due to problems with the processed
   data. It is a subclass of DatabaseError.


.. exception:: OperationalError

   Exception raised for errors that are related to the operation of the
   database but are not necessarily under the control of the progammer. It is a
   subclass of DatabaseError.


.. exception:: IntegrityError

   Exception raised when the relational integrity of the database is affected.
   It is a subclass of DatabaseError.


.. exception:: InternalError

   Exception raised when the database encounters an internal error. It is a
   subclass of DatabaseError.


.. exception:: ProgrammingError

   Exception raised for programming errors. It is a subclass of DatabaseError.


.. exception:: NotSupportedError

   Exception raised when a method or database API was used which is not
   supported by the database. It is a subclass of DatabaseError.


Exception handling
==================

.. note::

   PEP 249 (Python Database API Specification v2.0) says the following about
   exception values:

       [...] The values of these exceptions are not defined. They should
       give the user a fairly good idea of what went wrong, though. [...]

   With cx_Oracle every exception object has exactly one argument in the
   ``args`` tuple. This argument is a ``cx_Oracle._Error`` object which has
   the following three read-only attributes.

.. attribute:: _Error.code

   Integer attribute representing the Oracle error number (ORA-XXXXX).

.. attribute:: _Error.message

   String attribute representing the Oracle message of the error. This
   message is localized by the environment of the Oracle connection.

.. attribute:: _Error.context

   String attribute representing the context in which the exception was
   raised..

This allows you to use the exceptions for example in the following way:

::

    import sys
    import cx_Oracle

    connection = cx_Oracle.Connection("user/pw@tns")
    cursor = connection.cursor()

    try:
        cursor.execute("select 1 / 0 from dual")
    except cx_Oracle.DatabaseError, exc:
        error, = exc.args
        print >> sys.stderr, "Oracle-Error-Code:", error.code
        print >> sys.stderr, "Oracle-Error-Message:", error.message

