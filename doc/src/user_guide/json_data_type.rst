.. _jsondatatype:

*******************************
Working with the JSON Data Type
*******************************

Native support for JSON data was introduced in Oracle database 12c. You can use
the relational database to store and query JSON data and benefit from the easy
extensibility of JSON data while retaining the performance and structure of the
relational database. JSON data is stored in the database in BLOB, CLOB or
VARCHAR2 columns. For performance reasons, it is always a good idea to store
JSON data in BLOB columns. To ensure that only JSON data is stored in that
column, use a check constraint with the clause ``is JSON`` as shown in the
following SQL to create a table containing JSON data:

.. code-block:: sql

    create table customers (
        id integer not null primary key,
        json_data blob check (json_data is json)
    );

The following Python code can then be used to insert some data into the
database:

.. code-block:: python

    import json

    customerData = dict(name="Rod", dept="Sales", location="Germany")
    cursor.execute("insert into customers (id, json_data) values (:1, :2)",
            [1, json.dumps(customerData)])

The data can be retrieved in its entirety using the following code:

.. code-block:: python

    import json

    for blob, in cursor.execute("select json_data from customers"):
        data = json.loads(blob.read())
        print(data["name"])     # will print Rod

If only the department needs to be read, the following code can be used
instead:

.. code-block:: python

    for deptName, in cursor.execute("select c.json_data.dept from customers c"):
        print(deptName)         # will print Sales

You can convert the data stored in relational tables into JSON data by using
the JSON_OBJECT SQL operator. For example:

.. code-block:: python

    import json
    cursor.execute("""
            select json_object(
                'id' value employee_id,
                'name' value (first_name || ' ' || last_name))
            from employees where rownum <= 3""")
    for value, in cursor:
        print(json.loads(value,))

The result is::

    {'id': 100, 'name': 'Steven King'}
    {'id': 101, 'name': 'Neena Kochhar'}
    {'id': 102, 'name': 'Lex De Haan'}


See `JSON Developer's Guide
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-17642E43-7D87-4590-8870-06E9FDE9A6E9>`__ for more information about
using JSON in Oracle Database.
