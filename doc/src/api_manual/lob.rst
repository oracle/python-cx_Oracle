.. _lobobj:

***********
LOB Objects
***********

See :ref:`lobdata` for more information about using LOBs.

.. note::

    This object is an extension the DB API. It is returned whenever Oracle
    :data:`CLOB`, :data:`BLOB` and :data:`BFILE` columns are fetched.


.. method:: LOB.close()

    Close the LOB. Call this when writing is completed so that the indexes
    associated with the LOB can be updated -- but only if :meth:`~LOB.open()`
    was called first.


.. method:: LOB.fileexists()

    Return a boolean indicating if the file referenced by the BFILE type LOB
    exists.


.. method:: LOB.getchunksize()

    Return the chunk size for the internal LOB. Reading and writing to the LOB
    in chunks of multiples of this size will improve performance.


.. method:: LOB.getfilename()

    Return a two-tuple consisting of the directory alias and file name for a
    BFILE type LOB.


.. method:: LOB.isopen()

    Return a boolean indicating if the LOB has been opened using the method
    :meth:`~LOB.open()`.


.. method:: LOB.open()

    Open the LOB for writing. This will improve performance when writing to a
    LOB in chunks and there are functional or extensible indexes associated
    with the LOB. If this method is not called, each write will perform an open
    internally followed by a close after the write has been completed.


.. method:: LOB.read([offset=1, [amount]])

    Return a portion (or all) of the data in the LOB object. Note that the
    amount and offset are in bytes for BLOB and BFILE type LOBs and in UCS-2
    code points for CLOB and NCLOB type LOBs. UCS-2 code points are equivalent
    to characters for all but supplemental characters. If supplemental
    characters are in the LOB, the offset and amount will have to be chosen
    carefully to avoid splitting a character.


.. method:: LOB.setfilename(dirAlias, name)

    Set the directory alias and name of the BFILE type LOB.


.. method:: LOB.size()

    Returns the size of the data in the LOB object. For BLOB and BFILE type
    LOBs this is the number of bytes. For CLOB and NCLOB type LOBs this is the
    number of UCS-2 code points. UCS-2 code points are equivalent to characters
    for all but supplemental characters.


.. method:: LOB.trim(new_size=0)

    Trim the LOB to the new size.


.. attribute:: LOB.type

    This read-only attribute returns the type of the LOB as one of the
    :ref:`database type constants <dbtypes>`.

    .. versionadded:: 8.0


.. method:: LOB.write(data, offset=1)

    Write the data to the LOB object at the given offset. The offset is in
    bytes for BLOB type LOBs and in UCS-2 code points for CLOB and NCLOB type
    LOBs. UCS-2 code points are equivalent to characters for all but
    supplemental characters. If supplemental characters are in the LOB, the
    offset will have to be chosen carefully to avoid splitting a character.
    Note that if you want to make the LOB value smaller, you must use the
    :meth:`~LOB.trim()` function.
