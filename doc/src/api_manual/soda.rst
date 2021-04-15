.. _soda:

****
SODA
****

`Oracle Database Simple Oracle Document Access (SODA)
<https://docs.oracle.com/en/database/oracle/simple-oracle-document-access>`__
allows documents to be inserted, queried, and retrieved from Oracle Database
using a set of NoSQL-style cx_Oracle methods. By default, documents are JSON
strings. See the :ref:`user manual <sodausermanual>` for examples.

.. _sodarequirements:

-----------------
SODA Requirements
-----------------

To use SODA, the role SODA_APP must be granted to the user.  To create
collections, users need the CREATE TABLE privilege.  These can be granted by a
DBA:

.. code-block:: sql

    SQL> grant soda_app, create table to myuser;

Advanced users who are using Oracle sequences for keys will also need the CREATE
SEQUENCE privilege.

SODA requires Oracle Client 18.3 or higher and Oracle Database 18.1 and higher.

.. note::

    If you are using Oracle Database 21c (or later) and create new collections
    you need to do one of the following:

    - Use Oracle Client libraries 21c (or later).

    - Or, explicitly use collection metadata when creating collections and set
      the data storage type to BLOB, for example::

        {
            "keyColumn": {
                "name":"ID"
            },
            "contentColumn": {
                "name": "JSON_DOCUMENT",
                "sqlType": "BLOB"
            },
            "versionColumn": {
                "name": "VERSION",
                "method": "UUID"
            },
            "lastModifiedColumn": {
                "name": "LAST_MODIFIED"
            },
            "creationTimeColumn": {
                "name": "CREATED_ON"
            }
        }

    - Or, set the database initialization parameter `compatible
      <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
      id=GUID-A2E90F08-BC9F-4688-A9D0-4A948DD3F7A9>`__ to 19 or lower.

    Otherwise you may get errors such as "ORA-40842: unsupported value JSON in
    the metadata for the field sqlType" or "ORA-40659: Data type does not match
    the specification in the collection metadata".

.. _sodadb:

--------------------
SODA Database Object
--------------------

.. note::

    This object is an extension the DB API. It is returned by the method
    :meth:`Connection.getSodaDatabase()`.


.. method:: SodaDatabase.createCollection(name, metadata=None, mapMode=False)

    Creates a SODA collection with the given name and returns a new
    :ref:`SODA collection object <sodacoll>`. If you try to create a
    collection, and a collection with the same name and metadata already
    exists, then that existing collection is opened without error.

    If metadata is specified, it is expected to be a string containing valid
    JSON or a dictionary that will be transformed into a JSON string. This JSON
    permits you to specify the configuration of the collection including
    storage options; specifying the presence or absence of columns for creation
    timestamp, last modified timestamp and version; whether the collection can
    store only JSON documents; and methods of key and version generation. The
    default metadata creates a collection that only supports JSON documents and
    uses system generated keys. See this `collection metadata reference
    <https://www.oracle.com/pls/topic/
    lookup?ctx=dblatest&id=GUID-49EFF3D3-9FAB-4DA6-BDE2-2650383566A3>`__
    for more information.

    If the mapMode parameter is set to True, the new collection is mapped to an
    existing table instead of creating a table. If a collection is created in
    this way, dropping the collection will not drop the existing table either.

    .. versionadded:: 7.0


.. method:: SodaDatabase.createDocument(content, key=None, mediaType="application/json")

    Creates a :ref:`SODA document <sodadoc>` usable for SODA write operations.
    You only need to use this method if your collection requires
    client-assigned keys or has non-JSON content; otherwise, you can pass your
    content directly to SODA write operations. SodaDocument attributes
    'createdOn', 'lastModified' and 'version' will be None.

    The content parameter can be a dictionary or list which will be transformed
    into a JSON string and then UTF-8 encoded. It can also be a string which
    will be UTF-8 encoded or it can be a bytes object which will be stored
    unchanged. If a bytes object is provided and the content is expected to be
    JSON, note that SODA only supports UTF-8, UTF-16LE and UTF-16BE encodings.

    The key parameter should only be supplied if the collection in which the
    document is to be placed requires client-assigned keys.

    The mediaType parameter should only be supplied if the collection in which
    the document is to be placed supports non-JSON documents and the content
    for this document is non-JSON. Using a standard MIME type for this value is
    recommended but any string will be accepted.

    .. versionadded:: 7.0


.. method:: SodaDatabase.getCollectionNames(startName=None, limit=0)

    Returns a list of the names of collections in the database that match the
    criteria, in alphabetical order.

    If the startName parameter is specified, the list of names returned will
    start with this value and also contain any names that fall after this value
    in alphabetical order.

    If the limit parameter is specified and is non-zero, the number of
    collection names returned will be limited to this value.

    .. versionadded:: 7.0


