.. _bind:

********************
Using Bind Variables
********************

SQL and PL/SQL statements that pass data to and from Oracle Database should use
placeholders in SQL and PL/SQL statements that mark where data is supplied or
returned.  These placeholders are referred to as bind variables or bind
parameters A bind variable is a colon-prefixed identifier or numeral. For
example, there are two bind variables (``dept_id`` and ``dept_name``) in this
SQL statement:

.. code-block:: python

    sql = """insert into departments (department_id, department_name)
              values (:dept_id, :dept_name)"""
    cursor.execute(sql, [280, "Facility"])

Using bind variables is important for scalability and security.  They help avoid
SQL Injection security problems because data is never treated as part of an
executable statement.  Never concatenate or interpolate user data into SQL
statements:

.. code-block:: python

    did = 280
    dnm = "Facility"

    # !! Never do this !!
    sql = f"""insert into departments (department_id, department_name)
              values ({did}, '{dnm}')"""
    cursor.execute(sql)

Bind variables reduce parsing and execution costs when statements are executed
more than once with different data values.  If you do not use bind variables,
Oracle must reparse and cache multiple statements.  When using bind variables,
Oracle Database may be able to reuse the statement execution plan and context.

Bind variables can be used to substitute data, but cannot be used to substitute
the text of the statement.  You cannot, for example, use a bind variable where
a column name or a table name is required.  Bind variables also cannot be used
in Data Definition Language (DDL) statements, such as CREATE TABLE or ALTER
statements.

Binding By Name or Position
===========================

Binding can be done by name or by position. A named bind is performed when the
bind variables in a statement are associated with a name. For example:

.. code-block:: python

    cursor.execute("""
            insert into departments (department_id, department_name)
            values (:dept_id, :dept_name)""", dept_id=280,
            dept_name="Facility")

    # alternatively, the parameters can be passed as a dictionary instead of as
    # keyword parameters
    data = dict(dept_id=280, dept_name="Facility")
    cursor.execute("""
            insert into departments (department_id, department_name)
            values (:dept_id, :dept_name)""", data)

In the above example, the keyword parameter names or the keys of the dictionary
must match the bind variable names. The advantages of this approach are that
the location of the bind variables in the statement is not important, the
names can be meaningful and the names can be repeated while still only
supplying the value once.

A positional bind is performed when a list of bind values are passed to the
execute() call. For example:

.. code-block:: python

    cursor.execute("""
            insert into departments (department_id, department_name)
            values (:dept_id, :dept_name)""", [280, "Facility"])

Note that for SQL statements, the order of the bind values must exactly match
the order of each bind variable and duplicated names must have their values
repeated. For PL/SQL statements, however, the order of the bind values must
exactly match the order of each **unique** bind variable found in the PL/SQL
block and values should not be repeated. In order to avoid this difference,
binding by name is recommended when bind variable names are repeated.


Bind Direction
==============

The caller can supply data to the database (IN), the database can return
data to the caller (OUT) or the caller can supply initial data to the
database and the database can supply the modified data back to the caller
(IN/OUT). This is known as the bind direction.

The examples shown above have all supplied data to the database and are
therefore classified as IN bind variables. In order to have the database return
data to the caller, a variable must be created. This is done by calling the
method :func:`Cursor.var()`, which identifies the type of data that will be
found in that bind variable and its maximum size among other things.

Here is an example showing how to use OUT binds. It calculates the sum of the
integers 8 and 7 and stores the result in an OUT bind variable of type integer:

.. code-block:: python

    out_val = cursor.var(int)
    cursor.execute("""
            begin
                :out_val := :in_bind_var1 + :in_bind_var2;
            end;""",
            out_val=out_val, in_bind_var1=8, in_bind_var2=7)
    print(out_val.getvalue())        # will print 15

If instead of simply getting data back you wish to supply an initial value to
the database, you can set the variable's initial value. This example is the
same as the previous one but it sets the initial value first:

