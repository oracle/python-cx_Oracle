.. _initialization:

**************************
cx_Oracle 8 Initialization
**************************

The cx_Oracle module loads Oracle Client libraries which communicate over
Oracle Net to an existing database.  The Oracle Client libraries need to be
installed separately.  See :ref:`installation`.  Oracle Net is not a separate
product: it is how the Oracle Client and Oracle Database communicate.

.. figure:: /images/cx_Oracle_arch.png

   cx_Oracle Architecture

.. _libinit:

Locating the Oracle Client Libraries
====================================

cx_Oracle dynamically loads the Oracle Client libraries using a search
heuristic.  Only the first set of libraries found are loaded.  The libraries
can be in an installation of Oracle Instant Client, in a full Oracle Client
installation, or in an Oracle Database installation (if Python is running on
the same machine as the database).  The versions of Oracle Client and Oracle
Database do not have to be the same.  For certified configurations see Oracle
Support's `Doc ID 207303.1
<https://support.oracle.com/epmos/faces/DocumentDisplay?id=207303.1>`__.


.. _wininit:

* On Windows, cx_Oracle looks for the Oracle Client libraries as follows:

    - In the ``lib_dir`` directory specified in a call to
      :meth:`cx_Oracle.init_oracle_client()`.  This directory should contain
      the libraries from an unzipped Instant Client 'Basic' or 'Basic Light'
      package.  If you pass the library directory from a full client or
      database installation, such as Oracle Database "XE" Express Edition, then
      you will need to have previously set your environment to use that
      software installation, otherwise files such as message files will not be
      located. On Windows when the path contains backslashes, use a 'raw'
      string like ``lib_dir = r"C:\instantclient_19_6"``. If the Oracle Client
      libraries cannot be loaded from ``lib_dir``, then an exception is raised.

    - If ``lib_dir`` was not specified, then Oracle Client libraries are looked
      for in the directory where the cx_Oracle binary module is installed.
      This directory should contain the libraries from an unzipped Instant
      Client 'Basic' or 'Basic Light' package.  If the libraries are not found,
      no exception is raised and the search continues, see next bullet point.

    - In the directories on the system library search path, e.g. the ``PATH``
      environment variable.  If the Oracle Client libraries cannot be loaded,
      then an exception is raised.

.. _macinit:

* On macOS, cx_Oracle looks for the Oracle Client libraries as follows:

    - In the ``lib_dir`` directory specified in a call to
      :meth:`cx_Oracle.init_oracle_client()`.  This directory should contain
      the libraries from an unzipped Instant Client 'Basic' or 'Basic Light'
      package.  If the Oracle Client libraries cannot be loaded from
      ``lib_dir``, then an exception is raised.

    - If ``lib_dir`` was not specified, then Oracle Client libraries are looked
      for in the directory where the cx_Oracle binary module is.  This directory
      should contain the libraries from an unzipped Instant Client 'Basic' or
      'Basic Light' package.  For example if
      ``/Users/your_username/Library/Python/3.8/lib/python/site-packages``
      contains ``cx_Oracle.cpython-38-darwin.so``, then you could run ``ln -s
      ~/instantclient_19_3/libclntsh.dylib
      ~/Library/Python/3.8/lib/python/site-packages``.  If the libraries are not
      found, no exception is raised and the search continues, see next bullet
      point.

    - In the directories on the system library search path, e.g. ``~/lib/`` and
      ``/usr/local/lib``, or in ``$DYLD_LIBRARY_PATH``.  These paths will vary
      with macOS version and Python version.  Any value in
      ``DYLD_LIBRARY_PATH`` will not propagate to a sub-shell.  If the Oracle
      Client libraries cannot be loaded, then an exception is raised.

.. _linuxinit:

