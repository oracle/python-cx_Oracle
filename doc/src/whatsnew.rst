.. _whatsnew:

**********
What's New
**********

.. _whatsnew60:

cx_Oracle 6.0
=============

This document contains a summary of the changes in cx_Oracle 6 compared to
cx_Oracle 5.3. cx_Oracle 6.0 was released on August 14, 2017. See the
:ref:`release notes <releasenotes60>` for complete details.

Highlights
----------

    - Has been re-implemented to use the new
      `ODPI-C <https://oracle.github.io/odpi>`__ abstraction layer for Oracle
      Database. The cx_Oracle API is unchanged. The cx_Oracle design, build and
      linking process has improved because of ODPI-C.

    - Now has Python Wheels available for install. This is made possible by the
      ODPI-C architecture. Windows installers and Linux RPMs are no longer
      produced since PyPI no longer supports them.

    - Has less code in Python's Global Interpreter Lock, giving better
      scalability.

    - Added support for universal rowids. 

    - Added support for DML returning of multiple rows.

    - Now associates LOB locators to LOB objects so they are not overwritten on
      database round trips.


Installation Changes
--------------------

    - On Linux, cx_Oracle 6 no longer uses instant client RPMs automatically.
      You must set LD_LIBRARY_PATH or use ldconfig to locate the Oracle Client
      library.

    - On platforms other than Windows, if ORACLE_HOME is set (in a database or
      full client installation), remove requirement to set LD_LIBRARY_PATH in
      order to locate the Oracle Client library
      (`issue 20 <https://github.com/oracle/odpi/issues/20>`__).


Connection Management Enhancements
----------------------------------

    - Prevent closing the connection when there are any open statements or LOBs
      and add new error "DPI-1054: connection cannot be closed when open
      statements or LOBs exist" when this situation is detected; this is needed
      to prevent crashes under certain conditions when statements or LOBs are
      being acted upon while at the same time (in another thread) a connection
      is being closed; it also prevents leaks of statements and LOBs when a
      connection is returned to a session pool. 

    - Added attribute :attr:`SessionPool.stmtcachesize` to support getting and
      setting the default statement cache size for connections in the pool. 

    - Added attribute :attr:`Connection.dbop` to support setting the database
      operation that is to be monitored. 

    - Added attribute :attr:`Connection.handle` to facilitate testing the
      creation of a connection using a OCI service context handle. 

    - Added parameters tag and matchanytag to the :meth:`cx_Oracle.connect` and
      :meth:`SessionPool.acquire` methods and added parameters tag and retag to
      the :meth:`SessionPool.release` method in order to support session
      tagging. 

    - Added parameter edition to the :meth:`cx_Oracle.SessionPool` method. 

    - Added parameters region, sharding_key and super_sharding_key to the
      :meth:`cx_Oracle.makedsn()` method to support connecting to a sharded
      database (new in Oracle Database 12.2). 

    - Removed requirement that encoding and nencoding both be specified when
      creating a connection or session pool. The missing value is set to its
      default value if one of the values is set and the other is not
      (`issue 36 <https://github.com/oracle/python-cx_Oracle/issues/36>`__). 

    - Permit use of both string and unicode for Python 2.7 for creating session
      pools and for changing passwords
      (`issue 23 <https://github.com/oracle/python-cx_Oracle/issues/23>`__). 


Data Type and Data Handling Enhancements
----------------------------------------

    - Added attributes :attr:`Variable.actualElements` and
      :attr:`Variable.values` to variables. 

    - Added support for smallint and float data types in Oracle objects, as
      requested
      (`issue 4 <https://github.com/oracle/python-cx_Oracle/issues/4>`__).

    - Added support for getting/setting attributes of objects or element values
      in collections that contain LOBs, BINARY_FLOAT values, BINARY_DOUBLE
      values and NCHAR and NVARCHAR2 values. The error message for any types
      that are not supported has been improved as well.

    - An exception is no longer raised when a collection is empty for methods
      :meth:`Object.first()` and :meth:`Object.last()`. Instead, the value None
      is returned to be consistent with the methods :meth:`Object.next()` and
      :meth:`Object.prev()`.

    - Removed requirement for specifying a maximum size when fetching LONG or
      LONG raw columns. This also allows CLOB, NCLOB, BLOB and BFILE columns to
      be fetched as strings or bytes without needing to specify a maximum size.
      The method :meth:`Cursor.setoutputsize` no longer does anything, since
      ODPI-C automatically manages buffer sizes of LONG and LONG RAW columns. 

    - Enable temporary LOB caching in order to avoid disk I/O as suggested
      (`issue 10 <https://github.com/oracle/odpi/issues/10>`__). 


Error Handling Enhancements
---------------------------

    - Provide improved error message when OCI environment cannot be created,
      such as when the oraaccess.xml file cannot be processed properly. 

    - Define exception classes on the connection object in addition to at
      module scope in order to simplify error handling in multi-connection
      environments, as specified in the Python DB API. 


Test Enhancements
-----------------

    - Reworked test suite and samples so that they are independent of each
      other and so that the SQL scripts used to create/drop schemas are easily
      adjusted to use different schema names, if desired. 

    - Updated DB API test suite stub to support Python 3. 


Removals
--------

    - Dropped deprecated parameter twophase from the :meth:`cx_Oracle.connect`
      method. Applications should set the :attr:`Connection.internal_name` and
      :attr:`Connection.external_name` attributes instead to a value
      appropriate to the application. 

    - Dropped deprecated parameters action, module and clientinfo from the
      :meth:`cx_Oracle.connect` method. The appcontext parameter should be used
      instead as shown in this `sample <https://github.com/oracle/
      python-cx_Oracle/blob/master/samples/AppContext.py>`__.

    - Dropped deprecated attribute numbersAsString from
      :ref:`cursor objects <cursorobj>`. Use an output type handler instead as
      shown in this `sample <https://github.com/oracle/python-cx_Oracle/blob/
      master/samples/ReturnNumbersAsDecimals.py>`__.

    - Dropped deprecated attributes cqqos and rowids from
      :ref:`subscription objects <subscrobj>`. Use the qos attribute instead as
      shown in this `sample <https://github.com/oracle/python-cx_Oracle/blob/
      master/samples/CQN.py>`__.

    - Dropped deprecated parameters cqqos and rowids from the
      :meth:`Connection.subscribe()` method. Use the qos parameter instead as
      shown in this `sample <https://github.com/oracle/python-cx_Oracle/blob/
      master/samples/CQN.py>`__.