.. code-block:: python

    in_out_var = cursor.var(int)
    in_out_var.setvalue(0, 25)
    cursor.execute("""
            begin
                :in_out_bind_var := :in_out_bind_var + :in_bind_var1 +
                        :in_bind_var2;
            end;""",
            in_out_bind_var=in_out_var, in_bind_var1=8, in_bind_var2=7)
    print(in_out_var.getvalue())        # will print 40

When binding data to parameters of PL/SQL procedures that are declared as OUT
parameters, it is worth noting that any value that is set in the bind variable
will be ignored. In addition, any parameters declared as IN/OUT that do not
have a value set will start out with a value of ``null``.


Binding Null Values
===================

In cx_Oracle, null values are represented by the Python singleton ``None``.

For example:

.. code-block:: python

    cursor.execute("""
            insert into departments (department_id, department_name)
            values (:dept_id, :dept_name)""", dept_id=280, dept_name=None)

In this specific case, because the ``DEPARTMENT_NAME`` column is defined as a
``NOT NULL`` column, an error will occur::

    cx_Oracle.IntegrityError: ORA-01400: cannot insert NULL into ("HR"."DEPARTMENTS"."DEPARTMENT_NAME")


If this value is bound directly, cx_Oracle assumes it to be a string
(equivalent to a VARCHAR2 column).  If you need to use a different Oracle type
you will need to make a call to :func:`Cursor.setinputsizes()` or create a bind
variable with the correct type by calling :func:`Cursor.var()`.


Binding ROWID Values
====================

The pseudo-column ``ROWID`` uniquely identifies a row within a table.  In
cx_Oracle, ROWID values are represented as strings. The example below shows
fetching a row and then updating that row by binding its rowid:

.. code-block:: python

    # fetch the row
    cursor.execute("""
            select rowid, manager_id
            from departments
            where department_id = :dept_id""", dept_id=280)
    rowid, manager_id = cursor.fetchone()

    # update the row by binding ROWID
    cursor.execute("""
            update departments set
                manager_id = :manager_id
            where rowid = :rid""", manager_id=205, rid=rowid)


DML RETURNING Bind Variables
============================

When a RETURNING clause is used with a DML statement like UPDATE,
INSERT, or DELETE, the values are returned to the application through
the use of OUT bind variables. Consider the following example:

.. code-block:: python

    # The RETURNING INTO bind variable is a string
    dept_name = cursor.var(str)

    cursor.execute("""
            update departments set
                location_id = :loc_id
            where department_id = :dept_id
            returning department_name into :dept_name""",
            loc_id=1700, dept_id=50, dept_name=dept_name)
    print(dept_name.getvalue())     # will print ['Shipping']

In the above example, since the WHERE clause matches only one row, the output
contains a single item in the list. If the WHERE clause matched multiple rows,
however, the output would contain as many items as there were rows that were
updated.

No duplicate binds are allowed in a DML statement with a RETURNING clause, and
no duplication is allowed between bind variables in the DML section and the
RETURNING section of the statement.


LOB Bind Variables
==================

Database CLOBs, NCLOBS, BLOBs and BFILEs can be bound with types
:attr:`cx_Oracle.DB_TYPE_CLOB`, :attr:`cx_Oracle.DB_TYPE_NCLOB`,
:attr:`cx_Oracle.DB_TYPE_BLOB` and :attr:`cx_Oracle.DB_TYPE_BFILE`
respectively. LOBs fetched from the database or created with
:meth:`Connection.createlob()` can also be bound.

LOBs may represent Oracle Database persistent LOBs (those stored in tables) or
temporary LOBs (such as those created with :meth:`Connection.createlob()` or
returned by some SQL and PL/SQL operations).

LOBs can be used as IN, OUT or IN/OUT bind variables.

See :ref:`lobdata` for examples.

.. _refcur:

REF CURSOR Bind Variables
=========================

cx_Oracle provides the ability to bind and define PL/SQL REF cursors.  As an
example, consider the PL/SQL procedure:

.. code-block:: sql

    CREATE OR REPLACE PROCEDURE find_employees (
        p_query IN VARCHAR2,
        p_results OUT SYS_REFCURSOR
    ) AS
    BEGIN
        OPEN p_results FOR
            SELECT employee_id, first_name, last_name
            FROM employees
            WHERE UPPER(first_name || ' ' || last_name || ' ' || email)
                LIKE '%' || UPPER(p_query) || '%';
    END;
    /

A newly opened cursor can be bound to the REF CURSOR parameter, as shown in the
following Python code. After the PL/SQL procedure has been called with
:meth:`Cursor.callproc()`, the cursor can then be fetched just like any other
cursor which had executed a SQL query:

.. code-block:: python

    ref_cursor = connection.cursor()
    cursor.callproc("find_employees", ['Smith', ref_cursor])
    for row in ref_cursor:
        print(row)

With Oracle's `sample HR schema
<https://github.com/oracle/db-sample-schemas>`__ there are two
employees with the last name 'Smith' so the result is::

    (159, 'Lindsey', 'Smith')
    (171, 'William', 'Smith')

To return a REF CURSOR from a PL/SQL function, use ``cx_Oracle.DB_TYPE_CURSOR`` for the
return type of :meth:`Cursor.callfunc()`:

.. code-block:: python

    ref_cursor = cursor.callfunc('example_package.f_get_cursor',
                                 cx_Oracle.DB_TYPE_CURSOR)
    for row in ref_cursor:
        print(row)

See :ref:`tuning` for information on how to tune REF CURSORS.

Binding PL/SQL Collections
==========================

PL/SQL Collections like Associative Arrays can be bound as IN, OUT, and IN/OUT
variables.  When binding IN values, an array can be passed directly as shown in
this example, which sums up the lengths of all of the strings in the provided
array. First the PL/SQL package definition:

.. code-block:: sql

    create or replace package mypkg as

        type udt_StringList is table of varchar2(100) index by binary_integer;

        function DemoCollectionIn (
            a_Values            udt_StringList
        ) return number;

    end;
    /

    create or replace package body mypkg as

        function DemoCollectionIn (
            a_Values            udt_StringList
        ) return number is
            t_ReturnValue       number := 0;
        begin
            for i in 1..a_Values.count loop
                t_ReturnValue := t_ReturnValue + length(a_Values(i));
            end loop;
            return t_ReturnValue;
        end;

    end;
    /

Then the Python code:

.. code-block:: python

    values = ["String One", "String Two", "String Three"]
    return_val = cursor.callfunc("mypkg.DemoCollectionIn", int, [values])
    print(return_val)        # will print 32

In order get values back from the database, a bind variable must be created
using :meth:`Cursor.arrayvar()`. The first parameter to this method is a Python
type that cx_Oracle knows how to handle or one of the cx_Oracle :ref:`types`.
The second parameter is the maximum number of elements that the array can hold
or an array providing the value (and indirectly the maximum length). The final
parameter is optional and only used for strings and bytes. It identifies the
maximum length of the strings and bytes that can be stored in the array. If not
specified, the length defaults to 4000 bytes.

Consider the following PL/SQL package:

.. code-block:: sql

    create or replace package mypkg as

        type udt_StringList is table of varchar2(100) index by binary_integer;

        procedure DemoCollectionOut (
            a_NumElements       number,
            a_Values            out nocopy udt_StringList
        );

        procedure DemoCollectionInOut (
            a_Values            in out nocopy udt_StringList
        );

    end;
    /

    create or replace package body mypkg as

        procedure DemoCollectionOut (
            a_NumElements       number,
            a_Values            out nocopy udt_StringList
        ) is
        begin
            for i in 1..a_NumElements loop
                a_Values(i) := 'Demo out element #' || to_char(i);
            end loop;
        end;

        procedure DemoCollectionInOut (
            a_Values            in out nocopy udt_StringList
        ) is
        begin
            for i in 1..a_Values.count loop
                a_Values(i) := 'Converted element #' || to_char(i) ||
                        ' originally had length ' || length(a_Values(i));
            end loop;
        end;

    end;
    /