.. method:: SodaDatabase.openCollection(name)

    Opens an existing collection with the given name and returns a new
    :ref:`SODA collection object <sodacoll>`. If a collection with that name
    does not exist, None is returned.

    .. versionadded:: 7.0


.. _sodacoll:

----------------------
SODA Collection Object
----------------------

.. note::

    This object is an extension the DB API. It is used to represent SODA
    collections and is created by methods
    :meth:`SodaDatabase.createCollection()` and
    :meth:`SodaDatabase.openCollection()`.


.. method:: SodaCollection.createIndex(spec)

    Creates an index on a SODA collection. The spec is expected to be a
    dictionary or a JSON-encoded string. See this `overview
    <https://www.oracle.com/pls/topic/
    lookup?ctx=dblatest&id=GUID-4848E6A0-58A7-44FD-8D6D-A033D0CCF9CB>`__
    for information on indexes in SODA.

    Note that a commit should be performed before attempting to create an
    index.

    .. versionadded:: 7.0


.. method:: SodaCollection.drop()

    Drops the collection from the database, if it exists. Note that if the
    collection was created with mapMode set to True the underlying table will
    not be dropped.

    A boolean value is returned indicating if the collection was actually
    dropped.

    .. versionadded:: 7.0


.. method:: SodaCollection.dropIndex(name, force=False)

    Drops the index with the specified name, if it exists.

    The force parameter, if set to True, can be used to force the dropping of
    an index that the underlying Oracle Database domain index doesn't normally
    permit. This is only applicable to spatial and JSON search indexes.
    See `here <https://www.oracle.com/pls/topic/
    lookup?ctx=dblatest&id=GUID-F60F75DF-2866-4F93-BB7F-8FCE64BF67B6>`__
    for more information.

    A boolean value is returned indicating if the index was actually dropped.

    .. versionadded:: 7.0


.. method:: SodaCollection.find()

    This method is used to begin an operation that will act upon documents in
    the collection. It creates and returns a
    :ref:`SodaOperation object <sodaop>` which is used to specify the criteria
    and the operation that will be performed on the documents that match that
    criteria.

    .. versionadded:: 7.0


.. method:: SodaCollection.getDataGuide()

    Returns a :ref:`SODA document object <sodadoc>` containing property names,
    data types and lengths inferred from the JSON documents in the collection.
    It can be useful for exploring the schema of a collection. Note that this
    method is only supported for JSON-only collections where a JSON search
    index has been created with the 'dataguide' option enabled. If there are
    no documents in the collection, None is returned.

    .. versionadded:: 7.0


.. method:: SodaCollection.insertMany(docs)

    Inserts a list of documents into the collection at one time. Each of the
    input documents can be a dictionary or list or an existing :ref:`SODA
    document object <sodadoc>`.

    .. note::

        This method requires Oracle Client 18.5 and higher and is available
        only as a preview.

    .. versionadded:: 7.2


.. method:: SodaCollection.insertManyAndGet(docs, hint=None)

    Similarly to :meth:`~SodaCollection.insertMany()` this method inserts a
    list of documents into the collection at one time. The only difference is
    that it returns a list of :ref:`SODA Document objects <sodadoc>`. Note that
    for performance reasons the returned documents do not contain the content.

    The hint parameter, if specified, supplies a hint to the database when
    processing the SODA operation. This is expected to be a string in the same
    format as SQL hints but without the enclosing comment characters. Use of
    this parameter requires Oracle Client 21.3 or higher (or Oracle Client 19
    from 19.11).

    .. note::

        This method requires Oracle Client 18.5 and higher.

    .. versionadded:: 7.2

    .. versionchanged:: 8.2

        The parameter `hint` was added.


.. method:: SodaCollection.insertOne(doc)

    Inserts a given document into the collection. The input document can be a
    dictionary or list or an existing :ref:`SODA document object <sodadoc>`.

    .. versionadded:: 7.0


.. method:: SodaCollection.insertOneAndGet(doc, hint=None)

    Similarly to :meth:`~SodaCollection.insertOne()` this method inserts a
    given document into the collection. The only difference is that it
    returns a :ref:`SODA Document object <sodadoc>`. Note that for performance
    reasons the returned document does not contain the content.

    The hint parameter, if specified, supplies a hint to the database when
    processing the SODA operation. This is expected to be a string in the same
    format as SQL hints but without the enclosing comment characters. Use of
    this parameter requires Oracle Client 21.3 or higher (or Oracle Client 19
    from 19.11).

    .. versionadded:: 7.0

    .. versionchanged:: 8.2

        The parameter `hint` was added.