* On Linux and related platforms, cx_Oracle looks for the Oracle Client
  libraries as follows:

    - In the ``lib_dir`` directory specified in a call to
      :meth:`cx_Oracle.init_oracle_client()`.

      **Note this is only useful to force immediate loading of the libraries
      because on Linux and related platforms the libraries must always be in the
      system library search path**.

      The ``lib_dir`` directory should contain the libraries from an unzipped
      Instant Client 'Basic' or 'Basic Light' package.  If you pass the library
      directory from a full client or database installation, such as Oracle
      Database "XE" Express Edition then you will need to have previously set
      the ``ORACLE_HOME`` environment variable.  If the Oracle Client libraries
      cannot be loaded from ``lib_dir``, then an exception is raised.

    - If ``lib_dir`` was not specified, then Oracle Client libraries are looked
      for in the operating system library search path, such as configured with
      ``ldconfig`` or set in the environment variable ``LD_LIBRARY_PATH``.  On
      some UNIX platforms an OS specific equivalent, such as ``LIBPATH`` or
      ``SHLIB_PATH`` is used instead of ``LD_LIBRARY_PATH``.  If the libraries
      are not found, no exception is raised and the search continues, see next
      bullet point.

    - In ``$ORACLE_HOME/lib``.  Note the environment variable ``ORACLE_HOME``
      should only ever be set when you have a full database installation or
      full client installation.  It should not be set if you are using Oracle
      Instant Client.  The ``ORACLE_HOME`` variable, and other necessary
      variables, should be set before starting Python.  See :ref:`envset`.  If
      the Oracle Client libraries cannot be loaded, then an exception is
      raised.

If you call :meth:`cx_Oracle.init_oracle_client()` with a ``lib_dir``
parameter, the Oracle Client libraries are loaded immediately from that
directory.  If you call :meth:`cx_Oracle.init_oracle_client()` but do *not* set
the ``lib_dir`` parameter, the Oracle Client libraries are loaded immediately
using the search heuristic above.  If you do not call
:meth:`cx_Oracle.init_oracle_client()`, then the libraries are loaded using the
search heuristic when the first cx_Oracle function that depends on the
libraries is called, for example when a connection pool is created.  If there
is a problem loading the libraries, then an exception is raised.

Make sure the Python process has directory and file access permissions for the
Oracle Client libraries.  On Linux ensure a ``libclntsh.so`` file exists.  On
macOS ensure a ``libclntsh.dylib`` file exists.  cx_Oracle will not directly
load ``libclntsh.*.XX.1`` files in ``lib_dir`` or from the directory where the
cx_Oracle binary module is.  Note other libraries used by ``libclntsh*`` are
also required.

To trace the loading of Oracle Client libraries, the environment variable
``DPI_DEBUG_LEVEL`` can be set to 64 before starting Python.  For example, on
Linux, you might use::

    $ export DPI_DEBUG_LEVEL=64
    $ python myapp.py 2> log.txt


.. _usinginitoracleclient:

Using cx_Oracle.init_oracle_client() to set the Oracle Client directory
-----------------------------------------------------------------------

Applications can call the function :meth:`cx_Oracle.init_oracle_client()` to
specify the directory containing Oracle Instant Client libraries.  The Oracle
Client Libraries are loaded when ``init_oracle_client()`` is called.  For
example, if the Oracle Instant Client Libraries are in
``C:\oracle\instantclient_19_9`` on Windows or
``$HOME/Downloads/instantclient_19_8`` on macOS, then you can use:

.. code-block:: python

    import cx_Oracle
    import sys
    import os

    try:
        if sys.platform.startswith("darwin"):
            lib_dir = os.path.join(os.environ.get("HOME"), "Downloads",
                                   "instantclient_19_8")
            cx_Oracle.init_oracle_client(lib_dir=lib_dir)
        elif sys.platform.startswith("win32"):
            cx_Oracle.init_oracle_client(lib_dir=r"C:\oracle\instantclient_19_9")
    except Exception as err:
        print("Whoops!")
        print(err);
        sys.exit(1);

Note the use of a 'raw' string ``r"..."`` on Windows so that backslashes are
treated as directory separators.

The :meth:`~cx_Oracle.init_oracle_client()` function can only be called once.

**Note if you set** ``lib_dir`` **on Linux and related platforms, you must still
have configured the system library search path to include that directory before
starting Python**.

On any operating system, if you set ``lib_dir`` to the library directory of a
full database or full client installation, you will need to have previously set
the Oracle environment, for example by setting the ``ORACLE_HOME`` environment
variable.  Otherwise you will get errors like ORA-1804.  You should set this,
and other Oracle environment variables, before starting Python, as
shown in :ref:`envset`.

.. _optnetfiles:

Optional Oracle Net Configuration Files
=======================================

Optional Oracle Net configuration files are read when cx_Oracle is loaded.
These files affect connections and applications.  The common files are:

* ``tnsnames.ora``: A configuration file that defines databases addresses
  for establishing connections. See :ref:`Net Service Name for Connection
  Strings <netservice>`.

* ``sqlnet.ora``: A profile configuration file that may contain information
  on features such as connection failover, network encryption, logging, and
  tracing.  See `Oracle Net Services Reference
  <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&
  id=GUID-19423B71-3F6C-430F-84CC-18145CC2A818>`__ for more information.