The Python code to process an OUT collection would look as follows. Note the
call to :meth:`Cursor.arrayvar()` which creates space for an array of strings.
Each string would permit up to 100 bytes and only 10 strings would be
permitted. If the PL/SQL block exceeds the maximum number of strings allowed
the error ``ORA-06513: PL/SQL: index for PL/SQL table out of range for host
language array`` would be raised.

.. code-block:: python

    out_array_var = cursor.arrayvar(str, 10, 100)
    cursor.callproc("mypkg.DemoCollectionOut", [5, out_array_var])
    for val in out_array_var.getvalue():
        print(val)

This would produce the following output::

    Demo out element #1
    Demo out element #2
    Demo out element #3
    Demo out element #4
    Demo out element #5

The Python code to process an IN/OUT collections is similar. Note the different
call to :meth:`Cursor.arrayvar()` which creates space for an array of strings,
but uses an array to determine both the maximum length of the array and its
initial value.

.. code-block:: python

    in_values = ["String One", "String Two", "String Three", "String Four"]
    in_out_array_var = cursor.arrayvar(str, in_values)
    cursor.callproc("mypkg.DemoCollectionInOut", [in_out_array_var])
    for val in in_out_array_var.getvalue():
        print(val)

This would produce the following output::

    Converted element #1 originally had length 10
    Converted element #2 originally had length 10
    Converted element #3 originally had length 12
    Converted element #4 originally had length 11

If an array variable needs to have an initial value but also needs to allow
for more elements than the initial value contains, the following code can be
used instead:

.. code-block:: python

    in_out_array_var = cursor.arrayvar(str, 10, 100)
    in_out_array_var.setvalue(0, ["String One", "String Two"])

All of the collections that have been bound in preceding examples have used
contiguous array elements. If an associative array with sparse array elements
is needed, a different approach is required. Consider the following PL/SQL
code:

.. code-block:: sql

    create or replace package mypkg as

        type udt_StringList is table of varchar2(100) index by binary_integer;

        procedure DemoCollectionOut (
            a_Value                         out nocopy udt_StringList
        );

    end;
    /

    create or replace package body mypkg as

        procedure DemoCollectionOut (
            a_Value                         out nocopy udt_StringList
        ) is
        begin
            a_Value(-1048576) := 'First element';
            a_Value(-576) := 'Second element';
            a_Value(284) := 'Third element';
            a_Value(8388608) := 'Fourth element';
        end;

    end;
    /

Note that the collection element indices are separated by large values. The
technique used above would fail with the exception ``ORA-06513: PL/SQL: index
for PL/SQL table out of range for host language array``. The code required to
process this collection looks like this instead:

.. code-block:: python

    collection_type = connection.gettype("MYPKG.UDT_STRINGLIST")
    collection = collection_type.newobject()
    cursor.callproc("mypkg.DemoCollectionOut", [collection])
    print(collection.aslist())

This produces the output::

    ['First element', 'Second element', 'Third element', 'Fourth element']

Note the use of :meth:`Object.aslist()` which returns the collection element
values in index order as a simple Python list. The indices themselves are lost
in this approach.  Starting from cx_Oracle 7.0, the associative array can be
turned into a Python dictionary using :meth:`Object.asdict()`. If that value
was printed in the previous example instead, the output would be::

    {-1048576: 'First element', -576: 'Second element', 284: 'Third element', 8388608: 'Fourth element'}

If the elements need to be traversed in index order, the methods
:meth:`Object.first()` and :meth:`Object.next()` can be used. The method
:meth:`Object.getelement()` can be used to acquire the element at a particular
index. This is shown in the following code:

.. code-block:: python

    ix = collection.first()
    while ix is not None:
        print(ix, "->", collection.getelement(ix))
        ix = collection.next(ix)

This produces the output::

    -1048576 -> First element
    -576 -> Second element
    284 -> Third element
    8388608 -> Fourth element