.. attribute:: SodaCollection.metadata

    This read-only attribute returns a dictionary containing the metadata that
    was used to create the collection. See this `collection metadata reference
    <https://www.oracle.com/pls/topic/
    lookup?ctx=dblatest&id=GUID-49EFF3D3-9FAB-4DA6-BDE2-2650383566A3>`__
    for more information.

    .. versionadded:: 7.0


.. attribute:: SodaCollection.name

    This read-only attribute returns the name of the collection.

    .. versionadded:: 7.0


.. method:: SodaCollection.save(doc)

    Saves a document into the collection. This method is equivalent to
    :meth:`~SodaCollection.insertOne()` except that if client-assigned keys are
    used, and the document with the specified key already exists in the
    collection, it will be replaced with the input document.

    This method requires Oracle Client 19.9 or higher in addition to the usual
    SODA requirements.

    .. versionadded:: 8.0


.. method:: SodaCollection.saveAndGet(doc, hint=None)

    Saves a document into the collection. This method is equivalent to
    :meth:`~SodaCollection.insertOneAndGet()` except that if client-assigned
    keys are used, and the document with the specified key already exists in
    the collection, it will be replaced with the input document.

    The hint parameter, if specified, supplies a hint to the database when
    processing the SODA operation. This is expected to be a string in the same
    format as SQL hints but without the enclosing comment characters. Use of
    this parameter requires Oracle Client 21.3 or higher (or Oracle Client 19
    from 19.11).

    This method requires Oracle Client 19.9 or higher in addition to the usual
    SODA requirements.

    .. versionadded:: 8.0

    .. versionchanged:: 8.2

        The parameter `hint` was added.


.. method:: SodaCollection.truncate()

    Removes all of the documents in the collection, similarly to what is done
    for rows in a table by the TRUNCATE TABLE statement.

    .. versionadded:: 8.0


.. _sodadoc:

--------------------
SODA Document Object
--------------------

.. note::

    This object is an extension the DB API. It is returned by the methods
    :meth:`SodaDatabase.createDocument()`,
    :meth:`SodaOperation.getDocuments()` and
    :meth:`SodaOperation.getOne()` as well as by iterating over
    :ref:`SODA document cursors <sodadoccur>`.


.. attribute:: SodaDoc.createdOn

    This read-only attribute returns the creation time of the document in
    `ISO 8601 <https://www.iso.org/iso-8601-date-and-time-format.html>`__
    format. Documents created by :meth:`SodaDatabase.createDocument()` or
    fetched from collections where this attribute is not stored will return
    None.

    .. versionadded:: 7.0


.. method:: SodaDoc.getContent()

    Returns the content of the document as a dictionary or list. This method
    assumes that the content is application/json and will raise an exception if
    this is not the case. If there is no content, however, None will be
    returned.

    .. versionadded:: 7.0


.. method:: SodaDoc.getContentAsBytes()

    Returns the content of the document as a bytes object. If there is no
    content, however, None will be returned.

    .. versionadded:: 7.0


.. method:: SodaDoc.getContentAsString()

    Returns the content of the document as a string. If the document encoding
    is not known, UTF-8 will be used. If there is no content, however, None
    will be returned.

    .. versionadded:: 7.0


.. attribute:: SodaDoc.key

    This read-only attribute returns the unique key assigned to this document.
    Documents created by :meth:`SodaDatabase.createDocument()` may not have a
    value assigned to them and return None.

    .. versionadded:: 7.0


.. attribute:: SodaDoc.lastModified

    This read-only attribute returns the last modified time of the document in
    `ISO 8601 <https://www.iso.org/iso-8601-date-and-time-format.html>`__
    format. Documents created by :meth:`SodaDatabase.createDocument()` or
    fetched from collections where this attribute is not stored will return
    None.

    .. versionadded:: 7.0


.. attribute:: SodaDoc.mediaType

    This read-only attribute returns the media type assigned to the document.
    By convention this is expected to be a MIME type but no checks are
    performed on this value. If a value is not specified when calling
    :meth:`SodaDatabase.createDocument()` or the document is fetched from a
    collection where this component is not stored, the string
    "application/json" is returned.

    .. versionadded:: 7.0


.. attribute:: SodaDoc.version

    This read-only attribute returns the version assigned to this document.
    Documents created by :meth:`SodaDatabase.createDocument()` or fetched
    from collections where this attribute is not stored will return None.

    .. versionadded:: 7.0


.. _sodadoccur:

---------------------------
SODA Document Cursor Object
---------------------------

.. note::

    This object is an extension the DB API. It is returned by the method
    :meth:`SodaOperation.getCursor()` and implements the iterator protocol.
    Each iteration will return a :ref:`SODA document object <sodadoc>`.


.. method:: SodaDocCursor.close()

    Close the cursor now, rather than whenever __del__ is called. The cursor
    will be unusable from this point forward; an Error exception will be raised
    if any operation is attempted with the cursor.

    .. versionadded:: 7.0


