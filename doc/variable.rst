.. _varobj:

****************
Variable Objects
****************

.. note::

   The DB API definition does not define this object.



.. attribute:: Variable.allocelems

   This read-only attribute returns the number of elements allocated in an
   array, or the number of scalar items that can be fetched into the variable.


.. method:: Variable.getvalue([pos=0])

   Return the value at the given position in the variable.


.. attribute:: Variable.maxlength

   This read-only attribute returns the maximum length of the variable.


.. method:: Variable.setvalue(pos, value)

   Set the value at the given position in the variable.

