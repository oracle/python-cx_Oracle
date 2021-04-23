.. _highavailability:

********************************
High Availability with cx_Oracle
********************************

Applications can utilize many features for high availability (HA) during planned and
unplanned outages in order to:

*  Reduce application downtime
*  Eliminate compromises between high availability and performance
*  Increase operational productivity

.. _harecommend:

General HA Recommendations
--------------------------

General recommendations for creating highly available cx_Oracle programs are:

* Tune operating system and Oracle Network parameters to avoid long TCP timeouts, to prevent firewalls killing connections, and to avoid connection storms.
* Implement application error handling and recovery.
* Use the most recent version of the Oracle client libraries.  New versions have improvements to features such as dead database server detection, and make it easier to set connection options.
* Use the most recent version of Oracle Database.  New database versions introduce, and enhance, features such as Application Continuity (AC) and Transparent Application Continuity (TAC).
* Utilize Oracle Database technologies such as `RAC <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=RACAD>`__ or standby databases.
* Configure database services to emit :ref:`FAN <fan>` events.
* Use a :ref:`connection pool <connpool>`, because pools can handle database events and take proactive and corrective action for draining, run time load balancing, and fail over.  Set the minimum and maximum pool sizes to the same values to avoid connection storms. Remove resource manager or user profiles that prematurely close sessions.
* Test all scenarios thoroughly.

.. _hanetwork:

Network Configuration
---------------------

The operating system TCP and :ref:`Oracle Net configuration <optnetfiles>`
should be configured for performance and availability.

Options such as `SQLNET.OUTBOUND_CONNECT_TIMEOUT
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-0857C817-675F-4CF0-BFBB-C3667F119176>`__,
`SQLNET.RECV_TIMEOUT
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-4A19D81A-75F0-448E-B271-24E5187B5909>`__
and `SQLNET.SEND_TIMEOUT
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-48547756-9C0B-4D14-BE85-E7ADDD1A3A66>`__
can be explored.

`Oracle Net Services
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=NETRF>`__ options may
also be useful for high availability and performance tuning.  For example the
database's `listener.ora` file can have `RATE_LIMIT
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-F302BF91-64F2-4CE8-A3C7-9FDB5BA6DCF8>`__
and `QUEUESIZE
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-FF87387C-1779-4CC3-932A-79BB01391C28>`__
parameters that can help handle connection storms.

With Oracle Client 19c, `EXPIRE_TIME
<https://docs.oracle.com/en/database/oracle/oracle-database/20/netrf/local-naming-parameters-in-tns-ora-file.html#GUID-6140611A-83FC-4C9C-B31F-A41FC2A5B12D>`__
can be used in :ref:`tnsnames.ora <optnetfiles>` connect descriptors to prevent
firewalls from terminating idle connections and to adjust keepalive timeouts.
The general recommendation for ``EXPIRE_TIME`` is to use a value that is
slightly less than half of the termination period.  In older versions of Oracle
Client, a ``tnsnames.ora`` connect descriptor option `ENABLE=BROKEN
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-7A18022A-E40D-4880-B3CE-7EE9864756CA>`_
can be used instead of ``EXPIRE_TIME``.  These settings can also aid detection
of a terminated remote database server.

When cx_Oracle uses :ref:`Oracle Client libraries 19c <archfig>`, then the
:ref:`Easy Connect Plus syntax <easyconnect>` syntax enables some options to be
used without needing a ``sqlnet.ora`` file.  For example, if your firewall times
out every 4 minutes, and you cannot alter the firewall settings, then you may
decide to use ``EXPIRE_TIME`` in your connect string to send a probe every 2
minutes to the database to keep connections 'alive'::

    connection = cx_Oracle.connect("hr", userpwd, "dbhost.example.com/orclpdb1?expire_time=2")

.. _fan:

Fast Application Notification (FAN)
-----------------------------------

Users of `Oracle Database FAN
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-F3FBE48B-468B-4393-8B0C-D5C8E0E4374D>`__
must connect to a FAN-enabled database service.  The application should have
``events`` set to True when connecting.  This value can also be changed via
:ref:`Oracle Client Configuration <optclientfiles>`.

FAN support is useful for planned and unplanned outages.  It provides immediate
notification to cx_Oracle following outages related to the database, computers,
and networks.  Without FAN, cx_Oracle can hang until a TCP timeout occurs and an
error is returned, which might be several minutes.

