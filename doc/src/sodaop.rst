.. _sodaop:

*********************
SODA Operation Object
*********************

.. note::

    This object is an extension the DB API. It represents an operation that
    will be performed on all or some of the documents in a SODA collection. It
    is created by the method :meth:`SodaCollection.find()`.

    SODA support in cx_Oracle is in Preview status and should not be used in
    production. It will be supported with a future version of Oracle Client
    libraries.


.. method:: SodaOperation.count()

    Returns a count of the number of documents in the collection that match
    the criteria. If :meth:`~SodaOperation.skip()` or
    :meth:`~SodaOperation.limit()` were called on this object, an exception is
    raised.

    .. versionadded:: 7.0


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

