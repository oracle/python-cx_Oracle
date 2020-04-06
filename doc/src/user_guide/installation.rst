.. _installation:

************************
cx_Oracle 8 Installation
************************

Overview
========

To use cx_Oracle 8 with Python and Oracle Database you need:

- Python 3.5 and higher.  Older versions of cx_Oracle may work with older
  versions of Python.

- Oracle client libraries. These can be from the free `Oracle Instant
  Client
  <https://www.oracle.com/database/technologies/instant-client.html>`__,
  or those included in Oracle Database if Python is on the same
  machine as the database.  Oracle client libraries versions 19, 18, 12,
  and 11.2 are supported on Linux, Windows and macOS.  Users have
  also reported success with other platforms.

- An Oracle Database. Oracle's standard client-server version
  interoperability allows cx_Oracle to connect to both older and newer
  databases.

If you are upgrading, review the :ref:`release notes <releasenotes>`.

Quick Start cx_Oracle Installation
==================================

- An installation of `Python <https://www.python.org/downloads>`__ is
  needed. Python 3.5 and higher are supported by cx_Oracle 8.

- Install cx_Oracle from `PyPI
  <https://pypi.org/project/cx-Oracle/>`__ with::

      python -m pip install cx_Oracle --upgrade

  Note: if a binary wheel package is not available for your platform,
  the source package will be downloaded instead. This will be compiled
  and the resulting binary installed.

  If you are behind a proxy, specify your proxy server::

      python -m pip install cx_Oracle --proxy=http://proxy.example.com:80 --upgrade

- Add Oracle 19, 18, 12 or 11.2 client libraries to your operating
  system library search path such as ``PATH`` on Windows or
  ``LD_LIBRARY_PATH`` on Linux.  On macOS move the files to ``~/lib``
  or ``/usr/local/lib``.

    - If your database is on a remote computer, then download and unzip the client
      libraries from the free `Oracle Instant Client
      <https://www.oracle.com/database/technologies/instant-client.html>`__
      "Basic" or "Basic Light" package for your operating system
      architecture.

      Instant Client on Windows requires an appropriate `Microsoft
      Windows Redistributables
      <https://oracle.github.io/odpi/doc/installation.html#windows>`__.
      On Linux, the ``libaio`` (sometimes called ``libaio1``) package
      is needed.

    - Alternatively use the client libraries already available in a
      locally installed database such as the free `Oracle XE
      <https://www.oracle.com/database/technologies/appdev/xe.html>`__
      release.

  Version 19, 18 and 12.2 client libraries can connect to Oracle Database 11.2
  or greater. Version 12.1 client libraries can connect to Oracle Database 10.2
  or greater. Version 11.2 client libraries can connect to Oracle Database 9.2
  or greater.

  The database abstraction layer in cx_Oracle is `ODPI-C
  <https://github.com/oracle/odpi>`__, which means that the `ODPI-C
  installation instructions
  <https://oracle.github.io/odpi/doc/installation.html>`__ can be useful
  to review.

- Create a script like the one below:

  .. code-block:: python

    # myscript.py

    from __future__ import print_function

    import cx_Oracle

    # Connect as user "hr" with password "welcome" to the "orclpdb1" service running on this computer.
    connection = cx_Oracle.connect("hr", "welcome", "localhost/orclpdb1")

    cursor = connection.cursor()
    cursor.execute("""
            SELECT first_name, last_name
            FROM employees
            WHERE department_id = :did AND employee_id > :eid""",
            did = 50,
            eid = 190)
    for fname, lname in cursor:
        print("Values:", fname, lname)

  Locate your Oracle Database username and password, and the database
  connection string.  The connection string is commonly of the format
  ``hostname/servicename``, using the hostname where the database is
  running, and the service name of the Oracle Database instance.

  Substitute your username, password and connection string in the
  code. Run the Python script, for example::

        python myscript.py

You can learn how to use cx_Oracle from the :ref:`API documentation <module>`
and `samples
<https://github.com/oracle/python-cx_Oracle/blob/master/samples>`__.

If you run into installation trouble, check out the section on `Troubleshooting`_.


Oracle Client and Oracle Database Interoperability
==================================================

