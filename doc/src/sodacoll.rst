.. _sodacoll:

**********************
SODA Collection Object
**********************

.. note::

    This object is an extension the DB API. It is used to represent SODA
    collections and is created by methods 
    :meth:`SodaDatabase.createCollection()` and
    :meth:`SodaDatabase.openCollection()`.

    SODA support in cx_Oracle is in Preview status and should not be used in
    production. It will be supported with a future version of Oracle Client
    libraries.


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


.. method:: SodaCollection.insertOne(doc)

    Inserts a given document into the collection. The input document can be a
    dictionary or list or an existing :ref:`SODA document object <sodadoc>`.

    .. versionadded:: 7.0


.. method:: SodaCollection.insertOneAndGet(doc)

    Similarly to :meth:`~SodaCollection.insertOne()` this method inserts a
    given document into the collection. The only difference is that it
    returns a :ref:`SODA Document object <sodadoc>`. Note that for performance
    reasons the returned document does not contain the content.

    .. versionadded:: 7.0


.. attribute:: SodaCollection.metadata

    This read-only attribute returns a dicationary containing the metadata that
    was used to create the collection. See this `collection metadata reference
    <https://www.oracle.com/pls/topic/
    lookup?ctx=dblatest&id=GUID-49EFF3D3-9FAB-4DA6-BDE2-2650383566A3>`__
    for more information.

    .. versionadded:: 7.0


.. attribute:: SodaCollection.name

    This read-only attribute returns the name of the collection.

    .. versionadded:: 7.0

