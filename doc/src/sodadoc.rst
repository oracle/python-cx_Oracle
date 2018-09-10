.. _sodadoc:

********************
SODA Document Object
********************

.. note::

    This object is an extension the DB API. It is returned by the methods
    :meth:`SodaDatabase.createDocument()`,
    :meth:`SodaOperation.getDocuments()` and
    :meth:`SodaOperation.getOne()` as well as by iterating over
    :ref:`SODA document cursors <sodadoccur>`.

    SODA support in cx_Oracle is in Preview status and should not be used in
    production. It will be supported with a future version of Oracle Client
    libraries.


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

    Returns the content of the document as a string. This method assumes that
    the content is application/json and will raise an exception if this is not
    the case. If there is no content, however, None will be returned.

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

