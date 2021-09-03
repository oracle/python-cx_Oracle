.. _introduction:

*************************
Introduction to cx_Oracle
*************************

cx_Oracle is a Python extension module that enables Python access to Oracle
Database.  It conforms to the `Python Database API v2.0 Specification
<https://www.python.org/dev/peps/pep-0249/>`__ with a considerable number of
additions and a couple of exclusions.

Architecture
------------

Python programs call cx_Oracle functions.  Internally cx_Oracle dynamically
loads Oracle Client libraries to access Oracle Database.  The database can be on
the same machine as Python, or it can be remote.

.. _archfig:
.. figure:: /images/cx_Oracle_arch.png

   cx_Oracle Architecture

cx_Oracle is typically installed from `PyPI
<https://pypi.org/project/cx-Oracle/>`__ using `pip
<https://pip.pypa.io/en/latest/installing/>`__.  The Oracle Client libraries
need to be installed separately.  The libraries can be from an installation of
`Oracle Instant Client
<https://www.oracle.com/database/technologies/instant-client.html>`__, from a
full Oracle Client installation, or even from an Oracle Database installation
(if Python is running on the same machine as the database).  Oracle’s standard
client-server version interoperability allows connection to both older and
newer databases from different Client library versions, see :ref:`cx_Oracle
Installation <installation>`.

Some behaviors of the Oracle Client libraries can optionally be configured with
an ``oraaccess.xml`` file, for example to enable auto-tuning of a statement
cache.  See :ref:`optclientfiles`.

The Oracle Net layer can optionally be configured with files such as
``tnsnames.ora`` and ``sqlnet.ora``, for example to enable :ref:`network
encryption <netencrypt>`.  See :ref:`optnetfiles`.

Oracle environment variables that are set before cx_Oracle first creates a
database connection will affect cx_Oracle behavior.  Optional variables include
NLS_LANG, NLS_DATE_FORMAT and TNS_ADMIN.  See :ref:`envset`.

Features
--------

The cx_Oracle feature highlights are:

    *   Easy installation from PyPI
    *   Support for multiple Oracle Client and Database versions
    *   Execution of SQL and PL/SQL statements
    *   Extensive Oracle data type support, including large objects (CLOB and
        BLOB) and binding of SQL objects
    *   Connection management, including connection pooling
    *   Oracle Database High Availability features
    *   Full use of Oracle Network Service infrastructure, including encrypted
        network traffic and security features

A complete list of supported features can be seen `here
<https://oracle.github.io/python-cx_Oracle/index.html#features>`_.

Getting Started
---------------

Install cx_Oracle using the :ref:`installation <installation>` steps.

Create a script ``query.py`` as shown below:

.. code-block:: python

    # query.py

    import cx_Oracle

    # Establish the database connection
    connection = cx_Oracle.connect(user="hr", password=userpwd,
                                   dsn="dbhost.example.com/orclpdb1")

    # Obtain a cursor
    cursor = connection.cursor()

    # Data for binding
    manager_id = 145
    first_name = "Peter"

    # Execute the query
    sql = """SELECT first_name, last_name
             FROM employees
             WHERE manager_id = :mid AND first_name = :fn"""
    cursor.execute(sql, mid=manager_id, fn=first_name)

    # Loop over the result set
    for row in cursor:
        print(row)

This uses Oracle's `sample HR schema
<https://github.com/oracle/db-sample-schemas>`__.

Simple :ref:`connection <connhandling>` to the database requires a username,
password and connection string.  Locate your Oracle Database `user name and
password <https://www.youtube.com/watch?v=WDJacg0NuLo>`_ and the database
:ref:`connection string <connstr>`, and use them in ``query.py``.  For
cx_Oracle, the connection string is commonly of the format
``hostname/servicename``, using the host name where the database is running and
the Oracle Database service name of the database instance.

The :ref:`cursor <cursorobj>` is the object that allows statements to be
executed and results (if any) fetched.

The data values in ``managerId`` and ``firstName`` are 'bound' to the statement
placeholder 'bind variables' ``:mid`` and ``:fn`` when the statement is
executed.  This separates the statement text from the data, which helps avoid
SQL Injection security risks.  :ref:`Binding <bind>` is also important for
performance and scalability.

The cursor allows rows to be iterated over and displayed.

Run the script::

    python query.py

The output is::

    ('Peter', 'Hall')
    ('Peter', 'Tucker')

Examples and Tutorials
----------------------

The `Quick Start: Developing Python Applications for Oracle Database
<https://www.oracle.com/database/technologies/appdev/python/quickstartpythononprem.html>`__
and `Quick Start: Developing Python Applications for Oracle Autonomous Database
<https://www.oracle.com/database/technologies/appdev/python/quickstartpython.html>`__
instructions have steps for Windows, Linux, and macOS.

Runnable examples are in the `GitHub samples directory
<https://github.com/oracle/python-cx_Oracle/tree/main/samples>`__.  A `Python
cx_Oracle tutorial
<https://oracle.github.io/python-cx_Oracle/samples/tutorial/Python-and-Oracle-Database-Scripting-for-the-Future.html>`__
is also available.
