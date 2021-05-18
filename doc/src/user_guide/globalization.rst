.. _globalization:

********************************
Character Sets and Globalization
********************************

Data fetched from, and sent to, Oracle Database will be mapped between the
database character set and the "Oracle client" character set of the Oracle
Client libraries used by cx_Oracle.  If data cannot be correctly mapped between
client and server character sets, then it may be corrupted or queries may fail
with :ref:`"codec can't decode byte" <codecerror>`.

cx_Oracle uses Oracle’s National Language Support (NLS) to assist in
globalizing applications.  As well as character set support, there are many
other features that will be useful in applications.  See the
`Database Globalization Support Guide
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=NLSPG>`__.


Setting the Client Character Set
================================

In cx_Oracle 8 the default encoding used for all character data changed to
"UTF-8". This universal encoding is suitable for most applications.  If you
have a special need, you can pass the ``encoding`` and ``nencoding`` parameters
to the :meth:`cx_Oracle.connect` and :meth:`cx_Oracle.SessionPool` methods to
specify different Oracle Client character sets. For example:

.. code-block:: python

    import cx_Oracle
    connection = cx_Oracle.connect(dsn=connect_string, encoding="US-ASCII",
                                   nencoding="UTF-8")

.. note::

    In a future release of cx_Oracle, only UTF-8 will be supported.

The ``encoding`` parameter affects character data such as VARCHAR2 and CLOB
columns.  The ``nencoding`` parameter affects "National Character" data such as
NVARCHAR2 and NCLOB.  If you are not using national character types, then you
can omit ``nencoding``. Both the ``encoding`` and ``nencoding`` parameters are
expected to be one of the `Python standard encodings
<https://docs.python.org/3/library/codecs.html#standard-encodings>`__ such as
``UTF-8``. Do not accidentally use ``UTF8``, which Oracle uses to specify the
older Unicode 3.0 Universal character set, ``CESU-8``. Note that Oracle does
not recognize all of the encodings that Python recognizes. You can see which
encodings are usable in cx_Oracle by issuing this query:

.. code-block:: sql

    select distinct utl_i18n.map_charset(value)
    from v$nls_valid_values
    where parameter = 'CHARACTERSET'
      and utl_i18n.map_charset(value) is not null
    order by 1

.. note::

    From cx_Oracle 8, it is no longer possible to change the character set
    using the ``NLS_LANG`` environment variable.  The character set component
    of that variable is ignored.  The language and territory components of
    ``NLS_LANG`` are still respected by the Oracle Client libraries.

Character Set Example
---------------------

The script below tries to display data containing a Euro symbol from the
database.

.. code-block:: python

    connection = cx_Oracle.connect(user=user, password=password,
                                   dsn="dbhost.example.com/orclpdb1",
                                   encoding="US-ASCII")
    cursor = connection.cursor()
    for row in cursor.execute("select nvarchar2_column from nchar_test"):
        print(row)

Because the '€' symbol is not supported by the ``US-ASCII`` character set, all
'€' characters are replaced by '¿' in the cx_Oracle output::

    ('¿',)

When the ``encoding`` parameter is removed (or set to "UTF-8") during
connection:

.. code-block:: python

    connection = cx_Oracle.connect(user=user, password=password,
                                   dsn="dbhost.example.com/orclpdb1")

Then the output displays the Euro symbol as desired::

    ('€',)

.. _findingcharset:

Finding the Database and Client Character Set
---------------------------------------------

To find the database character set, execute the query:

.. code-block:: sql

    SELECT value AS db_charset
    FROM nls_database_parameters
    WHERE parameter = 'NLS_CHARACTERSET';

To find the database 'national character set' used for NCHAR and related types,
execute the query:

.. code-block:: sql

     SELECT value AS db_ncharset
     FROM nls_database_parameters
     WHERE parameter = 'NLS_NCHAR_CHARACTERSET';

To find the current "client" character set used by cx_Oracle, execute the
query:

.. code-block:: sql

    SELECT DISTINCT client_charset AS client_charset
    FROM v$session_connect_info
    WHERE sid = SYS_CONTEXT('USERENV', 'SID');

If these character sets do not match, characters transferred over Oracle Net
will be mapped from one character set to another.  This may impact performance
and may result in invalid data.

Setting the Oracle Client Locale
================================

You can use the ``NLS_LANG`` environment variable to set the language and
territory used by the Oracle Client libraries.  For example, on Linux you could
set::

    export NLS_LANG=JAPANESE_JAPAN

The language ("JAPANESE" in this example) specifies conventions such as the
language used for Oracle Database messages, sorting, day names, and month
names.  The territory ("JAPAN") specifies conventions such as the default date,
monetary, and numeric formats. If the language is not specified, then the value
defaults to AMERICAN.  If the territory is not specified, then the value is
derived from the language value.  See `Choosing a Locale with the NLS_LANG
Environment Variable
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-86A29834-AE29-4BA5-8A78-E19C168B690A>`__

If the ``NLS_LANG`` environment variable is set in the application with
``os.environ['NLS_LANG']``, it must be set before any connection pool is
created, or before any standalone connections are created.

Other Oracle globalization variables, such as ``NLS_DATE_FORMAT`` can also be
set to change the behavior of cx_Oracle, see `Setting NLS Parameters
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-6475CA50-6476-4559-AD87-35D431276B20>`__.
