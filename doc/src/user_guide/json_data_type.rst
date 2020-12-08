.. _jsondatatype:

*******************************
Working with the JSON Data Type
*******************************

Native support for JSON data was introduced in Oracle Database 12c.  You can
use JSON with relational database features, including transactions, indexing,
declarative querying, and views.  You can project JSON data relationally,
making it available for relational processes and tools.  Also see
:ref:`Simple Oracle Document Access (SODA) <sodausermanual>`, which allows
access to JSON documents through a set of NoSQL-style APIs.

Prior to Oracle Database 21, JSON in relational tables is stored as BLOB, CLOB
or VARCHAR2 data, allowing easy access with cx_Oracle.  Oracle Database 21
introduced a dedicated JSON data type with a new `binary storage format
<https://blogs.oracle.com/jsondb/osonformat>`__ that improves performance and
functionality.  To use the new dedicated JSON type, the Oracle Database and
Oracle Client libraries must be version 21, or later.  Also cx_Oracle must be
8.1, or later.

For more information about using JSON in Oracle Database see the
`Database JSON Developer's Guide
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=ADJSN>`__.

In Oracle Database 21, to create a table with a column called ``JSON_DATA`` for
JSON data:

.. code-block:: sql

    create table customers (
        id integer not null primary key,
        json_data json
    );

For older Oracle Database versions the syntax is:

.. code-block:: sql

    create table customers (
        id integer not null primary key,
        json_data blob check (json_data is json)
    );

The check constraint with the clause ``IS JSON`` ensures only JSON data is
stored in that column.

The older syntax can still be used in Oracle Database 21, however the
recommendation is to move to the new JSON type.  With the old syntax, the
storage can be BLOB, CLOB or VARCHAR2.  Of these, BLOB is preferred to avoid
character set conversion overheads.

Using Oracle Database 21 and Oracle Client 21 with cx_Oracle 8.1 (or later),
you can insert by binding as shown below:

.. code-block:: python

    import datetime

    json_data = [
        2.78,
        True,
        'Ocean Beach',
        b'Some bytes',
        {'keyA': 1, 'KeyB': 'Melbourne'},
        datetime.date.today()
    ]

    var = cursor.var(cx_Oracle.DB_TYPE_JSON)
    var.setvalue(0, json_data)
    cursor.execute("insert into customers values (:1, :2)", [123, var])

    # or these two lines can replace the three previous lines
    cursor.setinputsizes(None, cx_Oracle.DB_TYPE_JSON)
    cursor.execute("insert into customers values (:1, :2)", [123, json_data])

Fetching with:

.. code-block:: python

    for row in cursor.execute("SELECT c.json_data FROM customers c"):
        print(row)

gives output like::

    ([Decimal('2.78'), True, 'Ocean Beach',
        b'Some bytes',
        {'keyA': Decimal('1'), 'KeyB': 'Melbourne'},
        datetime.datetime(2020, 12, 2, 0, 0)],)

With the older BLOB storage, or to insert JSON strings, use:

.. code-block:: python

    import json

    customer_data = dict(name="Rod", dept="Sales", location="Germany")
    cursor.execute("insert into customers (id, json_data) values (:1, :2)",
                   [1, json.dumps(customer_data)])


IN Bind Type Mapping
====================

When binding to a JSON value, the type parameter for the variable must be
specified as :data:`cx_Oracle.DB_TYPE_JSON`. Python values are converted to
JSON values as shown in the following table.  The 'SQL Equivalent' syntax can
be used in SQL INSERT and UPDATE statements if specific attribute types are
needed but there is no direct mapping from Python.

.. list-table::
    :header-rows: 1
    :widths: 1 1 1
    :align: left

    * - Python Type or Value
      - JSON Attribute Type or Value
      - SQL Equivalent Example
    * - None
      - null
      - NULL
    * - True
      - true
      - n/a
    * - False
      - false
      - n/a
    * - int
      - NUMBER
      - json_scalar(1)
    * - float
      - NUMBER
      - json_scalar(1)
    * - decimal.Decimal
      - NUMBER
      - json_scalar(1)
    * - str
      - VARCHAR2
      - json_scalar('String')
    * - datetime.date
      - TIMESTAMP
      - json_scalar(to_timestamp('2020-03-10', 'YYYY-MM-DD'))
    * - datetime.datetime
      - TIMESTAMP
      - json_scalar(to_timestamp('2020-03-10', 'YYYY-MM-DD'))
    * - bytes
      - RAW
      - json_scalar(utl_raw.cast_to_raw('A raw value'))
    * - list
      - Array
      - json_array(1, 2, 3 returning json)
    * - dict
      - Object
      - json_object(key 'Fred' value json_scalar(5), key 'George' value json_scalar('A string') returning json)
    * - n/a
      - CLOB
      - json_scalar(to_clob('A short CLOB'))
    * - n/a
      - BLOB
      - json_scalar(to_blob(utl_raw.cast_to_raw('A short BLOB')))
    * - n/a
      - DATE
      - json_scalar(to_date('2020-03-10', 'YYYY-MM-DD'))
    * - n/a
      - INTERVAL YEAR TO MONTH
      - json_scalar(to_yminterval('+5-9'))
    * - n/a
      - INTERVAL DAY TO SECOND
      - json_scalar(to_dsinterval('P25DT8H25M'))
    * - n/a
      - BINARY_DOUBLE
      - json_scalar(to_binary_double(25))
    * - n/a
      - BINARY_FLOAT
      - json_scalar(to_binary_float(15.5))

