.. _globalization:

***************************************************
Characters Sets and National Language Support (NLS)
***************************************************

Data fetched from, and sent to, Oracle Database will be mapped between the
database character set and the "Oracle client" character set of the Oracle
client libraries used by cx_Oracle.  If data cannot be correctly mapped between
client and server character sets, then it may be corrupted or queries may fail
with :ref:`"codec can't decode byte" <codecerror>`.  Most applications will need
to specify the client character set.

cx_Oracle uses Oracle’s National Language Support (NLS) to assist in
globalizing applications.  As well as character set support, there are many
other features that will be useful in applications.  See the
`Database Globalization Support Guide
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=NLSPG>`__.


Setting the Client Character Set
================================

You can specify the Oracle client character set used by cx_Oracle by passing
the ``encoding`` and ``nencoding`` parameters to the :meth:`cx_Oracle.connect`
and :meth:`cx_Oracle.SessionPool` methods. For example:

.. code-block:: python

    import cx_Oracle
    connection = cx_Oracle.connect(connectString, encoding="UTF-8",
            nencoding="UTF-8")

The ``encoding`` parameter affects character data such as VARCHAR2 and CLOB
columns.  The ``nencoding`` parameter affects "National Character" data such as
NVARCHAR2 and NCLOB.  If you are not using national character types, then you
can omit ``nencoding``. Both the ``encoding`` and ``nencoding`` parameters are
expected to be one of the `Python standard encodings
<https://docs.python.org/3/library/codecs.html#standard-encodings>`__ such as
``UTF-8``. Do not accidentally use ``UTF8``, which Oracle uses to specify the
older Unicode 3.0 Universal character set, ``CESU-8``. Note that Oracle does
not recognize all of the encodings that Python recognizes. You can see which
encodings Oracle supports by issuing this query:

.. code-block:: sql

    select distinct utl_i18n.map_charset(value)
    from v$nls_valid_values
    where parameter = 'CHARACTERSET'
      and utl_i18n.map_charset(value) is not null
    order by 1

An alternative to setting the encoding parameters is to set Oracle's
``NLS_LANG`` environment variable to a value such as
``AMERICAN_AMERICA.AL32UTF8``. See :ref:`Setting environment variables
<envset>`.  As well as setting the character set, the ``NLS_LANG`` environment
variable lets you specify the Language (``AMERICAN`` in this example) and
Territory (``AMERICA``) used for NLS globalization.  See
`Choosing a Locale with the NLS_LANG Environment Variables
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-86A29834-AE29-4BA5-8A78-E19C168B690A>`__.

A character set specified by an ``encoding`` parameter will override the
character set in ``NLS_LANG``.  The language and territory components will still
be used by Oracle.

If the ``NLS_LANG`` environment variable is set in the application with
``os.environ['NLS_LANG']``, it must be set before any connection pool is
created, or before any standalone connections are created.

If neither of the encoding parameters are specified and the ``NLS_LANG``
environment variable is not set, the character set ``ASCII`` is used.

Other Oracle globalization variable can also be set, see
`Setting NLS Parameters
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-6475CA50-6476-4559-AD87-35D431276B20>`__.


Character Set Example
=====================

The script below tries to display data containing a Euro symbol from the
database.  The ``NLS_LANG`` environment variable on the operating system is set
to ``AMERICAN_AMERICA.WE8ISO8859P1``:

.. code-block:: python

    connection = cx_Oracle.connect(userName, password, "dbhost.example.com/orclpdb1")
    cursor = connection.cursor()
    for row in cursor.execute("select * from nchar_test"):
        print(row)
    print(connection.encoding)
    print(connection.nencoding)

Because the '€' symbol is not supported by the ``WE8ISO8859P1`` character set,
all '€' characters are replaced by '¿' in the cx_Oracle output::

    ('¿', 'test      ', 'test', 'test      ')
    ('¿', 'test      ', '¿', 'test      ')
    ('"nvarchar"', '/"nchar"  ', 'varchardata', 'chardata  ')
    ('°', 'Second    ', 'Third', 'Fourth    ')
    ISO-8859-1
    ISO-8859-1

When the ``encoding`` parameter is set during connection:

.. code-block:: python

    connection = cx_Oracle.connect(userName, password, "dbhost.example.com/orclpdb1",
            encoding="UTF-8", nencoding="UTF-8")

Then the output displays the Euro symbol as desired::

    ('€', 'test      ', 'test', 'test      ')
    ('€', 'test      ', '€', 'test      ')
    ('"nvarchar"', '/"nchar"  ', 'varchardata', 'chardata  ')
    ('°', 'Second    ', 'Third', 'Fourth    ')
    UTF-8
    UTF-8


.. _findingcharset:

Finding the Database and Client Character Set
=============================================

To find the database character set, execute the query:

.. code-block:: sql

    SELECT value AS db_charset
    FROM nls_database_parameters
    WHERE parameter = 'NLS_CHARACTERSET';

To find the current "client" character set used by cx_Oracle, execute the
query:

.. code-block:: sql

    SELECT DISTINCT client_charset AS client_charset
    FROM v$session_connect_info
    WHERE sid = SYS_CONTEXT('USERENV', 'SID');

If these character sets do not match, characters will be mapped when
transferred over Oracle Net.  This may impact performance and may result in
invalid data.
