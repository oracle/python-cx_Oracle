.. _lobobj:

***********
LOB Objects
***********

.. note::

   This object is an extension the DB API. It is returned whenever Oracle
   :data:`CLOB`, :data:`BLOB` and :data:`BFILE` columns are fetched.


.. note::

   Internally, Oracle uses LOB locators which are allocated based on the
   cursor array size. Thus, it is important that the data in the LOB object be
   manipulated before another internal fetch takes place. The safest way to do
   this is to use the cursor as an iterator. In particular, do not use the
   fetchall() method. The exception "LOB variable no longer valid after
   subsequent fetch" will be raised if an attempt to access a LOB variable
   after a subsequent fetch is detected.


.. method:: LOB.close()

   Close the :data:`LOB`. Call this when writing is completed so that the
   indexes associated with the LOB can be updated.


.. method:: LOB.fileexists()

   Return a boolean indicating if the file referenced by the :data:`BFILE`
   type LOB exists.


.. method:: LOB.getchunksize()

   Return the chunk size for the internal LOB. Reading and writing to the LOB
   in chunks of multiples of this size will improve performance.


.. method:: LOB.getfilename()

   Return a two-tuple consisting of the directory alias and file name for a
   :data:`BFILE` type LOB.


.. method:: LOB.isopen()

   Return a boolean indicating if the LOB is opened.


.. method:: LOB.open()

   Open the LOB for writing. This will improve performance when writing to a
   LOB in chunks and there are functional or extensible indexes associated with
   the LOB.


.. method:: LOB.read([offset=1, [amount]])

   Return a portion (or all) of the data in the LOB object.


.. method:: LOB.setfilename(dirAlias, name)

   Set the directory alias and name of the :data:`BFILE` type LOB.


.. method:: LOB.size()

   Returns the size of the data in the LOB object.


.. method:: LOB.trim([newSize=0])

   Trim the LOB to the new size.


.. method:: LOB.write(data, [offset=1])

   Write the data to the LOB object at the given offset. Note that if you want
   to make the LOB value smaller, you must use the trim() function.