cx_Oracle requires Oracle Client libraries.  The libraries provide the
necessary network connectivity to access an Oracle Database instance.
They also provide basic and advanced connection management and data
features to cx_Oracle.

The simplest way to get Oracle Client libraries is to install the free
`Oracle Instant Client
<https://www.oracle.com/database/technologies/instant-client.html>`__
"Basic" or "Basic Light" package.  The libraries are also available in
any Oracle Database installation or full Oracle Client installation.

Oracle's standard client-server network interoperability allows
connections between different versions of Oracle Client libraries and
Oracle Database.  For certified configurations see Oracle Support's
`Doc ID 207303.1
<https://support.oracle.com/epmos/faces/DocumentDisplay?id=207303.1>`__.
In summary, Oracle Client 19, 18 and 12.2 can connect to Oracle Database 11.2 or
greater. Oracle Client 12.1 can connect to Oracle Database 10.2 or
greater. Oracle Client 11.2 can connect to Oracle Database 9.2 or
greater.  The technical restrictions on creating connections may be
more flexible.  For example Oracle Client 12.2 can successfully
connect to Oracle Database 10.2.

cx_Oracle uses the shared library loading mechanism available on each
supported platform to load the Oracle Client libraries at runtime.  It
does not need to be rebuilt for different versions of the libraries.
Since a single cx_Oracle binary can use different client versions and
also access multiple database versions, it is important your
application is tested in your intended release environments.  Newer
Oracle clients support new features, such as the `oraaccess.xml
<http://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-9D12F489-EC02-46BE-8CD4-5AECED0E2BA2>`__ external configuration
file available with 12.1 or later clients, session pool improvements,
call timeouts with 18 or later clients, and `other enhancements
<http://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-D60519C3-406F-4588-8DA1-D475D5A3E1F6>`__.

The cx_Oracle function :func:`~cx_Oracle.clientversion()` can be used
to determine which Oracle Client version is in use and the attribute
:attr:`Connection.version` can be used to determine which Oracle
Database version a connection is accessing. These can then be used to
adjust application behavior accordingly. Attempts to use some Oracle
features that are not supported by a particular client/server
combination may result in runtime errors. These include:

    - when attempting to access attributes that are not supported by the
      current Oracle Client library you will get the error "ORA-24315: illegal
      attribute type"

    - when attempting to use implicit results with Oracle Client 11.2
      against Oracle Database 12c you will get the error "ORA-29481:
      Implicit results cannot be returned to client"

    - when attempting to get array DML row counts with Oracle Client
      11.2 you will get the error "DPI-1050: Oracle Client library must be at
      version 12.1 or higher"


Installing cx_Oracle on Linux
=============================

This section discusses the generic installation method on Linux.
Using Python and cx_Oracle RPM packages on Oracle Linux is discussed
in :ref:`oraclelinux`.

Install cx_Oracle
-----------------

The generic way to install cx_Oracle on Linux is to use Python's `Pip
<http://pip.readthedocs.io/en/latest/installing/>`__ package to
install cx_Oracle from `PyPI
<https://pypi.org/project/cx-Oracle/>`__::

    python -m pip install cx_Oracle --upgrade

If you are behind a proxy, specify your proxy server::

   python -m pip install cx_Oracle --proxy=http://proxy.example.com:80 --upgrade

This will download and install a pre-compiled binary `if one is
available <https://pypi.org/project/cx-Oracle/>`__ for your
architecture.  If a pre-compiled binary is not available, the source
will be downloaded, compiled, and the resulting binary installed.
Compiling cx_Oracle requires the `Python.h` header file.  If you are
using the default python package, this file is in the ``python-devel``
package or equivalent.

Install Oracle Client
---------------------

Using cx_Oracle requires Oracle Client libraries to be installed.
These provide the necessary network connectivity allowing cx_Oracle
to access an Oracle Database instance. Oracle Client versions 19, 18,
12 and 11.2 are supported.

    - If your database is on a remote computer, then download the free `Oracle
      Instant Client
      <https://www.oracle.com/database/technologies/instant-client.html>`__
      "Basic" or "Basic Light" package for your operating system
      architecture.  Use the RPM or ZIP packages, based on your
      preferences.

    - Alternatively use the client libraries already available in a
      locally installed database such as the free `Oracle XE
      <https://www.oracle.com/database/technologies/appdev/xe.html>`__
      release.