An example of creating a CLOB attribute with key ``mydocument`` in a JSON column
using SQL is:

.. code-block:: python

    cursor.execute("""
            insert into mytab (myjsoncol) values
            (json_object(key 'mydocument' value json_scalar(to_clob(:b))
                    returning json))""",
            ['A short CLOB'])

When `mytab` is queried in cx_Oracle, the CLOB data will be returned as a
Python string, as shown by the following table.  Output might be like::

    {mydocument: 'A short CLOB'}


Query and OUT Bind Type Mapping
===============================

When getting Oracle Database 21 JSON values from the database, the following
attribute mapping occurs:

.. list-table::
    :header-rows: 1
    :widths: 1 1
    :align: left

    * - Database JSON Attribute Type or Value
      - Python Type or Value
    * - null
      - None
    * - false
      - False
    * - true
      - True
    * - NUMBER
      - decimal.Decimal
    * - VARCHAR2
      - str
    * - RAW
      - bytes
    * - CLOB
      - str
    * - BLOB
      - bytes
    * - DATE
      - datetime.datetime
    * - TIMESTAMP
      - datetime.datetime
    * - INTERVAL YEAR TO MONTH
      - not supported
    * - INTERVAL DAY TO SECOND
      - datetime.timedelta
    * - BINARY_DOUBLE
      - float
    * - BINARY_FLOAT
      - float
    * - Arrays
      - list
    * - Objects
      - dict

SQL/JSON Path Expressions
=========================

Oracle Database provides SQL access to JSON data using SQL/JSON path
expressions.  A path expression selects zero or more JSON values that match, or
satisfy, it.  Path expressions can use wildcards and array ranges.  A simple
path expression is ``$.friends`` which is the value of the JSON field
``friends``.

For example, the previously created ``customers`` table with JSON column
``json_data`` can be queried like:

.. code-block:: sql

    select c.json_data.location FROM customers c

With the JSON ``'{"name":"Rod","dept":"Sales","location":"Germany"}'`` stored
in the table, the queried value would be ``Germany``.

The JSON_EXISTS functions tests for the existence of a particular value within
some JSON data.  To look for JSON entries that have a ``location`` field:

.. code-block:: python

    for blob, in cursor.execute("""
            select json_data
            from customers
            where json_exists(json_data, '$.location')"""):
        data = json.loads(blob.read())
        print(data)

This query might display::

    {'name': 'Rod', 'dept': 'Sales', 'location': 'Germany'}

The SQL/JSON functions ``JSON_VALUE`` and ``JSON_QUERY`` can also be used.

Note that the default error-handling behavior for these functions is
``NULL ON ERROR``, which means that no value is returned if an error occurs.
To ensure that an error is raised, use ``ERROR ON ERROR``.

For more information, see `SQL/JSON Path Expressions
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-2DC05D71-3D62-4A14-855F-76E054032494>`__
in the Oracle JSON Developer's Guide.


Accessing Relational Data as JSON
=================================

In Oracle Database 12.2, or later, the `JSON_OBJECT
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-1EF347AE-7FDA-4B41-AFE0-DD5A49E8B370>`__
function is a great way to convert relational table data to JSON:

.. code-block:: python

    cursor.execute("""
            select json_object('deptId' is d.department_id, 'name' is d.department_name) department
            from departments d
            where department_id < :did
            order by d.department_id""",
            [50]);
    for row in cursor:
        print(row)

This produces::

    ('{"deptId":10,"name":"Administration"}',)
    ('{"deptId":20,"name":"Marketing"}',)
    ('{"deptId":30,"name":"Purchasing"}',)
    ('{"deptId":40,"name":"Human Resources"}',)
