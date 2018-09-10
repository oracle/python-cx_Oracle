.. _sodadb:

********************
SODA Database Object
********************

.. note::

    This object is an extension the DB API. It is returned by the method
    :meth:`Connection.getSodaDatabase()`.

    SODA support in cx_Oracle is in Preview status and should not be used in
    production. It will be supported with a future version of Oracle Client
    libraries.


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

    Creates a :ref:`Soda document <sodadoc>` usable for SODA write operations.
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

