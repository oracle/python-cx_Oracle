.. _cursorobj:

*************
Cursor Object
*************


.. attribute:: Cursor.arraysize

   This read-write attribute specifies the number of rows to fetch at a time
   internally and is the default number of rows to fetch with the
   :meth:`~Cursor.fetchmany()` call.  It defaults to 1 meaning to fetch a
   single row at a time. Note that this attribute can drastically affect the
   performance of a query since it directly affects the number of network round
   trips that need to be performed.


.. attribute:: Cursor.bindarraysize

   This read-write attribute specifies the number of rows to bind at a time and
   is used when creating variables via setinputsizes() or var(). It defaults to
   1 meaning to bind a single row at a time.

   .. note::

      The DB API definition does not define this attribute.


.. method:: Cursor.arrayvar(dataType, value, [size])

   Create an array variable associated with the cursor of the given type and
   size and return a variable object (:ref:`varobj`). The value is either an
   integer specifying the number of elements to allocate or it is a list and
   the number of elements allocated is drawn from the size of the list. If the
   value is a list, the variable is also set with the contents of the list. If
   the size is not specified and the type is a string or binary, 4000 bytes
   (maximum allowable by Oracle) is allocated. This is needed for passing
   arrays to PL/SQL (in cases where the list might be empty and the type cannot
   be determined automatically) or returning arrays from PL/SQL.

   .. note::

      The DB API definition does not define this method.


.. method:: Cursor.bindnames()

   Return the list of bind variable names bound to the statement. Note that the
   statement must have been prepared first.

   .. note::

      The DB API definition does not define this method.


.. method:: Cursor.callfunc(name, returnType, [parameters=[]])

   Call a function with the given name. The return type is specified in the
   same notation as is required by setinputsizes(). The sequence of parameters
   must contain one entry for each argument that the function expects. The
   result of the call is the return value of the function.

   .. note::

      The DB API definition does not define this method.


.. method:: Cursor.callproc(name, [parameters=[]])

   Call a procedure with the given name. The sequence of parameters must
   contain one entry for each argument that the procedure expects. The result
   of the call is a modified copy of the input sequence. Input parameters are
   left untouched; output and input/output parameters are replaced with
   possibly new values.


.. method:: Cursor.close()

   Close the cursor now, rather than whenever __del__ is called. The cursor
   will be unusable from this point forward; an Error exception will be raised
   if any operation is attempted with the cursor.


.. method:: Cursor.connection

   This read-only attribute returns a reference to the connection object on
   which the cursor was created.

   .. note::

      This attribute is an extension to the DB API definition but it is
      mentioned in PEP 249 as an optional extension.


.. data:: Cursor.description

   This read-only attribute is a sequence of 7-item sequences. Each of these
   sequences contains information describing one result column: (name, type,
   display_size, internal_size, precision, scale, null_ok). This attribute will
   be None for operations that do not return rows or if the cursor has not had
   an operation invoked via the execute() method yet.

   The type will be one of the type objects defined at the module level.


.. method:: Cursor.execute(statement, [parameters], \*\*keywordParameters)

   Execute a statement against the database. Parameters may be passed as a
   dictionary or sequence or as keyword arguments. If the arguments are a
   dictionary, the values will be bound by name and if the arguments are a
   sequence the values will be bound by position.

   A reference to the statement will be retained by the cursor. If None or the
   same string object is passed in again, the cursor will execute that
   statement again without performing a prepare or rebinding and redefining.
   This is most effective for algorithms where the same statement is used, but
   different parameters are bound to it (many times).

   For maximum efficiency when reusing an statement, it is best to use the
   setinputsizes() method to specify the parameter types and sizes ahead of
   time; in particular, None is assumed to be a string of length 1 so any
   values that are later bound as numbers or dates will raise a TypeError
   exception.

   If the statement is a query, a list of variable objects (:ref:`varobj`) will
   be returned corresponding to the list of variables into which data will be
   fetched with the :meth:`~Cursor.fetchone()`, :meth:`~Cursor.fetchmany()` and
   :meth:`~Cursor.fetchall()` methods; otherwise, ``None`` will be returned.

   .. note

      ::The DB API definition does not define the return value of this method.


.. method:: Cursor.executemany(statement, parameters)

   Prepare a statement for execution against a database and then execute it
   against all parameter mappings or sequences found in the sequence
   parameters. The statement is managed in the same way as the execute()
   method manages it.


.. method:: Cursor.executemanyprepared(numIters)

   Execute the previously prepared and bound statement the given number of
   times.  The variables that are bound must have already been set to their
   desired value before this call is made.  This method was designed for the
   case where optimal performance is required as it comes at the expense of
   compatibility with the DB API.

   .. note::

      The DB API definition does not define this method.


