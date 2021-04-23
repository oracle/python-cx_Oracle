.. _tracingsql:

*********************************
Tracing SQL and PL/SQL Statements
*********************************

Subclass Connections
====================

Subclassing enables applications to add "hooks" for connection and statement
execution.  This can be used to alter, or log, connection and execution
parameters, and to extend cx_Oracle functionality.

The example below demonstrates subclassing a connection to log SQL execution
to a file.  This example also shows how connection credentials can be embedded
in the custom subclass, so application code does not need to supply them.

.. code-block:: python

    class Connection(cx_Oracle.Connection):
        logFileName = "log.txt"

        def __init__(self):
            connectString = "hr/hr_password@dbhost.example.com/orclpdb1"
            self._log("Connect to the database")
            return super(Connection, self).__init__(connectString)

        def _log(self, message):
            with open(self.logFileName, "a") as f:
                print(message, file=f)

        def execute(self, sql, parameters):
            self._log(sql)
            cursor = self.cursor()
            try:
                return cursor.execute(sql, parameters)
            except cx_Oracle.Error as e:
                errorObj, = e.args
                self._log(errorObj.message)

    connection = Connection()
    connection.execute("""
            select department_name
            from departments
            where department_id = :id""", dict(id=270))

The messages logged in ``log.txt`` are::

    Connect to the database

                select department_name
                from departments
                where department_id = :id

If an error occurs, perhaps due to a missing table, the log file would contain
instead::

    Connect to the database

                select department_name
                from departments
                where department_id = :id
    ORA-00942: table or view does not exist

In production applications be careful not to log sensitive information.

See `Subclassing.py
<https://github.com/oracle/python-cx_Oracle/blob/master/
samples/subclassing.py>`__ for an example.


.. _endtoendtracing:

Oracle Database End-to-End Tracing
==================================

Oracle Database End-to-end application tracing simplifies diagnosing application
code flow and performance problems in multi-tier or multi-user environments.

The connection attributes, :attr:`~Connection.client_identifier`,
:attr:`~Connection.clientinfo`, :attr:`~Connection.dbop`,
:attr:`~Connection.module` and :attr:`~Connection.action`, set the metadata for
end-to-end tracing.  You can use data dictionary and ``V$`` views to monitor
tracing or use other application tracing utilities.

The attributes are sent to the database when the next :ref:`round-trip
<roundtrips>` to the database occurs, for example when the next SQL statement is
executed.

The attribute values will remain set in connections released back to connection
pools.  When the application re-acquires a connection from the pool it should
initialize the values to a desired state before using that connection.

The example below shows setting the action, module and client identifier
attributes on the connection object:

.. code-block:: python

    # Set the tracing metadata
    connection.client_identifier = "pythonuser"
    connection.action = "Query Session tracing parameters"
    connection.module = "End-to-end Demo"

    for row in cursor.execute("""
            SELECT username, client_identifier, module, action
            FROM V$SESSION
            WHERE username = 'SYSTEM'"""):
        print(row)

The output will be::

    ('SYSTEM', 'pythonuser', 'End-to-end Demo', 'Query Session tracing parameters')

The values can also be manually set as shown by calling
`DBMS_APPLICATION_INFO procedures
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-14484F86-44F2-4B34-B34E-0C873D323EAD>`__
or `DBMS_SESSION.SET_IDENTIFIER
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-988EA930-BDFE-4205-A806-E54F05333562>`__. These incur round-trips to
the database, however, reducing scalability.

.. code-block:: sql

    BEGIN
        DBMS_SESSION.SET_IDENTIFIER('pythonuser');
        DBMS_APPLICATION_INFO.set_module('End-to-End Demo');
        DBMS_APPLICATION_INFO.set_action(action_name => 'Query Session tracing parameters');
    END;


Low Level SQL Tracing in cx_Oracle
==================================

cx_Oracle is implemented using the `ODPI-C <https://oracle.github.io/odpi>`__
wrapper on top of the Oracle Client libraries.  The ODPI-C tracing capability
can be used to log executed cx_Oracle statements to the standard error stream.
Before executing Python, set the environment variable ``DPI_DEBUG_LEVEL`` to
16.

At a Windows command prompt, this could be done with::

    set DPI_DEBUG_LEVEL=16

On Linux, you might use::

    export DPI_DEBUG_LEVEL=16

After setting the variable, run the Python Script, for example on Linux::

    python end-to-endtracing.py 2> log.txt

For an application that does a single query, the log file might contain a
tracing line consisting of the prefix 'ODPI', a thread identifier, a timestamp,
and the SQL statement executed::

    ODPI [26188] 2019-03-26 09:09:03.909: ODPI-C 3.1.1
    ODPI [26188] 2019-03-26 09:09:03.909: debugging messages initialized at level 16
    ODPI [26188] 2019-03-26 09:09:09.917: SQL SELECT * FROM jobss
    Traceback (most recent call last):
    File "end-to-endtracing.py", line 14, in <module>
      cursor.execute("select * from jobss")
    cx_Oracle.DatabaseError: ORA-00942: table or view does not exist

See `ODPI-C Debugging
<https://oracle.github.io/odpi/doc/user_guide/debugging.html>`__ for
documentation on ``DPI_DEBUG_LEVEL``.