.. _sodaop:

---------------------
SODA Operation Object
---------------------

.. note::

    This object is an extension to the DB API. It represents an operation that
    will be performed on all or some of the documents in a SODA collection. It
    is created by the method :meth:`SodaCollection.find()`.


.. method:: SodaOperation.count()

    Returns a count of the number of documents in the collection that match
    the criteria. If :meth:`~SodaOperation.skip()` or
    :meth:`~SodaOperation.limit()` were called on this object, an exception is
    raised.

    .. versionadded:: 7.0


.. method:: SodaOperation.fetchArraySize(value)

    This is a tuning method to specify the number of documents that are
    internally fetched in batches by calls to :meth:`~SodaOperation.getCursor()`
    and :meth:`~SodaOperation.getDocuments()`. It does not affect how many
    documents are returned to the application. A value of 0 will use the default
    value (100). This method is only available in Oracle Client 19.5 and higher.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 8.0


.. method:: SodaOperation.filter(value)

    Sets a filter specification for complex document queries and ordering of
    JSON documents. Filter specifications must be provided as a dictionary or
    JSON-encoded string and can include comparisons, regular expressions,
    logical and spatial operators, among others. See the
    `overview of SODA filter specifications
    <https://www.oracle.com/pls/topic/
    lookup?ctx=dblatest&id=GUID-CB09C4E3-BBB1-40DC-88A8-8417821B0FBE>`__
    for more information.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 7.0


.. method:: SodaOperation.getCursor()

    Returns a :ref:`SODA Document Cursor object <sodadoccur>` that can be used
    to iterate over the documents that match the criteria.

    .. versionadded:: 7.0


.. method:: SodaOperation.getDocuments()

    Returns a list of :ref:`SODA Document objects <sodadoc>` that match the
    criteria.

    .. versionadded:: 7.0


.. method:: SodaOperation.getOne()

    Returns a single :ref:`SODA Document object <sodadoc>` that matches the
    criteria. Note that if multiple documents match the criteria only the first
    one is returned.

    .. versionadded:: 7.0


.. method:: SodaOperation.hint(value)

    Specifies a hint that will be provided to the SODA operation when it is
    performed. This is expected to be a string in the same format as SQL hints
    but without the enclosing comment characters. Use of this method
    requires Oracle Client 21.3 or higher (or Oracle Client 19 from 19.11).

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 8.2


.. method:: SodaOperation.key(value)

    Specifies that the document with the specified key should be returned.
    This causes any previous calls made to this method and
    :meth:`~SodaOperation.keys()` to be ignored.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 7.0


.. method:: SodaOperation.keys(seq)

    Specifies that documents that match the keys found in the supplied sequence
    should be returned. This causes any previous calls made to this method and
    :meth:`~SodaOperation.key()` to be ignored.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 7.0


.. method:: SodaOperation.limit(value)

    Specifies that only the specified number of documents should be returned.
    This method is only usable for read operations such as
    :meth:`~SodaOperation.getCursor()` and
    :meth:`~SodaOperation.getDocuments()`. For write operations, any value set
    using this method is ignored.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 7.0


.. method:: SodaOperation.remove()

    Removes all of the documents in the collection that match the criteria. The
    number of documents that have been removed is returned.

    .. versionadded:: 7.0


.. method:: SodaOperation.replaceOne(doc)

    Replaces a single document in the collection with the specified document.
    The input document can be a dictionary or list or an existing
    :ref:`SODA document object <sodadoc>`. A boolean indicating if a document
    was replaced or not is returned.

    Currently the method :meth:`~SodaOperation.key()` must be called before
    this method can be called.

    .. versionadded:: 7.0


.. method:: SodaOperation.replaceOneAndGet(doc)

    Similarly to :meth:`~SodaOperation.replaceOne()`, this method replaces a
    single document in the collection with the specified document. The only
    difference is that it returns a :ref:`SODA document object <sodadoc>`.
    Note that for performance reasons the returned document does not contain
    the content.

    .. versionadded:: 7.0


.. method:: SodaOperation.skip(value)

    Specifies the number of documents that match the other criteria that will
    be skipped. This method is only usable for read operations such as
    :meth:`~SodaOperation.getCursor()` and
    :meth:`~SodaOperation.getDocuments()`. For write operations, any value set
    using this method is ignored.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 7.0


.. method:: SodaOperation.version(value)

    Specifies that documents with the specified version should be returned.
    Typically this is used with :meth:`~SodaOperation.key()` to implement
    optimistic locking, so that the write operation called later does not
    affect a document that someone else has modified.

    As a convenience, the SodaOperation object is returned so that further
    criteria can be specified by chaining methods together.

    .. versionadded:: 7.0
