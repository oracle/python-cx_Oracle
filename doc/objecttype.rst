.. _objecttype:

*******************
Object Type Objects
*******************

.. note::

   This object is an extension to the DB API. It is returned by the
   :meth:`~Connection.gettype()` call and is available as the
   :data:`~Variable.type` for variables containing Oracle objects.


.. attribute:: ObjectType.attributes

   This read-only attribute returns a list of the attributes that make up the
   object type. Each attribute has a name attribute on it.


.. attribute:: ObjectType.name

   This read-only attribute returns the name of the type.


.. method:: ObjectType.newobject()

   Return a new Oracle object of the given type. This object can then be
   modified by setting its attributes and then bound to a cursor for
   interaction with Oracle.


.. attribute:: ObjectType.schema

   This read-only attribute returns the name of the schema that owns the type.