Similarly, the elements can be traversed in reverse index order using the
methods :meth:`Object.last()` and :meth:`Object.prev()` as shown in the
following code:

.. code-block:: python

    ix = collection.last()
    while ix is not None:
        print(ix, "->", collection.getelement(ix))
        ix = collection.prev(ix)

This produces the output::

    8388608 -> Fourth element
    284 -> Third element
    -576 -> Second element
    -1048576 -> First element


Binding PL/SQL Records
======================

PL/SQL record type objects can also be bound for IN, OUT and IN/OUT
bind variables.  For example:

.. code-block:: sql

    create or replace package mypkg as

        type udt_DemoRecord is record (
            NumberValue                     number,
            StringValue                     varchar2(30),
            DateValue                       date,
            BooleanValue                    boolean
        );

        procedure DemoRecordsInOut (
            a_Value                         in out nocopy udt_DemoRecord
        );

    end;
    /

    create or replace package body mypkg as

        procedure DemoRecordsInOut (
            a_Value                         in out nocopy udt_DemoRecord
        ) is
        begin
            a_Value.NumberValue := a_Value.NumberValue * 2;
            a_Value.StringValue := a_Value.StringValue || ' (Modified)';
            a_Value.DateValue := a_Value.DateValue + 5;
            a_Value.BooleanValue := not a_Value.BooleanValue;
        end;

    end;
    /

Then this Python code can be used to call the stored procedure which will
update the record:

.. code-block:: python

    # create and populate a record
    record_type = connection.gettype("MYPKG.UDT_DEMORECORD")
    record = record_type.newobject()
    record.NUMBERVALUE = 6
    record.STRINGVALUE = "Test String"
    record.DATEVALUE = datetime.datetime(2016, 5, 28)
    record.BOOLEANVALUE = False

    # show the original values
    print("NUMBERVALUE ->", record.NUMBERVALUE)
    print("STRINGVALUE ->", record.STRINGVALUE)
    print("DATEVALUE ->", record.DATEVALUE)
    print("BOOLEANVALUE ->", record.BOOLEANVALUE)
    print()

    # call the stored procedure which will modify the record
    cursor.callproc("mypkg.DemoRecordsInOut", [record])

    # show the modified values
    print("NUMBERVALUE ->", record.NUMBERVALUE)
    print("STRINGVALUE ->", record.STRINGVALUE)
    print("DATEVALUE ->", record.DATEVALUE)
    print("BOOLEANVALUE ->", record.BOOLEANVALUE)

This will produce the following output::

    NUMBERVALUE -> 6
    STRINGVALUE -> Test String
    DATEVALUE -> 2016-05-28 00:00:00
    BOOLEANVALUE -> False

    NUMBERVALUE -> 12
    STRINGVALUE -> Test String (Modified)
    DATEVALUE -> 2016-06-02 00:00:00
    BOOLEANVALUE -> True

Note that when manipulating records, all of the attributes must be set by the
Python program in order to avoid an Oracle Client bug which will result in
unexpected values or the Python application segfaulting.

.. _spatial:

Binding Spatial Datatypes
=========================

Oracle Spatial datatypes objects can be represented by Python objects and their
attribute values can be read and updated. The objects can further be bound and
committed to database. This is similar to the examples above.

An example of fetching SDO_GEOMETRY is in :ref:`Oracle Database Objects and
Collections <fetchobjects>`.


.. _inputtypehandlers:

Changing Bind Data Types using an Input Type Handler
====================================================

Input Type Handlers allow applications to change how data is bound to
statements, or even to enable new types to be bound directly.

An input type handler is enabled by setting the attribute
:attr:`Cursor.inputtypehandler` or :attr:`Connection.inputtypehandler`.

Input type handlers can be combined with variable converters to bind Python
objects seamlessly:

