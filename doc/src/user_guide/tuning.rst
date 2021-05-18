.. _tuning:

****************
Tuning cx_Oracle
****************

Some general tuning tips are:

* Tune your application architecture.

  A general application goal is to reduce the number of :ref:`round-trips
  <roundtrips>` between cx_Oracle and the database.

  For multi-user applications, make use of connection pooling.  Create the pool
  once during application initialization.  Do not oversize the pool, see
  :ref:`connpool` .  Use a session callback function to set session state, see
  :ref:`Session CallBacks for Setting Pooled Connection State <sessioncallback>`.

  Make use of efficient cx_Oracle functions.  For example, to insert
  multiple rows use :meth:`Cursor.executemany()` instead of
  :meth:`Cursor.execute()`.

* Tune your SQL statements.  See the `SQL Tuning Guide
  <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=TGSQL>`__.

  Use :ref:`bind variables <bind>` to avoid statement reparsing.

  Tune :attr:`Cursor.arraysize` and :attr:`Cursor.prefetchrows` for each query,
  see :ref:`Tuning Fetch Performance <tuningfetch>`.

  Do simple optimizations like :ref:`limiting the number of rows <rowlimit>` and
  avoiding selecting columns not used in the application.

  It may be faster to work with simple scalar relational values than to use
  Oracle Database object types.

  Make good use of PL/SQL to avoid executing many individual statements from
  cx_Oracle.

  Tune the :ref:`Statement Cache <stmtcache>`.

  Enable :ref:`Client Result Caching <clientresultcache>` for small lookup tables.

* Tune your database.  See the `Database Performance Tuning Guide
  <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=TGDBA>`__.

* Tune your network.  For example, when inserting or retrieving a large number
  of rows (or for large data), or when using a slow network, then tune the
  Oracle Network Session Data Unit (SDU) and socket buffer sizes, see `Oracle
  Net Services: Best Practices for Database Performance and High Availability
  <https://static.rainfocus.com/oracle/oow19/sess/1553616880266001WLIh/PF/OOW19_Net_CON4641_1569022126580001esUl.pdf>`__.

* Do not commit or rollback unnecessarily.  Use :attr:`Connection.autocommit` on
  the last of a sequence of DML statements.

.. _tuningfetch:

Tuning Fetch Performance
========================

To tune queries you can adjust cx_Oracle's internal buffer sizes to improve the
speed of fetching rows across the network from the database, and to optimize
memory usage.  Regardless of which cx_Oracle method is used to get query
results, internally all rows are fetched in batches from the database and
buffered before being returned to the application.  The internal buffer sizes
can have a significant performance impact.  The sizes do not affect how, or
when, rows are returned to your application.  They do not affect the minimum or
maximum number of rows returned by a query.

For best performance, tune "array fetching" with :attr:`Cursor.arraysize` and
"row prefetching" with :attr:`Cursor.prefetchrows` before calling
:meth:`Cursor.execute()`.  Queries that return LOBs and similar types will never
prefetch rows, so the ``prefetchrows`` value is ignored in those cases.

The common query tuning scenario is for SELECT statements that return a large
number of rows over a slow network.  Increasing ``arraysize`` can improve
performance by reducing the number of :ref:`round-trips <roundtrips>` to the
database.  However increasing this value increases the amount of memory
required.  Adjusting ``prefetchrows`` will also affect performance and memory
usage.

Row prefetching and array fetching are both internal buffering techniques to
reduce :ref:`round-trips <roundtrips>` to the database. The difference is the
code layer that is doing the buffering, and when the buffering occurs.  The
Oracle Client libraries used by cx_Oracle have separate "execute SQL statement"
and "fetch data" calls.  Prefetching allows query results to be returned to the
application when the successful statement execution acknowledgment is returned
from the database.  This means that a subsequent internal "fetch data" operation
does not always need to make a round-trip to the database because rows are
already buffered in the Oracle Client libraries.  Reducing round-trips helps
performance and scalability.  An overhead of prefetching is the need for an
additional data copy from Oracle Client's prefetch buffers.