FAN allows cx_Oracle to provide high availability features without the
application being aware of an outage.  Unused, idle connections in a
:ref:`connection pool <connpool>` will be automatically cleaned up.  A future
:meth:`SessionPool.acquire()` call will establish a fresh connection to a
surviving database instance without the application being aware of any service
disruption.

To handle errors that affect active connections, you can add application logic
to re-connect (this will connect to a surviving database instance) and replay
application logic without having to return an error to the application user.

FAN benefits users of Oracle Database's clustering technology `Oracle RAC
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-D04AA2A7-2E68-4C5C-BD6E-36C62427B98E>`__
because connections to surviving database instances can be immediately made.
Users of Oracle's Data Guard with a broker will get FAN events generated when
the standby database goes online.  Standalone databases will send FAN events
when the database restarts.

For a more information on FAN see the `white paper on Fast Application
Notification
<https://www.oracle.com/technetwork/database/options/clustering/applicationcontinuity/learnmore/fastapplicationnotification12c-2538999.pdf>`__.

.. _appcont:

Application Continuity (AC)
---------------------------

Oracle Application Continuity and Transparent Application Continuity are Oracle
Database technologies that record application interaction with the database and,
in the event of a database instance outage, attempt to replay the interaction on
a surviving database instance.  If successful, users will be unaware of any
database issue.  AC and TAC are best suited for OLTP applications.

When AC or TAC are configured on the database service, they are transparently
available to cx_Oracle applications.

You must thoroughly test your application because not all lower level calls in
the cx_Oracle implementation can be replayed.

See `OCI and Application Continuity
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-A8DD9422-2F82-42A9-9555-134296416E8F>`__
for more information.

.. _tg:

Transaction Guard
-----------------

cx_Oracle supports `Transaction Guard
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-A675AF7B-6FF0-460D-A6E6-C15E7C328C8F>`__ which enables Python
application to verify the success or failure of the last transaction in the
event of an unplanned outage. This feature is available when both client and
database are 12.1 or higher.

Using Transaction Guard helps to:

*  Preserve the commit outcome
*  Ensure a known outcome for every transaction

See `Oracle Database Development Guide
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
id=GUID-6C5880E5-C45F-4858-A069-A28BB25FD1DB>`__ for more information about
using Transaction Guard.

When an error occurs during commit, the Python application can acquire the
logical transaction id (``ltxid``) from the connection and then call a
procedure to determine the outcome of the commit for this logical transaction
id.

Follow the steps below to use the Transaction Guard feature in Python:

1.  Grant execute privileges to the database users who will be checking the
    outcome of the commit. Login as SYSDBA and run the following command:

    .. code-block:: sql

        GRANT EXECUTE ON DBMS_APP_CONT TO <username>;

2.  Create a new service by executing the following PL/SQL block as SYSDBA.
    Replace the ``<service-name>``, ``<network-name>`` and
    ``<retention-value>`` values with suitable values. It is important that the
    ``COMMIT_OUTCOME`` parameter be set to true for Transaction Guard to
    function properly.

    .. code-block:: sql

        DECLARE
            t_Params dbms_service.svc_parameter_array;
        BEGIN
            t_Params('COMMIT_OUTCOME') := 'true';
            t_Params('RETENTION_TIMEOUT') := <retention-value>;
            DBMS_SERVICE.CREATE_SERVICE('<service-name>', '<network-name>', t_Params);
        END;
        /

3.  Start the service by executing the following PL/SQL block as SYSDBA:

    .. code-block:: sql

        BEGIN
            DBMS_SERVICE.start_service('<service-name>');
        END;
        /

Ensure the service is running by examining the output of the following query:

    .. code-block:: sql

        SELECT name, network_name FROM V$ACTIVE_SERVICES ORDER BY 1;


**Python Application code requirements to use Transaction Guard**

In the Python application code:

* Use the connection attribute :attr:`~Connection.ltxid` to determine the
  logical transaction id.
* Call the ``DBMS_APP_CONT.GET_LTXID_OUTCOME`` PL/SQL procedure with the
  logical transaction id acquired from the connection attribute.  This returns
  a boolean value indicating if the last transaction was committed and whether
  the last call was completed successfully or not.

See the `Transaction Guard Sample
<https://github.com/oracle/python-cx_Oracle/blob/master/
samples/transaction_guard.py>`__ for further details.