The files should be in a directory accessible to Python, not on the database
server host.

For example, if the file ``/etc/my-oracle-config/tnsnames.ora`` should be used,
you can call :meth:`cx_Oracle.init_oracle_client()`:

.. code-block:: python

    import cx_Oracle
    import sys

    try:
        cx_Oracle.init_oracle_client(config_dir="/etc/my-oracle-config")
    except Exception as err:
        print("Whoops!")
        print(err);
        sys.exit(1);

This is equivalent to setting the environment variable `TNS_ADMIN
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-12C94B15-2CE1-4B98-9D0C-8226A9DDF4CB>`__
to ``/etc/my-oracle-config``.

If :meth:`~cx_Oracle.init_oracle_client()` is not called, or it is called but
``config_dir`` is not specified, then default directories searched for the
configuration files.  They include:

* ``$TNS_ADMIN``
* ``/opt/oracle/instantclient_19_6/network/admin`` if Instant Client is in ``/opt/oracle/instantclient_19_6``.
* ``/usr/lib/oracle/19.6/client64/lib/network/admin`` if Oracle 19.6 Instant Client RPMs are used on Linux.
* ``$ORACLE_HOME/network/admin`` if cx_Oracle is using libraries from a database installation.

A wallet configuration file ``cwallet.sso`` for secure connection can be
located with, or separately from, the ``tnsnames.ora`` and ``sqlnet.ora``
files.  It should be securely stored.  The ``sqlnet.ora`` file's
``WALLET_LOCATION`` path should be set to the directory containing
``cwallet.sso``.  For Oracle Autonomous Database use of wallets, see
:ref:`autononmousdb`.

Note the :ref:`easyconnect` can set many common configuration options without
needing ``tnsnames.ora`` or ``sqlnet.ora`` files.

The section :ref:`Network Configuration <hanetwork>` has some discussion about
Oracle Net configuration.

.. _optclientfiles:

Optional Oracle Client Configuration Files
==========================================

When cx_Oracle uses Oracle Client libraries version 12.1, or later, an optional
client parameter file called ``oraaccess.xml`` can be used to configure some
behviors of those libraries, such as statement caching and prefetching. This can
be useful if the application cannot be altered. The file is read from the same
directory as the `Optional Oracle Net Configuration Files`_.

A sample ``oraaccess.xml`` file that sets the Oracle client ‘prefetch’ value to
1000 rows.  This value affects every SQL query in the application::

    <?xml version="1.0"?>
     <oraaccess xmlns="http://xmlns.oracle.com/oci/oraaccess"
      xmlns:oci="http://xmlns.oracle.com/oci/oraaccess"
      schemaLocation="http://xmlns.oracle.com/oci/oraaccess
      http://xmlns.oracle.com/oci/oraaccess.xsd">
      <default_parameters>
        <prefetch>
          <rows>1000</rows>
        </prefetch>
      </default_parameters>
    </oraaccess>

Prefetching is the number of additional rows the underlying Oracle client
library fetches whenever cx_Oracle requests query data from the database.
Prefetching is a tuning option to maximize data transfer efficiency and minimize
:ref:`round-trips <roundtrips>` to the database.  The prefetch size does not
affect when, or how many, rows are returned by cx_Oracle to the application.
The cache management is transparently handled by the Oracle client libraries.
Note, standard cx_Oracle fetch tuning is via :attr:`Cursor.arraysize`, but
changing the prefetch value can be useful in some cases such as when modifying
the application is not feasible.

The `oraaccess.xml` file has other uses including:

- Changing the value of Fast Application Notification :ref:`FAN <fan>` events which affects notifications and Runtime Load Balancing (RLB).
- Configuring `Client Result Caching <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-D2FA7B29-301B-4AB8-8294-2B1B015899F9>`__ parameters
- Turning on `Client Statement Cache Auto-tuning <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-6E21AA56-5BBE-422A-802C-197CAC8AAEA4>`__

Refer to the documentation on `oraaccess.xml
<https://www.oracle.com/pls/topic/lookup?
ctx=dblatest&id=GUID-9D12F489-EC02-46BE-8CD4-5AECED0E2BA2>`__
for more details.

.. _envset:

Oracle Environment Variables
============================

Some common environment variables that influence cx_Oracle are shown below.  The
variables that may be needed depend on how Python is installed, how you connect
to the database, and what optional settings are desired.  It is recommended to
set Oracle variables in the environment before invoking Python, however they may
also be set in the application with ``os.putenv()`` before the first connection
is established.  System environment variables like ``LD_LIBRARY_PATH`` must be
set before Python starts.