.. method:: Cursor.fetchall()

   Fetch all (remaining) rows of a query result, returning them as a list of
   tuples. An empty list is returned if no more rows are available. Note that
   the cursor's arraysize attribute can affect the performance of this
   operation, as internally reads from the database are done in batches
   corresponding to the arraysize.

   An exception is raised if the previous call to execute() did not produce any
   result set or no call was issued yet.


.. method:: Cursor.fetchmany([numRows=cursor.arraysize])

   Fetch the next set of rows of a query result, returning a list of tuples. An
   empty list is returned if no more rows are available. Note that the cursor's
   arraysize attribute can affect the performance of this operation.

   The number of rows to fetch is specified by the parameter. If it is not
   given, the cursor's arrysize attribute determines the number of rows to be
   fetched. If the number of rows available to be fetched is fewer than the
   amount requested, fewer rows will be returned.

   An exception is raised if the previous call to execute() did not produce any
   result set or no call was issued yet.


.. method:: Cursor.fetchone()

   Fetch the next row of a query result set, returning a single tuple or None
   when no more data is available.

   An exception is raised if the previous call to execute() did not produce any
   result set or no call was issued yet.


.. method:: Cursor.fetchraw([numRows=cursor.arraysize])

   Fetch the next set of rows of a query result into the internal buffers of
   the defined variables for the cursor. The number of rows actually fetched is
   returned.  This method was designed for the case where optimal performance
   is required as it comes at the expense of compatibility with the DB API.

   An exception is raised if the previous call to execute() did not produce any
   result set or no call was issued yet.

   .. note::

      The DB API definition does not define this method.


.. method:: Cursor.__iter__()

   Returns the cursor itself to be used as an iterator.

   .. note::

      This method is an extension to the DB API definition but it is
      mentioned in PEP 249 as an optional extension.


.. method:: Cursor.next()

   Fetch the next row of a query result set, using the same semantics as the
   method fetchone().

   .. note::

      This method is an extension to the DB API definition but it is
      mentioned in PEP 249 as an optional extension.


.. attribute:: Cursor.numbersAsStrings

   This integer attribute defines whether or not numbers should be returned as
   strings rather than integers or floating point numbers. This is useful to
   get around the fact that Oracle floating point numbers have considerably
   greater precision than C floating point numbers and not require a change to
   the SQL being executed.

   .. note::

      The DB API definition does not define this attribute.


.. method:: Cursor.parse(statement)

   This can be used to parse a statement without actually executing it (this
   step is done automatically by Oracle when a statement is executed).

   .. note::

      The DB API definition does not define this method.


.. method:: Cursor.prepare(statement, [tag])

   This can be used before a call to execute() to define the statement that
   will be executed. When this is done, the prepare phase will not be performed
   when the call to execute() is made with None or the same string object as
   the statement.  If specified (Oracle 9i and higher) the statement will be
   returned to the statement cache with the given tag. See the Oracle
   documentation for more information about the statement cache.

   .. note::

      The DB API definition does not define this method.


.. attribute:: Cursor.rowcount

   This read-only attribute specifies the number of rows that have currently
   been fetched from the cursor (for select statements) or that have been
   affected by the operation (for insert, update and delete statements).


.. attribute:: Cursor.rowfactory

   This read-write attribute specifies a method to call for each row that is
   retrieved from the database. Ordinarily a tuple is returned for each row but
   if this attribute is set, the method is called with the argument tuple that
   would normally be returned and the result of the method is returned instead.

   .. note::

      The DB API definition does not define this attribute.


.. method:: Cursor.setinputsizes(\*args, \*\*keywordArgs)

   This can be used before a call to execute() to predefine memory areas for
   the operation's parameters. Each parameter should be a type object
   corresponding to the input that will be used or it should be an integer
   specifying the maximum length of a string parameter. Use keyword arguments
   when binding by name and positional arguments when binding by position. The
   singleton None can be used as a parameter when using positional arguments to
   indicate that no space should be reserved for that position.


.. method:: Cursor.setoutputsize(size, [column])

   This can be used before a call to execute() to predefine memory areas for
   the long columns that will be fetched. The column is specified as an index
   into the result sequence. Not specifying the column will set the default
   size for all large columns in the cursor.


.. attribute:: Cursor.statement

   This read-only attribute provides the string object that was previously
   prepared with prepare() or executed with execute().

   .. note::

      The DB API definition does not define this attribute.


.. method:: Cursor.var(dataType, [size])

   Create a variable associated with the cursor of the given type and size and
   return a variable object (:ref:`varobj`). If the size is not specified and
   the type is a string or binary, 4000 bytes (maximum allowable by Oracle) is
   allocated; if the size is not specified and the type is a long string or
   long binary, 128KB is allocated. This method was designed for use with
   PL/SQL in/out variables where the length or type cannot be determined
   automatically from the Python object passed in.

   .. note::

      The DB API definition does not define this method.