Oracle Instant Client Zip Files
+++++++++++++++++++++++++++++++

To use cx_Oracle with Oracle Instant Client zip files:

1. Download an Oracle 19, 18, 12, or 11.2 "Basic" or "Basic Light" zip file: `64-bit
   <https://www.oracle.com/database/technologies/instant-client/linux-x86-64-downloads.html>`__
   or `32-bit
   <https://www.oracle.com/database/technologies/instant-client/linux-x86-32-downloads.html>`__, matching your
   Python architecture.

2. Unzip the package into a single directory that is accessible to your
   application. For example::

       mkdir -p /opt/oracle
       cd /opt/oracle
       unzip instantclient-basic-linux.x64-19.3.0.0.0dbru.zip

3. Install the ``libaio`` package with sudo or as the root user. For example::

       sudo yum install libaio

   On some Linux distributions this package is called ``libaio1`` instead.

4. If there is no other Oracle software on the machine that will be
   impacted, permanently add Instant Client to the runtime link
   path. For example, with sudo or as the root user::

       sudo sh -c "echo /opt/oracle/instantclient_19_3 > /etc/ld.so.conf.d/oracle-instantclient.conf"
       sudo ldconfig

   Alternatively, set the environment variable ``LD_LIBRARY_PATH`` to
   the appropriate directory for the Instant Client version. For
   example::

       export LD_LIBRARY_PATH=/opt/oracle/instantclient_19_3:$LD_LIBRARY_PATH

5. If you intend to co-locate optional Oracle configuration files such
   as ``tnsnames.ora``, ``sqlnet.ora`` or ``oraaccess.xml`` with
   Instant Client, then put them in the ``network/admin``
   subdirectory.  With Instant Client 12.2 or earlier, create this
   manually.  For example::

       mkdir -p /opt/oracle/instantclient_12_2/network/admin

   This is the default Oracle configuration directory for executables
   linked with this Instant Client.

   Alternatively, Oracle configuration files can be put in another,
   accessible directory.  Then set the environment variable
   ``TNS_ADMIN`` to that directory name.

Oracle Instant Client RPMs
++++++++++++++++++++++++++

To use cx_Oracle with Oracle Instant Client RPMs:

1. Download an Oracle 19, 18, 12, or 11.2 "Basic" or "Basic Light" RPM: `64-bit
   <https://www.oracle.com/database/technologies/instant-client/linux-x86-64-downloads.html>`__
   or `32-bit
   <https://www.oracle.com/database/technologies/instant-client/linux-x86-32-downloads.html>`__, matching your
   Python architecture.

   Oracle's yum server has `Instant Client RPMs for Oracle Linux 7
   <http://yum.oracle.com/repo/OracleLinux/OL7/oracle/instantclient/x86_64/index.html>`__
   and `Instant Client RPMs for Oracle Linux 6
   <http://yum.oracle.com/repo/OracleLinux/OL6/oracle/instantclient/x86_64/index.html>`__
   that can be downloaded without needing a click-through.

2. Install the downloaded RPM with sudo or as the root user. For example::

       sudo yum install oracle-instantclient19.3-basic-19.3.0.0.0-1.x86_64.rpm

   Yum will automatically install required dependencies, such as ``libaio``.

3. For Instant Client 19, the system library search path is
   automatically configured during installation.

   For older versions, if there is no other Oracle software on the machine that will be
   impacted, permanently add Instant Client to the runtime link
   path. For example, with sudo or as the root user::

       sudo sh -c "echo /usr/lib/oracle/18.3/client64/lib > /etc/ld.so.conf.d/oracle-instantclient.conf"
       sudo ldconfig

   Alternatively, for version 18 and earlier, every shell running
   Python will need to have the environment variable
   ``LD_LIBRARY_PATH`` set to the appropriate directory for the
   Instant Client version. For example::

       export LD_LIBRARY_PATH=/usr/lib/oracle/18.3/client64/lib:$LD_LIBRARY_PATH

