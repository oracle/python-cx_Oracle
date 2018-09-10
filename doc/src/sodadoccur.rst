.. _sodadoccur:

***************************
SODA Document Cursor Object
***************************

.. note::

    This object is an extension the DB API. It is returned by the method
    :meth:`SodaOperation.getCursor()` and implements the iterator protocol.
    Each iteration will return a :ref:`SODA document object <sodadoc>`.

    SODA support in cx_Oracle is in Preview status and should not be used in
    production. It will be supported with a future version of Oracle Client
    libraries.


.. method:: SodaDocCursor.close()

    Close the cursor now, rather than whenever __del__ is called. The cursor
    will be unusable from this point forward; an Error exception will be raised
    if any operation is attempted with the cursor.

    .. versionadded:: 7.0

