.. _sodausermanual:

************************************
Simple Oracle Document Access (SODA)
************************************

Overview
========

Oracle Database Simple Oracle Document Access (SODA) allows documents to be
inserted, queried, and retrieved from Oracle Database using a set of
NoSQL-style cx_Oracle methods. Documents are generally JSON data but they can
be any data at all (including video, images, sounds, or other binary content).
Documents can be fetched from the database by key lookup or by using
query-by-example (QBE) pattern-matching.

SODA uses a SQL schema to store documents but you do not need to know SQL or
how the documents are stored. However, access via SQL does allow use of
advanced Oracle Database functionality such as analytics for reporting.

Oracle SODA implementations are also available in `Node.js
<https://oracle.github.io/node-oracledb/doc/api.html#sodaoverview>`__, `Java
<https://docs.oracle.com/en/database/oracle/simple-oracle-document-access/java/adsda/index.html>`__,
`PL/SQL <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=ADSDP>`__,
`Oracle Call Interface
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-23206C89-891E-43D7-827C-5C6367AD62FD>`__
and via `REST
<https://docs.oracle.com/en/database/oracle/simple-oracle-document-access/rest/index.html>`__.

For general information on SODA, see the `SODA home page
<https://docs.oracle.com/en/database/oracle/simple-oracle-document-access/index.html>`__
and the Oracle Database `Introduction to Simple Oracle Document Access (SODA)
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=ADSDI>`__ manual.

For specific requirements see the cx_Oracle :ref:`SODA requirements <sodarequirements>`.

cx_Oracle uses the following objects for SODA:

* :ref:`SODA Database Object <sodadb>`: The top level object for cx_Oracle SODA
  operations. This is acquired from an Oracle Database connection.  A 'SODA
  database' is an abstraction, allowing access to SODA collections in that
  'SODA database', which then allow access to documents in those collections.
  A SODA database is analogous to an Oracle Database user or schema, a
  collection is analogous to a table, and a document is analogous to a table
  row with one column for a unique document key, a column for the document
  content, and other columns for various document attributes.

* :ref:`SODA Collection Object <sodacoll>`: Represents a collection of SODA
  documents.  By default, collections allow JSON documents to be stored.  This
  is recommended for most SODA users.  However optional metadata can set
  various details about a collection, such as its database storage, whether it
  should track version and time stamp document components, how such components
  are generated, and what document types are supported. By default, the name of
  the Oracle Database table storing a collection is the same as the collection
  name. Note: do not use SQL to drop the database table, since SODA metadata
  will not be correctly removed.  Use the :meth:`SodaCollection.drop()` method
  instead.

* :ref:`SODA Document Object <sodadoc>`: Represents a document.  Typically the
  document content will be JSON.  The document has properties including the
  content, a key, timestamps, and the media type.  By default, document keys
  are automatically generated.  See :ref:`SODA Document objects <sodadoc>` for
  the forms of SodaDoc.

* :ref:`SODA Document Cursor <sodadoccur>`: A cursor object representing the
  result of the :meth:`SodaOperation.getCursor()` method from a
  :meth:`SodaCollection.find()` operation.  It can be iterated over to access
  each SodaDoc.

* :ref:`SODA Operation Object <sodaop>`: An internal object used with
  :meth:`SodaCollection.find()` to perform read and write operations on
  documents.  Chained methods set properties on a SodaOperation object which is
  then used by a terminal method to find, count, replace, or remove documents.
  This is an internal object that should not be directly accessed.

.. _sodametadatacache:

Using the SODA Metadata Cache
=============================

SODA metadata can be cached to improve the performance of
:meth:`SodaDatabase.createCollection()` and
:meth:`SodaDatabase.openCollection()` by reducing :ref:`round-trips
<roundtrips>` to the database. Caching is available with Oracle Client 21.3 (or
later). The feature is also available in Oracle Client 19 from 19.11 onwards.
Note: if collection metadata changes are made externally, the cache can become
invalid.