.. code-block:: python

    # A standard Python object
    class Building:

        def __init__(self, build_id, description, num_floors, date_built):
            self.building_id = build_id
            self.description = description
            self.num_floors = num_floors
            self.date_built = date_built

    building = Building(1, "Skyscraper 1", 5, datetime.date(2001, 5, 24))

    # Get Python representation of the Oracle user defined type UDT_BUILDING
    obj_type = con.gettype("UDT_BUILDING")

    # convert a Python Building object to the Oracle user defined type
    # UDT_BUILDING
    def building_in_converter(value):
        obj = obj_type.newobject()
        obj.BUILDINGID = value.building_id
        obj.DESCRIPTION = value.description
        obj.NUMFLOORS = value.num_floors
        obj.DATEBUILT = value.date_built
        return obj

    def input_type_handler(cursor, value, num_elements):
        if isinstance(value, Building):
            return cursor.var(obj_type, arraysize=num_elements,
                              inconverter=building_in_converter)


    # With the input type handler, the bound Python object is converted
    # to the required Oracle object before being inserted
    cur.inputtypehandler = input_type_handler
    cur.execute("insert into myTable values (:1, :2)", (1, building))


Binding Multiple Values to a SQL WHERE IN Clause
================================================

To use an IN clause with multiple values in a WHERE clause, you must define and
bind multiple values. You cannot bind an array of values. For example:

.. code-block:: python

    cursor.execute("""
            select employee_id, first_name, last_name
            from employees
            where last_name in (:name1, :name2)""",
            name1="Smith", name2="Taylor")
    for row in cursor:
        print(row)

This will produce the following output::

    (159, 'Lindsey', 'Smith')
    (171, 'William', 'Smith')
    (176, 'Jonathon', 'Taylor')
    (180, 'Winston', 'Taylor')

If this sort of query is executed multiple times with differing numbers of
values, a bind variable should be included for each possible value up to the
maximum number of values that can be provided. Missing values can be bound with
the value ``None``. For example, if the query above is used for up to 5 values,
the code should be adjusted as follows:

.. code-block:: python

    cursor.execute("""
            select employee_id, first_name, last_name
            from employees
            where last_name in (:name1, :name2, :name3, :name4, :name5)""",
            name1="Smith", name2="Taylor", name3=None, name4=None, name5=None)
    for row in cursor:
        print(row)

This will produce the same output as the original example.

If the number of values is only going to be known at runtime, then a SQL
statement can be built up as follows:

.. code-block:: python

    bind_values = ["Gates", "Marvin", "Fay"]
    bind_names = [":" + str(i + 1) for i in range(len(bind_values))]
    sql = "select employee_id, first_name, last_name from employees " + \
            "where last_name in (%s)" % (",".join(bind_names))
    cursor.execute(sql, bind_values)
    for row in cursor:
        print(row)

Another solution for a larger number of values is to construct a SQL
statement like::

    SELECT ... WHERE col IN ( <something that returns a list of rows> )

The easiest way to do the '<something that returns a list of rows>'
will depend on how the data is initially represented and the number of
items.  You might look at using CONNECT BY or nested tables.  Or,
for really large numbers of items, you might prefer to use a global
temporary table.

Binding Column and Table Names
==============================

Column and table names cannot be bound in SQL queries.  You can concatenate
text to build up a SQL statement, but make sure you use an Allow List or other
means to validate the data in order to avoid SQL Injection security issues:

.. code-block:: python

    table_allow_list = ['employees', 'departments']
    table_name = get_table_name() #  get the table name from user input
    if table_name.lower() not in table_allow_list:
        raise Exception('Invalid table name')
    sql = f'select * from {table_name}'

Binding column names can be done either by using the above method or by using a
CASE statement.  The example below demonstrates binding a column name in an
ORDER BY clause:

.. code-block:: python

    sql = """
            SELECT * FROM departments
            ORDER BY
                CASE :bindvar
                    WHEN 'department_id' THEN DEPARTMENT_ID
                    ELSE MANAGER_ID
                END"""

    col_name = get_column_name() # Obtain a column name from the user
    cursor.execute(sql, [col_name])

Depending on the name provided by the user, the query results will be
ordered either by the column ``DEPARTMENT_ID`` or the column ``MANAGER_ID``.
