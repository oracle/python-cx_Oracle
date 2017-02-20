.. _varobj:

****************
Variable Objects
****************

.. note::

   The DB API definition does not define this object.


.. attribute:: Variable.bufferSize

   This read-only attribute returns the size of the buffer allocated for each
   element in bytes.


.. method:: Variable.getvalue([pos=0])

   Return the value at the given position in the variable.


.. attribute:: Variable.inconverter

   This read-write attribute specifies the method used to convert data from
   Python to the Oracle database. The method signature is converter(value)
   and the expected return value is the value to bind to the database. If this
   attribute is None, the value is bound directly without any conversion.


.. attribute:: Variable.numElements

   This read-only attribute returns the number of elements allocated in an
   array, or the number of scalar items that can be fetched into the variable
   or bound to the variable.


.. attribute:: Variable.outconverter

    This read-write attribute specifies the method used to convert data from
    from the Oracle to Python. The method signature is converter(value)
    and the expected return value is the value to return to Python. If this
    attribute is None, the value is returned directly without any conversion.


.. method:: Variable.setvalue(pos, value)

   Set the value at the given position in the variable.


.. attribute:: Variable.size

   This read-only attribute returns the size of the variable. For strings this
   value is the size in characters. For all others, this is same value as the
   attribute bufferSize.


.. attribute:: Variable.type

   This read-only attribute returns the type of the variable for those
   variables that bind Oracle objects (it is not present for any other type of
   variable).