Choosing values for ``arraysize`` and ``prefetchrows``
++++++++++++++++++++++++++++++++++++++++++++++++++++++

The best :attr:`Cursor.arraysize` and :attr:`Cursor.prefetchrows` values can be
found by experimenting with your application under the expected load of normal
application use.  This is because the cost of the extra memory copy from the
prefetch buffers when fetching a large quantity of rows or very "wide" rows may
outweigh the cost of a round-trip for a single cx_Oracle user on a fast network.
However under production application load, the reduction of round-trips may help
performance and overall system scalability. The documentation in
:ref:`round-trips <roundtrips>` shows how to measure round-trips.

Here are some suggestions for the starting point to begin your tuning:

* To tune queries that return an unknown number of rows, estimate the number of
  rows returned and start with an appropriate :attr:`Cursor.arraysize` value.
  The default is 100.  Then set :attr:`Cursor.prefetchrows` to the ``arraysize``
  value.  For example:

  .. code-block:: python

      cur = connection.cursor()

      cur.prefetchrows = 1000
      cur.arraysize = 1000

      for row in cur.execute("SELECT * FROM very_big_table"):
          print(row)

  Adjust the values as needed for performance, memory and round-trip usage.  Do
  not make the sizes unnecessarily large.  For a large quantity of rows or very
  "wide" rows on fast networks you may prefer to leave ``prefetchrows`` at its
  default value of 2. Keep ``arraysize`` as big, or bigger than,
  ``prefetchrows``.

* If you are fetching a fixed number of rows, start your tuning by setting
  ``arraysize`` to the number of expected rows, and set ``prefetchrows`` to one
  greater than this value.  (Adding one removes the need for a round-trip to check
  for end-of-fetch).  For example, if you are querying 20 rows, perhaps to
  :ref:`display a page <rowlimit>` of data, set ``prefetchrows`` to 21 and
  ``arraysize`` to 20:

  .. code-block:: python

      cur = connection.cursor()

      cur.prefetchrows = 21
      cur.arraysize = 20

      for row in cur.execute("""
          SELECT last_name
             FROM employees
             ORDER BY last_name
             OFFSET 0 ROWS FETCH NEXT 20 ROWS ONLY"""):
          print(row)

  This will return all rows for the query in one round-trip.

* If you know that a query returns just one row then set :attr:`Cursor.arraysize`
  to 1 to minimize memory usage.  The default prefetch value of 2 allows minimal
  round-trips for single-row queries:

  .. code-block:: python

      cur = connection.cursor()
      cur.arraysize = 1
      cur.execute("select * from MyTable where id = 1"):
      row = cur.fetchone()
      print(row)

In cx_Oracle, the ``arraysize`` and ``prefetchrows`` values are only examined
when a statement is executed the first time.  To change the values, create a new
cursor.  For example, to change ``arraysize`` for a repeated statement:

.. code-block:: python

    array_sizes = (10, 100, 1000)
    for size in array_sizes:
        cursor = connection.cursor()
        cursor.arraysize = size
        start = time.time()
        cursor.execute(sql).fetchall()
        elapsed = time.time() - start
        print("Time for", size, elapsed, "seconds")

There are two cases that will benefit from setting :attr:`Cursor.prefetchrows`
to 0:

* When passing REF CURSORS into PL/SQL packages.  Setting ``prefetchrows`` to 0
  can stop rows being prematurely (and silently) fetched into cx_Oracle's
  internal buffers, making them unavailable to the PL/SQL code that receives the
  REF CURSOR.

* When querying a PL/SQL function that uses PIPE ROW to emit rows at
  intermittent intervals.  By default, several rows needs to be emitted by the
  function before cx_Oracle can return them to the application.  Setting
  ``prefetchrows`` to 0 helps give a consistent flow of data to the application.

Prefetching can also be enabled in an external :ref:`oraaccess.xml
<optclientfiles>` file, which may be useful for tuning an application when
modifying its code is not feasible.  Setting the size in ``oraaccess.xml`` will
affect the whole application, so it should not be the first tuning choice.

One place where increasing ``arraysize`` is particularly useful is in copying
data from one database to another:

.. code-block:: python

    # setup cursors
    source_cursor = source_connection.cursor()
    source_cursor.arraysize = 1000
    target_cursor = target_connection.cursor()

    # perform fetch and bulk insertion
    source_cursor.execute("select * from MyTable")
    while True:
        rows = source_cursor.fetchmany()
        if not rows:
            break
        target_cursor.executemany("insert into MyTable values (:1, :2)", rows)
        target_connection.commit()

Tuning REF CURSORS
++++++++++++++++++

In cx_Oracle, REF CURSORS can also be tuned by setting the values of ``arraysize``
and ``prefetchrows``. The prefetchrows value must be set before calling the PL/SQL
procedure as the REF CURSOR is executed on the server.

For example:

.. code-block:: python

    # Set the arraysize and prefetch rows of the REF cursor
    ref_cursor = connection.cursor()
    ref_cursor.prefetchrows = 1000
    ref_cursor.arraysize = 1000

    # Perform the tuned fetch
    sum_rows = 0
    cursor.callproc("myrefcursorproc", [ref_cursor])
    print("Sum of IntCol for", num_rows, "rows:")
    for row in ref_cursor:
        sum_rows += row[0]
    print(sum_rows)

.. _roundtrips:

Database Round-trips
====================

A round-trip is defined as the trip from the Oracle Client libraries (used by
cx_Oracle) to the database and back.  Calling each cx_Oracle function, or
accessing each attribute, will require zero or more round-trips.  Along with
tuning an application's architecture and `tuning its SQL statements
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=TGSQL>`__, a general
performance and scalability goal is to minimize `round-trips
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-9B2F05F9-D841-4493-A42D-A7D89694A2D1>`__.

Some general tips for reducing round-trips are:

* Tune :attr:`Cursor.arraysize` and :attr:`Cursor.prefetchrows` for each query.
* Use :meth:`Cursor.executemany()` for optimal DML execution.
* Only commit when necessary.  Use :attr:`Connection.autocommit` on the last statement of a transaction.
* For connection pools, use a callback to set connection state, see :ref:`Session CallBacks for Setting Pooled Connection State <sessioncallback>`.
* Make use of PL/SQL procedures which execute multiple SQL statements instead of executing them individually from cx_Oracle.
* Use scalar types instead of Oracle Database object types.
* Avoid overuse of :meth:`Connection.ping()`.
* Avoid setting :data:`SessionPool.ping_interval` to 0 or a small value.
* When using SODA, use pooled connections and enable the :ref:`SODA metadata cache <sodametadatacache>`.

Finding the Number of Round-Trips
+++++++++++++++++++++++++++++++++

Oracle's `Automatic Workload Repository
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-56AEF38E-9400-427B-A818-EDEC145F7ACD>`__
(AWR) reports show 'SQL*Net roundtrips to/from client' and are useful for
finding the overall behavior of a system.

Sometimes you may wish to find the number of round-trips used for a
specific application.  Snapshots of the ``V$SESSTAT`` view taken before
and after doing some work can be used for this:

.. code-block:: sql

    SELECT ss.value, sn.display_name
    FROM v$sesstat ss, v$statname sn
    WHERE ss.sid = SYS_CONTEXT('USERENV','SID')
    AND ss.statistic# = sn.statistic#
    AND sn.name LIKE '%roundtrip%client%';

.. _stmtcache:

Statement Caching
=================

cx_Oracle's :meth:`Cursor.execute()` and :meth:`Cursor.executemany()` functions
use the `Oracle Call Interface statement cache
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-4947CAE8-1F00-4897-BB2B-7F921E495175>`__
for efficient re-execution of statements.  Statement caching lets Oracle
Database cursors be used without re-parsing the statement.  Statement caching
also reduces metadata transfer costs between cx_Oracle and the database.
Performance and scalability are improved.

Each standalone or pooled connection has its own cache of statements with a
default size of 20.  The size can be set when creating connection pools or
standalone connections.  The size can subsequently be changed with
:attr:`Connection.stmtcachesize` or :attr:`SessionPool.stmtcachesize`.  In
general, set the statement cache size to the size of the working set of
statements being executed by the application.  To manually tune the cache,
monitor the general application load and the `Automatic Workload Repository
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-56AEF38E-9400-427B-A818-EDEC145F7ACD>`__
(AWR) "bytes sent via SQL*Net to client" values.  The latter statistic should
benefit from not shipping statement metadata to cx_Oracle.  Adjust the statement
cache size to your satisfaction. With Oracle Database 12c, or later, the
statement cache size can be automatically tuned using an :ref:`oraaccess.xml
<optclientfiles>` file.

Statement caching can be disabled by setting the size to 0.  Disabling
the cache may be beneficial when the quantity or order of statements
causes cache entries to be flushed before they get a chance to be
reused.  For example if there are more distinct statements than cache
slots, and the order of statement execution causes older statements to
be flushed from the cache before the statements are re-executed.

With connection pools, the effect of changing :attr:`SessionPool.stmtcachesize`
after pool creation depends on the Oracle Client version:

- When using Oracle Client 21 (or later), changing the cache size does not
  immediately affect connections previously acquired and currently in use. When
  those connections are subsequently released to the pool and re-acquired, they
  will then use the new value. If it is neccessary to change the size on a
  connection because it is not being released to the pool, use
  :data:`Connection.stmtcachesize`.

- When using Oracle Client prior to version 21, changing the pool's statement
  cache size has no effect on connections that already exist in the pool but
  will affect new connections that are subsequently created, for example when
  the pool grows.  To change the size on a connection, use
  :data:`Connection.stmtcachesize`.

When it is inconvenient to pass statement text through an application, the
:meth:`Cursor.prepare()` call can be used to avoid statement re-parsing.
Subsequent ``execute()`` calls use the value ``None`` instead of the SQL text:

.. code-block:: python

    cur.prepare("select * from dept where deptno = :id order by deptno")

    cur.execute(None, id = 20)
    res = cur.fetchall()
    print(res)

    cur.execute(None, id = 10)
    res = cur.fetchall()
    print(res)

Statements passed to :meth:`~Cursor.prepare()` are also stored in the statement
cache.

.. _clientresultcache:

Client Result Caching
=====================

cx_Oracle applications can use Oracle Database's `Client Result Cache
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-35CB2592-7588-4C2D-9075-6F639F25425E>`__.
The CRC enables client-side caching of SQL query (SELECT statement) results in
client memory for immediate use when the same query is re-executed.  This is
useful for reducing the cost of queries for small, mostly static, lookup tables,
such as for postal codes.  CRC reduces network :ref:`round-trips <roundtrips>`,
and also reduces database server CPU usage.

The cache is at the application process level.  Access and invalidation is
managed by the Oracle Client libraries.  This removes the need for extra
application logic, or external utilities, to implement a cache.

CRC can be enabled by setting the `database parameters
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-A9D4A5F5-B939-48FF-80AE-0228E7314C7D>`__
``CLIENT_RESULT_CACHE_SIZE`` and ``CLIENT_RESULT_CACHE_LAG``, and then
restarting the database, for example:

.. code-block:: sql

    SQL> ALTER SYSTEM SET CLIENT_RESULT_CACHE_LAG = 3000 SCOPE=SPFILE;
    SQL> ALTER SYSTEM SET CLIENT_RESULT_CACHE_SIZE = 64K SCOPE=SPFILE;
    SQL> STARTUP FORCE

CRC can alternatively be configured in an :ref:`oraaccess.xml <optclientfiles>`
or :ref:`sqlnet.ora <optnetfiles>` file on the Python host, see `Client
Configuration Parameters
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-E63D75A1-FCAA-4A54-A3D2-B068442CE766>`__.

Tables can then be created, or altered, so repeated queries use CRC.  This
allows existing applications to use CRC without needing modification.  For example:

.. code-block:: sql

    SQL> CREATE TABLE cities (id number, name varchar2(40)) RESULT_CACHE (MODE FORCE);
    SQL> ALTER TABLE locations RESULT_CACHE (MODE FORCE);

Alternatively, hints can be used in SQL statements.  For example:

.. code-block:: sql

    SELECT /*+ result_cache */ postal_code FROM locations