4. If you intend to co-locate optional Oracle configuration files such
   as ``tnsnames.ora``, ``sqlnet.ora`` or ``oraaccess.xml`` with
   Instant Client, then put them in the ``network/admin`` subdirectory
   under ``lib/``.  With Instant Client 12.2 or earlier, create this
   manually.  For example::

       sudo mkdir -p /usr/lib/oracle/12.2/client64/lib/network/admin

   This is the default Oracle configuration directory for executables
   linked with this Instant Client.

   Alternatively, Oracle configuration files can be put in another,
   accessible directory.  Then set the environment variable
   ``TNS_ADMIN`` to that directory name.

Local Database or Full Oracle Client
++++++++++++++++++++++++++++++++++++

cx_Oracle applications can use Oracle Client 19, 18, 12, or 11.2 libraries
from a local Oracle Database or full Oracle Client installation.

The libraries must be either 32-bit or 64-bit, matching your
Python architecture.

1. Set required Oracle environment variables by running the Oracle environment
   script. For example::

       source /usr/local/bin/oraenv

   For Oracle Database XE 11.2, run::

       source /u01/app/oracle/product/11.2.0/xe/bin/oracle_env.sh

2. Optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` can be placed in
   ``$ORACLE_HOME/network/admin``.

   Alternatively, Oracle configuration files can be put in another,
   accessible directory.  Then set the environment variable
   ``TNS_ADMIN`` to that directory name.


.. _oraclelinux:

Installing cx_Oracle RPMs on Oracle Linux
=========================================

Python and cx_Oracle RPM packages are available from the `Oracle Linux yum server
<http://yum.oracle.com/>`__.  Various versions of Python are easily installed.
Using the yum server makes it easy to keep up to date.

Installation instructions are at `Oracle Linux for Python
Developers <https://yum.oracle.com/oracle-linux-python.html>`__.

Installing cx_Oracle on Windows
===============================

Install cx_Oracle
-----------------

Use Python's `Pip <http://pip.readthedocs.io/en/latest/installing/>`__
package to install cx_Oracle from `PyPI
<https://pypi.org/project/cx-Oracle/>`__::

    python -m pip install cx_Oracle --upgrade

If you are behind a proxy, specify your proxy server::

   python -m pip install cx_Oracle --proxy=http://proxy.example.com:80 --upgrade

This will download and install a pre-compiled binary `if one is
available <https://pypi.org/project/cx-Oracle/>`__ for your
architecture.  If a pre-compiled binary is not available, the source
will be downloaded, compiled, and the resulting binary installed.

Install Oracle Client
---------------------

Using cx_Oracle requires Oracle Client libraries to be installed.
These provide the necessary network connectivity allowing cx_Oracle
to access an Oracle Database instance. Oracle Client versions 19, 18,
12 and 11.2 are supported.

    - If your database is on a remote computer, then download the free `Oracle
      Instant Client
      <https://www.oracle.com/database/technologies/instant-client.html>`__
      "Basic" or "Basic Light" package for your operating system
      architecture.

    - Alternatively use the client libraries already available in a
      locally installed database such as the free `Oracle XE
      <https://www.oracle.com/database/technologies/appdev/xe.html>`__
      release.


Oracle Instant Client Zip Files
+++++++++++++++++++++++++++++++

To use cx_Oracle with Oracle Instant Client zip files:

1. Download an Oracle 19, 18, 12, or 11.2 "Basic" or "Basic Light" zip
   file: `64-bit
   <https://www.oracle.com/database/technologies/instant-client/winx64-64-downloads.html>`__
   or `32-bit
   <https://www.oracle.com/database/technologies/instant-client/microsoft-windows-32-downloads.html>`__, matching your
   Python architecture.

   Windows 7 users: Note that Oracle 19c is not supported on Windows 7.

2. Unzip the package into a directory that is accessible to your
   application. For example unzip
   ``instantclient-basic-windows.x64-19.3.0.0.0dbru.zip`` to
   ``C:\oracle\instantclient_19_3``.

