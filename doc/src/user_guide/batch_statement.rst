.. _batchstmnt:

******************************************
Batch Statement Execution and Bulk Loading
******************************************

Inserting or updating multiple rows can be performed efficiently with
:meth:`Cursor.executemany()`, making it easy to work with large data sets with
cx_Oracle.  This method can significantly outperform repeated calls to
:meth:`Cursor.execute()` by reducing network transfer costs and database overheads.
The :meth:`~Cursor.executemany()` method can also be used to execute PL/SQL
statements multiple times at once.

There are examples in the `GitHub examples
<https://github.com/oracle/python-cx_Oracle/tree/main/samples>`__
directory.

The following tables will be used in the samples that follow:

.. code-block:: sql

    create table ParentTable (
        ParentId              number(9) not null,
        Description           varchar2(60) not null,
        constraint ParentTable_pk primary key (ParentId)
    );

    create table ChildTable (
        ChildId               number(9) not null,
        ParentId              number(9) not null,
        Description           varchar2(60) not null,
        constraint ChildTable_pk primary key (ChildId),
        constraint ChildTable_fk foreign key (ParentId)
                references ParentTable
    );


Batch Execution of SQL
======================

The following example inserts five rows into the table ``ParentTable``:

.. code-block:: python

    data = [
        (10, 'Parent 10'),
        (20, 'Parent 20'),
        (30, 'Parent 30'),
        (40, 'Parent 40'),
        (50, 'Parent 50')
    ]
    cursor.executemany("insert into ParentTable values (:1, :2)", data)

This code requires only one :ref:`round-trip <roundtrips>` from the client to
the database instead of the five round-trips that would be required for
repeated calls to :meth:`~Cursor.execute()`.  For very large data sets there
may be an external buffer or network limits to how many rows can be processed,
so repeated calls to ``executemany()`` may be required.  The limits are based
on both the number of rows being processed as well as the "size" of each row
that is being processed.  Repeated calls to :meth:`~Cursor.executemany()` are
still better than repeated calls to :meth:`~Cursor.execute()`.


Batch Execution of PL/SQL
=========================

PL/SQL functions and procedures and anonymous PL/SQL blocks can also be called
using :meth:`~Cursor.executemany()` in order to improve performance. For
example:

.. code-block:: python

    data = [
        (10, 'Parent 10'),
        (20, 'Parent 20'),
        (30, 'Parent 30'),
        (40, 'Parent 40'),
        (50, 'Parent 50')
    ]
    cursor.executemany("begin mypkg.create_parent(:1, :2); end;", data)

Note that the ``batcherrors`` parameter (discussed below) cannot be used with
PL/SQL block execution.


Handling Data Errors
====================

Large datasets may contain some invalid data.  When using batch execution as
discussed above, the entire batch will be discarded if a single error is
detected, potentially eliminating the performance benefits of batch execution
and increasing the complexity of the code required to handle those errors. If
the parameter ``batchErrors`` is set to the value ``True`` when calling
:meth:`~Cursor.executemany()`, however, processing will continue even if there
are data errors in some rows, and the rows containing errors can be examined
afterwards to determine what course the application should take. Note that if
any errors are detected, a transaction will be started but not committed, even
if :attr:`Connection.autocommit` is set to ``True``. After examining the errors
and deciding what to do with them, the application needs to explicitly commit
or roll back the transaction with :meth:`Connection.commit()` or
:meth:`Connection.rollback()`, as needed.

This example shows how data errors can be identified:

.. code-block:: python

    data = [
        (60, 'Parent 60'),
        (70, 'Parent 70'),
        (70, 'Parent 70 (duplicate)'),
        (80, 'Parent 80'),
        (80, 'Parent 80 (duplicate)'),
        (90, 'Parent 90')
    ]
    cursor.executemany("insert into ParentTable values (:1, :2)", data,
                       batcherrors=True)
    for error in cursor.getbatcherrors():
        print("Error", error.message, "at row offset", error.offset)

The output is::

    Error ORA-00001: unique constraint (PYTHONDEMO.PARENTTABLE_PK) violated at row offset 2
    Error ORA-00001: unique constraint (PYTHONDEMO.PARENTTABLE_PK) violated at row offset 4

The row offset is the index into the array of the data that could not be
inserted due to errors.  The application could choose to commit or rollback the
other rows that were successfully inserted.  Alternatively, it could correct
the data for the two invalid rows and attempt to insert them again before
committing.


Identifying Affected Rows
=========================

When executing a DML statement using :meth:`~Cursor.execute()`, the number of
rows affected can be examined by looking at the attribute
:attr:`~Cursor.rowcount`. When performing batch executing with
:meth:`Cursor.executemany()`, however, the row count will return the *total*
number of rows that were affected. If you want to know the total number of rows
affected by each row of data that is bound you must set the parameter
``arraydmlrowcounts`` to ``True``, as shown:

.. code-block:: python

    parent_ids_to_delete = [20, 30, 50]
    cursor.executemany("delete from ChildTable where ParentId = :1",
                       [(i,) for i in parent_ids_to_delete],
                       arraydmlrowcounts=True)
    row_counts = cursor.getarraydmlrowcounts()
    for parent_id, count in zip(parent_ids_to_delete, row_counts):
        print("Parent ID:", parent_id, "deleted", count, "rows.")

Using the data found in the `GitHub samples
<https://github.com/oracle/python-cx_Oracle/tree/main/samples>`__ the output
is as follows::

    Parent ID: 20 deleted 3 rows.
    Parent ID: 30 deleted 2 rows.
    Parent ID: 50 deleted 4 rows.


DML RETURNING
=============

DML statements like INSERT, UPDATE, DELETE and MERGE can return values by using
the DML RETURNING syntax. A bind variable can be created to accept this data.
See :ref:`bind` for more information.

If, instead of merely deleting the rows as shown in the previous example, you
also wanted to know some information about each of the rows that were deleted,
you could use the following code:

.. code-block:: python

    parent_ids_to_delete = [20, 30, 50]
    child_id_var = cursor.var(int, arraysize=len(parent_ids_to_delete))
    cursor.setinputsizes(None, child_id_var)
    cursor.executemany("""
            delete from ChildTable
            where ParentId = :1
            returning ChildId into :2""",
            [(i,) for i in parent_ids_to_delete])
    for ix, parent_id in enumerate(parent_ids_to_delete):
        print("Child IDs deleted for parent ID", parent_id, "are",
              child_id_var.getvalue(ix))

The output would then be::

    Child IDs deleted for parent ID 20 are [1002, 1003, 1004]
    Child IDs deleted for parent ID 30 are [1005, 1006]
    Child IDs deleted for parent ID 50 are [1012, 1013, 1014, 1015]

Note that the bind variable created to accept the returned data must have an
arraysize large enough to hold data for each row that is processed. Also,
the call to :meth:`Cursor.setinputsizes()` binds this variable immediately so
that it does not have to be passed in each row of data.


Predefining Memory Areas
========================

When multiple rows of data are being processed there is the possibility that
the data is not uniform in type and size.  In such cases, cx_Oracle makes some
effort to accommodate such differences.  Type determination for each column is
deferred until a value that is not ``None`` is found in the column's data.  If
all values in a particular column are ``None``, then cx_Oracle assumes the type
is a string and has a length of 1.  cx_Oracle will also adjust the size of the
buffers used to store strings and bytes when a longer value is encountered in
the data.  These sorts of operations incur overhead as memory has to be
reallocated and data copied.  To eliminate this overhead, using
:meth:`~Cursor.setinputsizes()` tells cx_Oracle about the type and size of the
data that is going to be used.

Consider the following code:

.. code-block:: python

    data = [
        (110, "Parent 110"),
        (2000, "Parent 2000"),
        (30000, "Parent 30000"),
        (400000, "Parent 400000"),
        (5000000, "Parent 5000000")
    ]
    cursor.setinputsizes(None, 20)
    cursor.executemany("""
            insert into ParentTable (ParentId, Description)
            values (:1, :2)""", data)

In this example, without the call to :meth:`~Cursor.setinputsizes()`, cx_Oracle
would perform five allocations of increasing size as it discovered each new,
longer string.  However ``cursor.setinputsizes(None, 20)`` tells cx_Oracle that
the maximum size of the strings that will be processed is 20 characters.  Since
cx_Oracle allocates memory for each row based on this value, it is best not to
oversize it. The first parameter of ``None`` tells cx_Oracle that its default
processing will be sufficient.

Loading CSV Files into Oracle Database
======================================

The :meth:`Cursor.executemany()` method and `csv module
<https://docs.python.org/3/library/csv.html#module-csv>`__ can be used to
efficiently load CSV (Comma Separated Values) files.  For example, consider the
file ``data.csv``::

    101,Abel
    154,Baker
    132,Charlie
    199,Delta
    . . .

And the schema:

.. code-block:: sql

    create table test (id number, name varchar2(25));

Instead of looping through each line of the CSV file and inserting it
individually, you can insert batches of records using
:meth:`Cursor.executemany()`:

.. code-block:: python

    import cx_Oracle
    import csv

    . . .

    # Predefine the memory areas to match the table definition
    cursor.setinputsizes(None, 25)

    # Adjust the batch size to meet your memory and performance requirements
    batch_size = 10000

    with open('testsp.csv', 'r') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        sql = "insert into test (id,name) values (:1, :2)"
        data = []
        for line in csv_reader:
            data.append((line[0], line[1]))
            if len(data) % batch_size == 0:
                cursor.executemany(sql, data)
                data = []
        if data:
            cursor.executemany(sql, data)
        con.commit()
