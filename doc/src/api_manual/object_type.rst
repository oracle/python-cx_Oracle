.. _objecttype:

*******************
Object Type Objects
*******************

.. note::

    This object is an extension to the DB API. It is returned by the
    :meth:`Connection.gettype()` call and is available as the
    :data:`Variable.type` for variables containing Oracle objects.


.. method:: ObjectType([sequence])

    The object type may be called directly and serves as an alternative way of
    calling :meth:`~ObjectType.newobject()`.


.. attribute:: ObjectType.attributes

    This read-only attribute returns a list of the :ref:`attributes
    <objectattr>` that make up the object type.


.. attribute:: ObjectType.iscollection

    This read-only attribute returns a boolean indicating if the object type
    refers to a collection or not.


.. attribute:: ObjectType.name

    This read-only attribute returns the name of the type.


.. attribute:: ObjectType.element_type

    This read-only attribute returns the type of elements found in collections
    of this type, if :attr:`~ObjectType.iscollection` is ``True``; otherwise,
    it returns ``None``. If the collection contains objects, this will be
    another object type; otherwise, it will be one of the
    :ref:`database type constants <dbtypes>`.

    .. versionadded:: 8.0


.. method:: ObjectType.newobject([sequence])

    Return a new Oracle object of the given type. This object can then be
    modified by setting its attributes and then bound to a cursor for
    interaction with Oracle. If the object type refers to a collection, a
    sequence may be passed and the collection will be initialized with the
    items in that sequence.


.. attribute:: ObjectType.schema

    This read-only attribute returns the name of the schema that owns the type.


Object Objects
--------------

.. note::

    This object is an extension to the DB API. It is returned by the
    :meth:`ObjectType.newobject()` call and can be bound to variables of
    type :data:`~cx_Oracle.OBJECT`. Attributes can be retrieved and set
    directly.

.. method:: Object.append(element)

    Append an element to the collection object. If no elements exist in the
    collection, this creates an element at index 0; otherwise, it creates an
    element immediately following the highest index available in the
    collection.


.. method:: Object.asdict()

    Return a dictionary where the collection's indexes are the keys and the
    elements are its values.

    .. versionadded:: 7.0


.. method:: Object.aslist()

    Return a list of each of the collection's elements in index order.


.. method:: Object.copy()

    Create a copy of the object and return it.


.. method:: Object.delete(index)

    Delete the element at the specified index of the collection. If the
    element does not exist or is otherwise invalid, an error is raised. Note
    that the indices of the remaining elements in the collection are not
    changed. In other words, the delete operation creates holes in the
    collection.


.. method:: Object.exists(index)

    Return True or False indicating if an element exists in the collection at
    the specified index.


.. method:: Object.extend(sequence)

    Append all of the elements in the sequence to the collection. This is
    the equivalent of performing :meth:`~Object.append()` for each element
    found in the sequence.


.. method:: Object.first()

    Return the index of the first element in the collection. If the collection
    is empty, None is returned.


.. method:: Object.getelement(index)

    Return the element at the specified index of the collection. If no element
    exists at that index, an exception is raised.


.. method:: Object.last()

    Return the index of the last element in the collection. If the collection
    is empty, None is returned.


.. method:: Object.next(index)

    Return the index of the next element in the collection following the
    specified index. If there are no elements in the collection following the
    specified index, None is returned.


.. method:: Object.prev(index)

    Return the index of the element in the collection preceding the specified
    index. If there are no elements in the collection preceding the
    specified index, None is returned.


.. method:: Object.setelement(index, value)

    Set the value in the collection at the specified index to the given value.


.. method:: Object.size()

    Return the number of elements in the collection.


.. method:: Object.trim(num)

    Remove the specified number of elements from the end of the collection.


.. _objectattr:

Object Attribute Objects
------------------------

.. note::

    This object is an extension to the DB API. The elements of
    :attr:`ObjectType.attributes` are instances of this type.


.. attribute:: ObjectAttribute.name

    This read-only attribute returns the name of the attribute.


.. attribute:: ObjectAttribute.type

    This read-only attribute returns the type of the attribute. This will be an
    :ref:`Oracle Object Type <objecttype>` if the variable binds
    Oracle objects; otherwise, it will be one of the
    :ref:`database type constants <dbtypes>`.

    .. versionadded:: 8.0
