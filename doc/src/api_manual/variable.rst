.. _varobj:

****************
Variable Objects
****************

.. note::

    The DB API definition does not define this object.


.. attribute:: Variable.actual_elements

    This read-only attribute returns the actual number of elements in the
    variable. This corresponds to the number of elements in a PL/SQL index-by
    table for variables that are created using the method
    :func:`Cursor.arrayvar()`. For all other variables this value will be
    identical to the attribute :attr:`~Variable.numElements`.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `actualElements` was renamed to `actual_elements`. The old
        name will continue to work for a period of time.


.. attribute:: Variable.buffer_size

    This read-only attribute returns the size of the buffer allocated for each
    element in bytes.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `bufferSize` was renamed to `buffer_size`. The old
        name will continue to work for a period of time.


.. method:: Variable.getvalue([pos=0])

    Return the value at the given position in the variable. For variables
    created using the method :func:`Cursor.arrayvar()` the value returned will
    be a list of each of the values in the PL/SQL index-by table. For variables
    bound to DML returning statements, the value returned will also be a list
    corresponding to the returned data for the given execution of the statement
    (as identified by the pos parameter).


.. attribute:: Variable.inconverter

    This read-write attribute specifies the method used to convert data from
    Python to the Oracle database. The method signature is converter(value)
    and the expected return value is the value to bind to the database. If this
    attribute is None, the value is bound directly without any conversion.


.. attribute:: Variable.num_elements

    This read-only attribute returns the number of elements allocated in an
    array, or the number of scalar items that can be fetched into the variable
    or bound to the variable.

    .. versionchanged:: 8.2

        For consistency and compliance with the PEP 8 naming style, the
        attribute `numElements` was renamed to `num_elements`. The old
        name will continue to work for a period of time.


.. attribute:: Variable.outconverter

     This read-write attribute specifies the method used to convert data from
     the Oracle database to Python. The method signature is converter(value)
     and the expected return value is the value to return to Python. If this
     attribute is None, the value is returned directly without any conversion.


.. method:: Variable.setvalue(pos, value)

    Set the value at the given position in the variable.


.. attribute:: Variable.size

    This read-only attribute returns the size of the variable. For strings this
    value is the size in characters. For all others, this is same value as the
    attribute bufferSize.


.. attribute:: Variable.type

    This read-only attribute returns the type of the variable. This will be an
    :ref:`Oracle Object Type <objecttype>` if the variable binds
    Oracle objects; otherwise, it will be one of the
    :ref:`database type constants <dbtypes>`.

    .. versionchanged:: 8.0
        Database type constants are now used when the variable is not used for
        binding Oracle objects.


.. attribute:: Variable.values

    This read-only attribute returns a copy of the value of all actual
    positions in the variable as a list. This is the equivalent of calling
    :meth:`~Variable.getvalue()` for each valid position and the length will
    correspond to the value of the :attr:`~Variable.actualElements` attribute.
