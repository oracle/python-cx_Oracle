.. _lobdata:

************************
Using CLOB and BLOB Data
************************

Oracle Database uses :ref:`lobobj` to store large data such as text, images,
videos and other multimedia formats.  The maximum size of a LOB is limited to
the size of the tablespace storing it.

There are four types of LOB (large object):

    * BLOB - Binary Large Object, used for storing binary data. cx_Oracle uses
      the type :attr:`cx_Oracle.DB_TYPE_BLOB`.
    * CLOB - Character Large Object, used for string strings in the database
      character set format. cx_Oracle uses the type
      :attr:`cx_Oracle.DB_TYPE_CLOB`.
    * NCLOB - National Character Large Object, used for string strings in the
      national character set format. cx_Oracle uses the type
      :attr:`cx_Oracle.DB_TYPE_NCLOB`.
    * BFILE - External Binary File, used for referencing a file stored on the
      host operating system outside of the database. cx_Oracle uses the type
      :attr:`cx_Oracle.DB_TYPE_BFILE`.

LOBs can be streamed to, and from, Oracle Database.

LOBs up to 1 GB in length can be also be handled directly as strings or bytes in
cx_Oracle.  This makes LOBs easy to work with, and has significant performance
benefits over streaming.  However it requires the entire LOB data to be present
in Python memory, which may not be possible.

See `GitHub <https://github.com/oracle/python-cx_Oracle/tree/main/samples>`__ for LOB examples.


Simple Insertion of LOBs
------------------------

Consider a table with CLOB and BLOB columns:

.. code-block:: sql

    CREATE TABLE lob_tbl (
        id NUMBER,
        c CLOB,
        b BLOB
    );

With cx_Oracle, LOB data can be inserted in the table by binding strings or
bytes as needed:

.. code-block:: python

    with open('example.txt', 'r') as f:
        text_data = f.read()

    with open('image.png', 'rb') as f:
        img_data = f.read()

    cursor.execute("""
            insert into lob_tbl (id, c, b)
            values (:lobid, :clobdata, :blobdata)""",
            lobid=10, clobdata=text_data, blobdata=img_data)

Note that with this approach, LOB data is limited to 1 GB in size.

.. _directlobs:

Fetching LOBs as Strings and Bytes
----------------------------------

CLOBs and BLOBs smaller than 1 GB can queried from the database directly as
strings and bytes.  This can be much faster than streaming.

A :attr:`Connection.outputtypehandler` or :attr:`Cursor.outputtypehandler` needs
to be used as shown in this example:

.. code-block:: python

    def output_type_handler(cursor, name, default_type, size, precision, scale):
        if default_type == cx_Oracle.DB_TYPE_CLOB:
            return cursor.var(cx_Oracle.DB_TYPE_LONG, arraysize=cursor.arraysize)
        if default_type == cx_Oracle.DB_TYPE_BLOB:
            return cursor.var(cx_Oracle.DB_TYPE_LONG_RAW, arraysize=cursor.arraysize)

    id_val = 1
    text_data = "The quick brown fox jumps over the lazy dog"
    binary_data = b"Some binary data"
    cursor.execute("insert into lob_tbl (id, c, b) values (:1, :2, :3)",
                   [id_val, text_data, binary_data])

    connection.outputtypehandler = output_type_handler
    cursor.execute("select c, b from lob_tbl where id = :1", [id_val])
    clob_data, blob_data = cursor.fetchone()
    print("CLOB length:", len(clob_data))
    print("CLOB data:", clob_data)
    print("BLOB length:", len(blob_data))
    print("BLOB data:", blob_data)

This displays::

    CLOB length: 43
    CLOB data: The quick brown fox jumps over the lazy dog
    BLOB length: 16
    BLOB data: b'Some binary data'


Streaming LOBs (Read)
---------------------

Without the output type handler, the CLOB and BLOB values are fetched as
:ref:`LOB objects<lobobj>`. The size of the LOB object can be obtained by
calling :meth:`LOB.size()` and the data can be read by calling
:meth:`LOB.read()`:

.. code-block:: python

    id_val = 1
    text_data = "The quick brown fox jumps over the lazy dog"
    binary_data = b"Some binary data"
    cursor.execute("insert into lob_tbl (id, c, b) values (:1, :2, :3)",
                   [id_val, text_data, binary_data])

    cursor.execute("select b, c from lob_tbl where id = :1", [id_val])
    b, c = cursor.fetchone()
    print("CLOB length:", c.size())
    print("CLOB data:", c.read())
    print("BLOB length:", b.size())
    print("BLOB data:", b.read())

This approach produces the same results as the previous example but it will
perform more slowly because it requires more :ref:`round-trips <roundtrips>` to
Oracle Database and has higher overhead. It is needed, however, if the LOB data
cannot be fetched as one block of data from the server.

To stream the BLOB column, the :meth:`LOB.read()` method can be called
repeatedly until all of the data has been read, as shown below:

.. code-block:: python

    cursor.execute("select b from lob_tbl where id = :1", [10])
    blob, = cursor.fetchone()
    offset = 1
    num_bytes_in_chunk = 65536
    with open("image.png", "wb") as f:
        while True:
            data = blob.read(offset, num_bytes_in_chunk)
            if data:
                f.write(data)
            if len(data) < num_bytes_in_chunk:
                break
            offset += len(data)


Streaming LOBs (Write)
----------------------

If a row containing a LOB is being inserted or updated, and the quantity of
data that is to be inserted or updated cannot fit in a single block of data,
the data can be streamed using the method :meth:`LOB.write()` instead as shown
in the following code:

.. code-block:: python

    id_val = 9
    lob_var = cursor.var(cx_Oracle.DB_TYPE_BLOB)
    cursor.execute("""
            insert into lob_tbl (id, b)
            values (:1, empty_blob())
            returning b into :2""", [id_val, lob_var])
    blob, = lobVar.getvalue()
    offset = 1
    num_bytes_in_chunk = 65536
    with open("image.png", "rb") as f:
        while True:
            data = f.read(num_bytes_in_chunk)
            if data:
                blob.write(data, offset)
            if len(data) < num_bytes_in_chunk:
                break
            offset += len(data)
    connection.commit()


Temporary LOBs
--------------

All of the examples shown thus far have made use of permanent LOBs. These are
LOBs that are stored in the database. Oracle also supports temporary LOBs that
are not stored in the database but can be used to pass large quantities of
data. These LOBs use space in the temporary tablespace until all variables
referencing them go out of scope or the connection in which they are created is
explicitly closed.

When calling PL/SQL procedures with data that exceeds 32,767 bytes in length,
cx_Oracle automatically creates a temporary LOB internally and passes that
value through to the procedure. If the data that is to be passed to the
procedure exceeds that which can fit in a single block of data, however, you
can use the method :meth:`Connection.createlob()` to create a temporary LOB.
This LOB can then be read and written just like in the examples shown above for
persistent LOBs.
