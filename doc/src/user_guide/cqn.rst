.. _cqn:

***********************************
Continuous Query Notification (CQN)
***********************************

`Continuous Query Notification (CQN)
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-373BAF72-3E63-42FE-8BEA-8A2AEFBF1C35>`__ allows applications to receive
notifications when a table changes, such as when rows have been updated,
regardless of the user or the application that made the change.  This can be
useful in many circumstances, such as near real-time monitoring, auditing
applications, or for such purposes as mid-tier cache invalidation.  A cache
might hold some values that depend on data in a table.  If the data in the
table changes, the cached values must then be updated with the new information.

CQN notification behavior is widely configurable.  Choices include specifying
what types of SQL should trigger a notification, whether notifications should
survive database loss, and control over unsubscription.  You can also choose
whether notification messages will include ROWIDs of affected rows.

By default, object-level (previously known as Database Change Notification)
occurs and the Python notification method is invoked whenever a database
transaction is committed that changes an object that a registered query
references, regardless of whether the actual query result changed.  However if
the :meth:`subscription <Connection.subscribe>` option ``qos`` is
:data:`cx_Oracle.SUBSCR_QOS_QUERY` then query-level notification occurs.  In
this mode, the database notifies the application whenever a transaction changing
the result of the registered query is committed.

CQN is best used to track infrequent data changes.


Requirements
============

Before using CQN, users must have appropriate permissions:

.. code-block:: sql

    GRANT CHANGE NOTIFICATION TO <user-name>;

To use CQN, connections must have ``events`` mode set to ``True``, for
example:

.. code-block:: python

    connection = cx_Oracle.connect(userName, password, "dbhost.example.com/orclpdb1", events=True)

The default CQN connection mode means the database must be able to connect back
to the application using cx_Oracle in order to receive notification events.
Alternatively, when using Oracle Database and Oracle client libraries 19.4, or
later, subscriptions can set the optional ``clientInitiated`` parameter to
``True``, see ``Connection.subscribe()`` below.

The default CQN connection mode typically means that the machine running
cx_Oracle needs a fixed IP address.  Note :meth:`Connection.subscribe()` does
not verify that this reverse connection is possible.  If there is any problem
sending a notification, then the callback method will not be invoked.
Configuration options can include an IP address and port on which cx_Oracle will
listen for notifications; otherwise, the database chooses values.


Creating a Subscription
=======================

Subscriptions allow Python to receives notifications for events that take place
in the database that match the given parameters.

For example, a basic CQN subscription might be created like:

.. code-block:: python

    connection.subscribe(namespace=cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE,
            callback=MyCallback)

See :meth:`Connection.subscribe()` for details on all of the parameters.

See :ref:`cqn-operation-codes` for the types of operations that are supported.

See :ref:`subscr-qos` for the quality of service values that are supported.

See :ref:`subscr-namespaces` and :ref:`subscr-protocols` for the namespaces and
protocols that are supported.

See :ref:`subscrobj` for more details on the subscription object that is
created.

When using Oracle Database and Oracle client libraries 19.4, or later, the
optional subscription parameter ``clientInitiated`` can be set:

.. code-block:: python

    connection.subscribe(namespace= . . ., callback=MyCallback, clientInitiated=True)

This enables CQN "client initiated" connections which internally use the same
approach as normal cx_Oracle connections to the database, and do not require the
database to be able to connect back to the application.  Since client initiated
connections do not need special network configuration they have ease-of-use and
security advantages.


Registering Queries
===================

Once a subscription has been created, one or more queries must be registered by
calling :meth:`Subscription.registerquery()`.  Registering a query behaves
similarly to :meth:`Cursor.execute()`, but only queries are permitted and the
``args`` parameter must be a sequence or dictionary.

An example script to receive query notifications when the 'CUSTOMER' table data
changes is:

.. code-block:: python

    def CQNCallback(message):
        print("Notification:")
        for query in message.queries:
            for tab in query.tables:
                print("Table:", tab.name)
                print("Operation:", tab.operation)
                for row in tab.rows:
                    if row.operation & cx_Oracle.OPCODE_INSERT:
                        print("INSERT of rowid:", row.rowid)
                    if row.operation & cx_Oracle.OPCODE_DELETE:
                        print("DELETE of rowid:", row.rowid)

    subscr = connection.subscribe(namespace=cx_Oracle.SUBSCR_NAMESPACE_DBCHANGE,
            callback=CQNCallback,
            operations=cx_Oracle.OPCODE_INSERT | cx_Oracle.OPCODE_DELETE,
            qos = cx_Oracle.SUBSCR_QOS_QUERY | cx_Oracle.SUBSCR_QOS_ROWIDS)
    subscr.registerquery("select * from regions")
    input("Hit enter to stop CQN demo\n")

Running the above script, shows the initial output as::

    Hit enter to stop CQN demo

Use SQL*Plus or another tool to commit a change to the table:

.. code-block:: sql

    insert into regions values(120, 'L');
    commit;

When the commit is executed, a notification will be received by the callback
which should print something like the following::

    Hit enter to stop CQN demo
    Notification:
    Table: HR.REGIONS
    Operation: 2
    INSERT of rowid: AAA7EsAAHAAAFS/AAA

See `GitHub Samples
<https://github.com/oracle/python-cx_Oracle/blob/master/samples/cqn.py>`__
for a runnable CQN example.