Caching can be enabled for pooled connections but not standalone connections.
Each pool has its own cache.  Applications using standalone connections should
retain and reuse the :ref:`collection <sodacoll>` returned from
``createCollection()`` or ``openCollection()`` wherever possible, instead of
making repeated calls to those methods.

The metadata cache can be turned on when creating a connection pool
with :meth:`cx_Oracle.SessionPool()`:

.. code-block:: python

    # Create the session pool
    pool = cx_Oracle.SessionPool("hr", userpwd, "dbhost.example.com/orclpdb1",
                                 soda_metadata_cache=True)

Note the cache is not used by ``createCollection()`` when explicitly passing
metadata.  In this case, instead of using only ``createCollection()`` and
relying on its behavior of opening an existing collection like:

.. code-block:: python

    mymetadata = { . . . }
    collection = soda.createCollection("mycollection", mymetadata) # open existing or create new collection
    collection.insertOne(mycontent)

you may find it more efficient to use logic similar to:

.. code-block:: python

    collection = soda.openCollection("mycollection")
    if (collection is None):
        mymetadata = { . . . }
        collection = soda.createCollection("mycollection", mymetadata)
    collection.insertOne(mycontent)

SODA Examples
=============

Creating and adding documents to a collection can be done as follows:

.. code-block:: python

    soda = connection.getSodaDatabase()

    # create a new SODA collection; this will open an existing collection, if
    # the name is already in use
    collection = soda.createCollection("mycollection")

    # insert a document into the collection; for the common case of a JSON
    # document, the content can be a simple Python dictionary which will
    # internally be converted to a JSON document
    content = {'name': 'Matilda', 'address': {'city': 'Melbourne'}}
    returnedDoc = collection.insertOneAndGet(content)
    key = returnedDoc.key
    print('The key of the new SODA document is: ', key)

By default, a system generated key is created when documents are inserted.
With a known key, you can retrieve a document:

.. code-block:: python

    # this will return a dictionary (as was inserted in the previous code)
    content = collection.find().key(key).getOne().getContent()
    print(content)

You can also search for documents using query-by-example syntax:

.. code-block:: python

    # Find all documents with names like 'Ma%'
    print("Names matching 'Ma%'")
    qbe = {'name': {'$like': 'Ma%'}}
    for doc in collection.find().filter(qbe).getDocuments():
        content = doc.getContent()
        print(content["name"])

See the `samples directory
<https://github.com/oracle/python-cx_Oracle/tree/master/samples>`__
for runnable SODA examples.

--------------------
Committing SODA Work
--------------------

The general recommendation for SODA applications is to turn on
:attr:`~Connection.autocommit` globally:

.. code-block:: python

    connection.autocommit = True

If your SODA document write operations are mostly independent of each other,
this removes the overhead of application transaction management and the need for
explicit :meth:`Connection.commit()` calls.

When deciding how to commit transactions, beware of transactional consistency
and performance requirements.  If you are using individual SODA calls to insert
or update a large number of documents with individual calls, you should turn
:attr:`~Connection.autocommit` off and issue a single, explicit
:meth:`~Connection.commit()` after all documents have been processed.  Also
consider using :meth:`SodaCollection.insertMany()` or
:meth:`SodaCollection.insertManyAndGet()` which have performance benefits.

If you are not autocommitting, and one of the SODA operations in your
transaction fails, then previous uncommitted operations will not be rolled back.
Your application should explicitly roll back the transaction with
:meth:`Connection.rollback()` to prevent any later commits from committing a
partial transaction.

Note:

- SODA DDL operations do not commit an open transaction the way that SQL always does for DDL statements.
- When :attr:`~Connection.autocommit` is ``True``, most SODA methods will issue a commit before successful return.
- SODA provides optimistic locking, see :meth:`SodaOperation.version()`.
- When mixing SODA and relational access, any commit or rollback on the connection will affect all work.
