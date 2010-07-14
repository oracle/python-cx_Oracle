.. _subscrobj:

*******************
Subscription Object
*******************

.. note::

   This object is an extension the DB API and is only available in Oracle 10g
   Release 2 and higher.


.. attribute:: Subscription.callback

   This read-only attribute returns the callback that was registered when the
   subscription was created.


.. attribute:: Subscription.connection

   This read-only attribute returns the connection that was used to register
   the subscription when it was created.


.. attribute:: Subscription.namespace

   This read-only attribute returns the namespace used to register the
   subscription when it was created.


.. attribute:: Subscription.operations

   This read-only attribute returns the operations that will send notifications
   for each table that is registered using this subscription.


.. attribute:: Subscription.port

   This read-only attribute returns the port used for callback notifications
   from the database server. If not set during construction, this value is
   zero.


.. attribute:: Subscription.protocol

   This read-only attribute returns the protocol used to register the
   subscription when it was created.


.. method:: Subscription.registerquery(statement, [args])

   Register the query for subsequent notification when tables referenced by the
   query are changed. This behaves similarly to cursor.execute() but only
   queries are permitted and the arguments must be a sequence or dictionary.


.. attribute:: Subscription.rowids

   This read-only attribute returns True or False specifying if rowids will be
   included in notifications sent using this subscription.


.. attribute:: Subscription.timeout

   This read-only attribute returns the timeout (in seconds) used to register
   the subscription when it was created. A timeout value of 0 indicates that
   there is no timeout.


Message Objects
===============

.. note::

   This object is created internally when notification is received and passed
   to the callback procedure specified when a subscription is created.


.. attribute:: Message.dbname

   This read-only attribute returns the name of the database that generated the
   notification.


.. attribute:: Message.tables

   This read-only attribute returns a list of message table objects that give
   information about the tables changed for this notification.


.. attribute:: Message.type

   This read-only attribute returns the type of message that has been sent.
   See the constants section on database change notification for additional
   information.


Message Table Objects
=====================

.. note::

   This object is created internally for each table changed when notification
   is received and is found in the tables attribute of message objects.


.. attribute:: MessageTable.name

   This read-only attribute returns the name of the table that was changed.


.. attribute:: MessageTable.operation

   This read-only attribute returns the operation that took place on the table
   that was changed.


.. attribute:: MessageTable.rows

   This read-only attribute returns a list of message row objects that give
   information about the rows changed on the table. This value is only filled
   in if the rowids argument to the Connection.subscribe() method is True.


Message Row Objects
===================

.. note::

   This object is created internally for each row changed on a table when
   notification is received and is found in the rows attribute of message table
   objects.


.. attribute:: MessageRow.operation

   This read-only attribute returns the operation that took place on the row
   that was changed.


.. attribute:: MessageRow.rowid

   This read-only attribute returns the rowid of the row that was changed.

