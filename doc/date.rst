.. _dateobj:

***********
Date Object
***********

.. note::

   This object is an extension the DB API. It is returned whenever Oracle
   date and timestamp (in Oracle 9i) columns are fetched and whenever the
   constructor methods (Date(), Time(), Timestamp()) are called.

.. note::

   As of Python 2.4 cx_Oracle returns the datetime objects from the
   standard library datetime module instead of these objects.


.. attribute:: Date.year

   This read-only attribute returns the year.


.. attribute:: Date.month

   This read-only attribute returns the month.


.. attribute:: Date.day

   This read-only attribute returns the day.


.. attribute:: Date.hour

   This read-only attribute returns the hour.


.. attribute:: Date.minute

   This read-only attribute returns the minute.


.. attribute:: Date.second

   This read-only attribute returns the second.


.. attribute:: Date.fsecond

   This read-only attribute returns the fractional second.