3. Add this directory to the ``PATH`` environment variable. For
   example, on Windows 7, update ``PATH`` in Control Panel -> System
   -> Advanced System Settings -> Advanced -> Environment Variables ->
   System Variables -> PATH.  The Instant Client directory must occur
   in ``PATH`` before any other Oracle directories.

   Restart any open command prompt windows.

   To avoid interfering with existing tools that require other Oracle Client
   versions, instead of updating the system-wide ``PATH`` variable you can set
   the value for each script.

   One way is to set it inside the application before cx_Oracle is used for the
   first time:

   .. code-block:: python

       import os
       os.environ['PATH'] = r'C:\instantclient_19_3' + os.pathsep + os.environ['PATH]

   Note this only works on Windows.

   Another alternative way to set ``PATH`` is to use a batch file that sets it
   before Python is executed, for example::

       REM mypy.bat
       SET PATH=C:\oracle\instantclient_19_3;%PATH%
       python %*

   Invoke this batch file every time you want to run Python.

   Alternatively use ``SET`` to change your ``PATH`` in each command
   prompt window before you run python.

4. Oracle Instant Client libraries require a Visual Studio redistributable with
   a 64-bit or 32-bit architecture to match Instant Client's architecture.
   Each Instant Client version requires a different redistributable version:

       - For Instant Client 19 install `VS 2017 <https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads>`__.
       - For Instant Client 18 or 12.2 install `VS 2013 <https://support.microsoft.com/en-us/kb/2977003#bookmark-vs2013>`__
       - For Instant Client 12.1 install `VS 2010 <https://support.microsoft.com/en-us/kb/2977003#bookmark-vs2010>`__
       - For Instant Client 11.2 install `VS 2005 64-bit <https://www.microsoft.com/en-us/download/details.aspx?id=18471>`__ or `VS 2005 32-bit <https://www.microsoft.com/en-ca/download/details.aspx?id=3387>`__

5. If you intend to co-locate optional Oracle configuration files such
   as ``tnsnames.ora``, ``sqlnet.ora`` or ``oraaccess.xml`` with
   Instant Client, then create a ``network\admin`` subdirectory, for example
   ``C:\oracle\instantclient_19_3\network\admin``.

   This is the default Oracle configuration directory for executables
   linked with this Instant Client.

   Alternatively, Oracle configuration files can be put in another,
   accessible directory.  Then set the environment variable
   ``TNS_ADMIN`` to that directory name.


Local Database or Full Oracle Client
++++++++++++++++++++++++++++++++++++

cx_Oracle applications can use Oracle Client 19, 18, 12, or 11.2
libraries libraries from a local Oracle Database or full Oracle
Client.

The Oracle libraries must be either 32-bit or 64-bit, matching your
Python architecture.

1. Set the environment variable ``PATH`` to include the path that
   contains OCI.dll, if it is not already set. For example, on Windows
   7, update ``PATH`` in Control Panel -> System -> Advanced System
   Settings -> Advanced -> Environment Variables -> System Variables
   -> PATH.

   Restart any open command prompt windows.

2. Optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` can be placed in the
   ``network\admin`` subdirectory of the Oracle Database software
   installation.

   Alternatively, Oracle configuration files can be put in another,
   accessible directory.  Then set the environment variable
   ``TNS_ADMIN`` to that directory name.


Installing cx_Oracle on macOS
=============================

Install Python
--------------

Make sure you are not using the bundled Python.  This has restricted
entitlements and will fail to load Oracle client libraries.  Instead
use `Homebrew <https://brew.sh>`__ or `Python.org
<https://www.python.org/downloads>`__.

Install cx_Oracle
-----------------

Use Python's `Pip <http://pip.readthedocs.io/en/latest/installing/>`__
package to install cx_Oracle from `PyPI
<https://pypi.org/project/cx-Oracle/>`__::

    python -m pip install cx_Oracle --upgrade

If you are behind a proxy, specify your proxy server::

   python -m pip install cx_Oracle --proxy=http://proxy.example.com:80 --upgrade

The ``--user`` option may also be useful, if you don't have permission to write
to ``/usr``.

The source will be downloaded, compiled, and the resulting binary
installed.


Install Oracle Instant Client
-----------------------------

cx_Oracle requires Oracle Client libraries, which are found in Oracle
Instant Client for macOS. These provide the necessary network
connectivity allowing cx_Oracle to access an Oracle Database
instance. Oracle Client versions 19, 18, 12 and 11.2 are supported.

To use cx_Oracle with Oracle Instant Client zip files:

1. Download the Oracle 19, 18, 12 or 11.2 "Basic" or "Basic Light" zip file from `here
   <https://www.oracle.com/database/technologies/instant-client/macos-intel-x86-downloads.html>`__.
   Choose either a 64-bit or 32-bit package, matching your
   Python architecture.

2. Unzip the package into a single directory that is accessible to your
   application. For example, in Terminal you could unzip in your home directory::

       cd ~
       unzip instantclient-basic-macos.x64-19.3.0.0.0dbru.zip

   This will create a directory ``/Users/yourname/instantclient_19_3``.

3. Add a link to ``$HOME/lib`` or ``/usr/local/lib`` to enable applications to
   find Instant Client. If the ``lib`` sub-directory does not exist, you can
   create it. For example::

       mkdir ~/lib
       ln -s ~/instantclient_19_3/libclntsh.dylib ~/lib/

   If you now run ``ls -l ~/lib/libclntsh.dylib`` you will see something like::

       lrwxr-xr-x  1 yourname  staff  48 12 Nov 15:04 /Users/yourname/lib/libclntsh.dylib -> /Users/yourname/instantclient_19_3/libclntsh.dylib

   Alternatively, copy the required OCI libraries. For example::

        mkdir ~/lib
        cp ~/instantclient_19_3/{libclntsh.dylib.19.1,libclntshcore.dylib.19.1,libnnz19.dylib,libociei.dylib} ~/lib/

   For Instant Client 11.2, the OCI libraries must be copied. For example::

        mkdir ~/lib
        cp ~/instantclient_11_2/{libclntsh.dylib.11.1,libnnz11.dylib,libociei.dylib} ~/lib/

4. If you intend to co-locate optional Oracle configuration files such
   as ``tnsnames.ora``, ``sqlnet.ora`` or ``oraaccess.xml`` with
   Instant Client, then create a ``network/admin`` subdirectory, if it
   does not already exist.  For example::

       mkdir -p ~/instantclient_12_2/network/admin

   This is the default Oracle configuration directory for executables
   linked with this Instant Client.

   Alternatively, Oracle configuration files can be put in another,
   accessible directory.  Then set the environment variable
   ``TNS_ADMIN`` to that directory name.

Installing cx_Oracle without Internet Access
============================================

To install cx_Oracle on a computer that is not connected to the
internet, download the appropriate cx_Oracle file from `PyPI
<https://pypi.org/project/cx-Oracle/#files>`__.  Transfer this file to
the offline computer and install it with::

    python -m pip install "<file_name>"

Then follow the general cx_Oracle platform installation instructions
to install Oracle client libraries.

Install Using GitHub
====================

In order to install using the source on GitHub, use the following commands::

    git clone https://github.com/oracle/python-cx_Oracle.git cx_Oracle
    cd cx_Oracle
    git submodule init
    git submodule update
    python setup.py install

Note that if you download a source zip file directly from GitHub then
you will also need to download an `ODPI-C
<https://github.com/oracle/odpi>`__ source zip file and extract it
inside the directory called "odpi".

cx_Oracle source code is also available from oss.oracle.com.  This can
be cloned with::

    git clone git://oss.oracle.com/git/oracle/python-cx_Oracle.git cx_Oracle
    cd cx_Oracle
    git submodule init
    git submodule update


Install Using Source from PyPI
==============================

The source package can be downloaded manually from
`PyPI <https://pypi.org/project/cx-Oracle/>`__ and extracted, after
which the following commands should be run::

    python setup.py build
    python setup.py install


Upgrading from Older Versions
=============================

Review the :ref:`release notes <releasenotes>` for deprecations and modify any
affected code.

If you are upgrading from cx_Oracle 7 note these changes:

    - Any uses of ``type(var)`` need to be changed to ``var.type``.

    - Any uses of ``var.type is not None`` need to be changed to
      ``isinstance(var.type, cx_Oracle.ObjectType)``

    - Note that ``TIMESTAMP WITH TIME ZONE`` columns will now be reported as
      :data:`cx_Oracle.DB_TYPE_TIMESTAMP_TZ` instead of
      :data:`cx_Oracle.TIMESTAMP` in :data:`Cursor.description`.

    - Note that ``TIMESTAMP WITH LOCAL TIME ZONE`` columns will now be reported
      as :data:`cx_Oracle.DB_TYPE_TIMESTAMP_LTZ` instead of
      :data:`cx_Oracle.TIMESTAMP` in :data:`Cursor.description`.

    - Note that ``BINARY_FLOAT`` columns will now be reported as
      :data:`cx_Oracle.DB_TYPE_BINARY_FLOAT` instead of
      :data:`cx_Oracle.NATIVE_DOUBLE` in :data:`Cursor.description`.

If you are upgrading from cx_Oracle 5 note these installation changes:

    - When using Oracle Instant Client, you should not set ``ORACLE_HOME``.

    - On Linux, cx_Oracle 6 and higher no longer uses Instant Client RPMs
      automatically.  You must set ``LD_LIBRARY_PATH`` or use ``ldconfig`` to
      locate the Oracle client library.

    - PyPI no longer allows Windows installers or Linux RPMs to be
      hosted.  Use the supplied cx_Oracle Wheels instead, or use RPMs
      from Oracle, see :ref:`oraclelinux`.


Installing cx_Oracle 5.3
========================

If you require cx_Oracle 5.3, download a Windows installer from `PyPI
<https://pypi.org/project/cx-Oracle/>`__ or use ``python -m pip
install cx-oracle==5.3`` to install from source.

Very old versions of cx_Oracle can be found in the files section at
`SourceForce <https://sourceforge.net/projects/cx-oracle/files/>`__.


Troubleshooting
===============

If installation fails:

    - Use option ``-v`` with pip. Review your output and logs. Try to install
      using a different method. **Google anything that looks like an error.**
      Try some potential solutions.

    - Was there a network connection error?  Do you need to set the
      environment variables ``http_proxy`` and/or ``https_proxy``?  Or
      try ``pip install --proxy=http://proxy.example.com:80 cx_Oracle
      --upgrade``?

    - If upgrading gave no errors but the old version is still
      installed, try ``pip install cx_Oracle --upgrade
      --force-reinstall``

    - If you do not have access to modify your system version of
      Python, can you use ``pip install cx_Oracle --upgrade --user``
      or venv?

    - Do you get the error "``No module named pip``"? The pip module is builtin
      to Python but is sometimes removed by the OS. Use the venv module
      (builtin to Python 3.x) or virtualenv module instead.

    - Do you get the error "``fatal error: dpi.h: No such file or directory``"
      when building from source code? Ensure that your source installation has
      a subdirectory called "odpi" containing files. If missing, review the
      section on `Install Using GitHub`_.

If using cx_Oracle fails:

    - Do you get the error "``DPI-1047: Oracle Client library cannot be
      loaded``"?

      - Check that Python, cx_Oracle and your Oracle Client libraries
        are all 64-bit or all 32-bit.  The ``DPI-1047`` message will
        tell you whether the 64-bit or 32-bit Oracle Client is needed
        for your Python.
      - On Windows, restart your command prompt and use ``set PATH``
        to check the environment variable has the correct Oracle
        Client listed before any other Oracle directories.
      - On Windows, use the ``DIR`` command on the directory set in
        ``PATH``. Verify that ``OCI.DLL`` exists there.
      - On Windows, check that the correct `Windows Redistributables
        <https://oracle.github.io/odpi/doc/installation.html#windows>`__ have
        been installed.
      - On Linux, check the ``LD_LIBRARY_PATH`` environment variable
        contains the Oracle Client library directory.
      - On macOS, make sure Oracle Instant Client is in ``~/lib`` or
        ``/usr/local/lib`` and that you are not using the bundled Python (use
        `Homebrew <https://brew.sh>`__ or `Python.org
        <https://www.python.org/downloads>`__ instead).

    - If you got "``DPI-1072: the Oracle Client library version is
      unsupported``", then review the installation requirements.  cx_Oracle
      needs Oracle client libraries 11.2 or later.  Note that version 19 is not
      supported on Windows 7.  Similar steps shown above for ``DPI-1047`` may
      help.

    - If you have multiple versions of Python installed, make sure you are
      using the correct python and pip (or python3 and pip3) executables.