.. list-table:: Common Oracle environment variables
    :header-rows: 1
    :widths: 1 2
    :align: left

    * - Oracle Environment Variables
      - Purpose
    * - LD_LIBRARY_PATH
      - The library search path for platforms like Linux should include the
        Oracle libraries, for example ``$ORACLE_HOME/lib`` or
        ``/opt/instantclient_19_3``. This variable is not needed if the
        libraries are located by an alternative method, such as with
        ``ldconfig``. On other UNIX platforms you may need to set an OS
        specific equivalent, such as ``LIBPATH`` or ``SHLIB_PATH``.
    * - PATH
      - The library search path for Windows should include the location where
        ``OCI.DLL`` is found.  Not needed if you set ``lib_dir`` in a call to
        :meth:`cx_Oracle.init_oracle_client()`
    * - TNS_ADMIN
      - The directory of optional Oracle Client configuration files such as
        ``tnsnames.ora`` and ``sqlnet.ora``. Not needed if the configuration
        files are in a default location or if ``config_dir`` was not used in
        :meth:`cx_Oracle.init_oracle_client()`.  See :ref:`optnetfiles`.
    * - ORA_SDTZ
      - The default session time zone.
    * - ORA_TZFILE
      - The name of the Oracle time zone file to use.  See below.
    * - ORACLE_HOME
      - The directory containing the Oracle Database software. The directory
        and various configuration files must be readable by the Python process.
        This variable should not be set if you are using Oracle Instant Client.
    * - NLS_LANG
      - Determines the 'national language support' globalization options for
        cx_Oracle. Note: from cx_Oracle 8, the character set component is
        ignored and only the language and territory components of ``NLS_LANG``
        are used. The character set can instead be specified during connection
        or connection pool creation. See :ref:`globalization`.
    * - NLS_DATE_FORMAT, NLS_TIMESTAMP_FORMAT
      - Often set in Python applications to force a consistent date format
        independent of the locale. The variables are ignored if the environment
        variable ``NLS_LANG`` is not set.

Oracle Instant Client includes a small and big time zone file, for example
``timezone_32.dat`` and ``timezlrg_32.dat``.  The versions can be shown by running
the utility ``genezi -v`` located in the Instant Client directory.  The small file
contains only the most commonly used time zones.  By default the larger
``timezlrg_n.dat`` file is used.  If you want to use the smaller ``timezone_n.dat``
file, then set the ``ORA_TZFILE`` environment variable to the name of the file
without any directory prefix, for example ``export ORA_TZFILE=timezone_32.dat``.
With Oracle Instant Client 12.2 or later, you can also use an external time zone
file.  Create a subdirectory ``oracore/zoneinfo`` under the Instant Client
directory, and move the file into it.  Then set ``ORA_TZFILE`` to the file name,
without any directory prefix.  The ``genezi -v`` utility will show the time zone
file in use.

If cx_Oracle is using Oracle Client libraries from an Oracle Database or full
Oracle Client software installation, and you want to use a non-default time zone
file, then set ``ORA_TZFILE`` to the file name with a directory prefix, for
example: ``export ORA_TZFILE=/opt/oracle/myconfig/timezone_31.dat``.

The Oracle Database documentation contains more information about time zone
files, see `Choosing a Time Zone File
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-805AB986-DE12-4FEA-AF56-5AABCD2132DF>`__.

.. _otherinit:

Other cx_Oracle Initialization
==============================

The :meth:`cx_Oracle.init_oracle_client()` function allows ``driver_name`` and
``error_url`` parameters to be set.  These are useful for applications whose
end-users are not aware cx_Oracle is being used.  An example of setting the
parameters is:

.. code-block:: python

    import cx_Oracle
    import sys

    try:
        cx_Oracle.init_oracle_client(driver_name = "My Great App : 3.1.4",
            error_url: "https://example.com/MyInstallInstructions.html")
    except Exception as err:
        print("Whoops!")
        print(err);
        sys.exit(1);

The convention for ``driver_name`` is to separate the product name from the
product version by a colon and single blank characters.  The value will be shown
in Oracle Database views like ``V$SESSION_CONNECT_INFO``.  If this parameter is
not specified, then the value "cx_Oracle : *version*" is used.

The ``error_url`` string will be shown in the exception raised if
``init_oracle_client()`` cannot load the Oracle Client libraries.  This allows
applications that use cx_Oracle to refer users to application-specific
installation instructions.  If this value is not specified, then the
:ref:`installation` URL is used.
