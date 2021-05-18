.. _exception:

******************
Exception Handling
******************

All exceptions raised by cx_Oracle are inherited from :attr:`cx_Oracle.Error`.
See :ref:`Exceptions <exceptions>` for more details on the various exceptions
defined by cx_Oracle. See the exception handling section in the
:ref:`API manual <exchandling>` for more details on the information available
when an exception is raised.

Applications can catch exceptions as needed. For example, when trying to add a
customer that already exists in the database, the following could be used
to catch the exception:

.. code-block:: python

    try:
        cursor.execute("insert into customer values (101, 'Customer A')")
    except cx_Oracle.IntegrityError:
        print("Customer ID already exists")
    else:
        print("Customer added")


If information about the exception needs to be processed instead, the following
code can be used:

.. code-block:: python

    try:
        cursor.execute("insert into customer values (101, 'Customer A')")
    except cx_Oracle.IntegrityError as e:
        error_obj, = e.args
        print("Customer ID already exists")
        print("Error Code:", error_obj.code)
        print("Error Message:", error_obj.message)
    else:
        print("Customer added")
