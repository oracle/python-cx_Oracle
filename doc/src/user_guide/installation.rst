.. _installation:

************************
cx_Oracle 8 Installation
************************

Overview
========

To use cx_Oracle 8 with Python and Oracle Database you need:

- Python 3.5 and higher. Older versions of cx_Oracle may work with older
  versions of Python.

- Oracle Client libraries. These can be from the free `Oracle Instant Client
  <https://www.oracle.com/database/technologies/instant-client.html>`__, from a
  full Oracle Client installation, or from those included in Oracle Database if
  Python is on the same machine as the database.  Oracle client libraries
  versions 21, 19, 18, 12, and 11.2 are supported where available on Linux,
  Windows and macOS (Intel x86).  Users have also reported success with other
  platforms.  Use the latest client possible: Oracle's standard client-server
  version interoperability allows connection to both older and newer databases.

- An Oracle Database, either local or remote.

The cx_Oracle module loads Oracle Client libraries which communicate
over Oracle Net to an existing database.  Oracle Net is not a separate
product: it is how the Oracle Client and Oracle Database communicate.

.. figure:: /images/cx_Oracle_arch.png

   cx_Oracle Architecture


Quick Start cx_Oracle Installation
==================================

The `Quick Start: Developing Python Applications for Oracle Database
<https://www.oracle.com/database/technologies/appdev/python/quickstartpythononprem.html>`__
and `Quick Start: Developing Python Applications for Oracle Autonomous Database
<https://www.oracle.com/database/technologies/appdev/python/quickstartpython.html>`__
instructions have steps for Windows, Linux, and macOS.

Alternatively you can:

- Install `Python <https://www.python.org/downloads>`__ 3, if not already
  available.  On macOS you must always install your own Python.

  Python 3.5 and higher are supported by cx_Oracle 8.  If you use Python 2,
  then the older cx_Oracle 7.3 will install.

- Install cx_Oracle from `PyPI
  <https://pypi.org/project/cx-Oracle/>`__ with:

  .. code-block:: shell

      python -m pip install cx_Oracle --upgrade

  Note: if a binary wheel package is not available for your platform,
  the source package will be downloaded instead. This will be compiled
  and the resulting binary installed.

  The ``--user`` option may be useful, if you don't have permission to write to
  system directories:

  .. code-block:: shell

      python -m pip install cx_Oracle --upgrade --user

  If you are behind a proxy, add a proxy server to the command, for example add
  ``--proxy=http://proxy.example.com:80``

- Add Oracle 21, 19, 18, 12 or 11.2 client libraries to your operating system
  library search path such as ``PATH`` on Windows or ``LD_LIBRARY_PATH`` on
  Linux.  On macOS use :meth:`~cx_Oracle.init_oracle_client()` in your
  application to pass the Oracle Client directory name, see
  :ref:`usinginitoracleclient`.  This is also usable on Windows.

  To get the libraries:

    - If your database is on a remote computer, then download and unzip the client
      libraries from the free `Oracle Instant Client
      <https://www.oracle.com/database/technologies/instant-client.html>`__
      "Basic" or "Basic Light" package for your operating system
      architecture.

      Instant Client on Windows requires an appropriate Microsoft Windows
      Redistributables, see :ref:`wininstall`.  On Linux, the ``libaio``
      (sometimes called ``libaio1``) package is needed.  Oracle Linux 8 also
      needs the ``libnsl`` package.

    - Alternatively, use the client libraries already available in a
      locally installed database such as the free `Oracle Database
      Express Edition ("XE")
      <https://www.oracle.com/database/technologies/appdev/xe.html>`__
      release.

  Version 21 client libraries can connect to Oracle Database 12.1 or greater.
  Version 19, 18 and 12.2 client libraries can connect to Oracle Database 11.2
  or greater. Version 12.1 client libraries can connect to Oracle Database 10.2
  or greater. Version 11.2 client libraries can connect to Oracle Database 9.2
  or greater.

- Create a script like the one below:

  .. code-block:: python

    # myscript.py

    import cx_Oracle

    # Connect as user "hr" with password "welcome" to the "orclpdb1" service running on this computer.
    connection = cx_Oracle.connect(user="hr", password="welcome",
                                   dsn="localhost/orclpdb1")

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
  running, and using the service name of the Oracle Database instance.

  Substitute your username, password and connection string in the
  code. Run the Python script, for example::

        python myscript.py

You can learn how to use cx_Oracle from the :ref:`API documentation <module>`
and `samples
<https://github.com/oracle/python-cx_Oracle/blob/main/samples>`__.

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
In summary, Oracle Client 21 can connect to Oracle Database 12.1 or greater.
Oracle Client 19, 18 and 12.2 can connect to Oracle Database 11.2 or
greater. Oracle Client 12.1 can connect to Oracle Database 10.2 or
greater. Oracle Client 11.2 can connect to Oracle Database 9.2 or greater.  The
technical restrictions on creating connections may be more flexible.  For
example Oracle Client 12.2 can successfully connect to Oracle Database 10.2.

cx_Oracle uses the shared library loading mechanism available on each
supported platform to load the Oracle Client libraries at runtime.  It
does not need to be rebuilt for different versions of the libraries.
Since a single cx_Oracle binary can use different client versions and
also access multiple database versions, it is important your
application is tested in your intended release environments.  Newer
Oracle clients support new features, such as the `oraaccess.xml
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-9D12F489-EC02-46BE-8CD4-5AECED0E2BA2>`__ external configuration
file available with 12.1 or later clients, session pool improvements,
improved high availability features, call timeouts, and `other enhancements
<https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-D60519C3-406F-4588-8DA1-D475D5A3E1F6>`__.

The cx_Oracle function :func:`~cx_Oracle.clientversion()` can be used to
determine which Oracle Client version is in use. The attribute
:attr:`Connection.version` can be used to determine which Oracle Database
version a connection is accessing. These can then be used to adjust application
behavior accordingly. Attempts to use Oracle features that are not supported by
a particular client/server library combination will result in runtime errors.

Installing cx_Oracle on Linux
=============================

This section discusses the generic installation methods on Linux.  To use Python
and cx_Oracle RPM packages from yum on Oracle Linux, see :ref:`oraclelinux`.

Install cx_Oracle
-----------------

The generic way to install cx_Oracle on Linux is to use Python's `Pip
<https://pip.readthedocs.io/en/latest/installing/>`__ package to
install cx_Oracle from `PyPI
<https://pypi.org/project/cx-Oracle/>`__:

.. code-block:: shell

    python -m pip install cx_Oracle --upgrade

The ``--user`` option may be useful, if you don't have permission to write to
system directories:

.. code-block:: shell

    python -m pip install cx_Oracle --upgrade --user

If you are behind a proxy, add a proxy server to the command, for example add
``--proxy=http://proxy.example.com:80``

This will download and install a pre-compiled binary `if one is
available <https://pypi.org/project/cx-Oracle/>`__ for your
architecture.  If a pre-compiled binary is not available, the source
will be downloaded, compiled, and the resulting binary installed.
Compiling cx_Oracle requires the ``Python.h`` header file.  If you are
using the default ``python`` package, this file is in the ``python-devel``
package or equivalent.

Install Oracle Client
---------------------

Using cx_Oracle requires Oracle Client libraries to be installed.
These provide the necessary network connectivity allowing cx_Oracle
to access an Oracle Database instance.

    - If your database is on a remote computer, then download the free `Oracle
      Instant Client
      <https://www.oracle.com/database/technologies/instant-client.html>`__
      "Basic" or "Basic Light" package for your operating system
      architecture.  Use the RPM or ZIP packages, based on your
      preferences.

    - Alternatively, use the client libraries already available in a
      locally installed database such as the free `Oracle Database
      Express Edition ("XE")
      <https://www.oracle.com/database/technologies/appdev/xe.html>`__
      release.

Oracle Instant Client Zip Files
+++++++++++++++++++++++++++++++

To use cx_Oracle with Oracle Instant Client zip files:

1. Download an Oracle 21, 19, 18, 12, or 11.2 "Basic" or "Basic Light" zip file
   matching your Python 64-bit or 32-bit architecture:

   - `x86-64 64-bit <https://www.oracle.com/database/technologies/instant-client/linux-x86-64-downloads.html>`__
   - `x86 32-bit <https://www.oracle.com/database/technologies/instant-client/linux-x86-32-downloads.html>`__
   - `ARM (aarch64) 64-bit <https://www.oracle.com/database/technologies/instant-client/linux-arm-aarch64-downloads.html>`__

   The latest version is recommended. Oracle Instant Client 21 will connect to
   Oracle Database 12.1 or later.

2. Unzip the package into a single directory that is accessible to your
   application. For example:

   .. code-block:: shell

       mkdir -p /opt/oracle
       cd /opt/oracle
       unzip instantclient-basic-linux.x64-21.1.0.0.0.zip

3. Install the ``libaio`` package with sudo or as the root user. For example::

       sudo yum install libaio

   On some Linux distributions this package is called ``libaio1`` instead.

   On recent Linux versions such as Oracle Linux 8, you may also need to
   install the ``libnsl`` package when using Oracle Instant Client 19.

4. If there is no other Oracle software on the machine that will be
   impacted, permanently add Instant Client to the runtime link
   path. For example, with sudo or as the root user:

   .. code-block:: shell

       sudo sh -c "echo /opt/oracle/instantclient_21_1 > /etc/ld.so.conf.d/oracle-instantclient.conf"
       sudo ldconfig

   Alternatively, set the environment variable ``LD_LIBRARY_PATH`` to
   the appropriate directory for the Instant Client version. For
   example::

       export LD_LIBRARY_PATH=/opt/oracle/instantclient_21_1:$LD_LIBRARY_PATH

5. If you use optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` with Instant Client, then put the files
   in an accessible directory, for example in
   ``/opt/oracle/your_config_dir``. Then use:

   .. code-block:: python

       import cx_Oracle
       cx_Oracle.init_oracle_client(config_dir="/home/your_username/oracle/your_config_dir")

   Or set the environment variable ``TNS_ADMIN`` to that directory name.

   Alternatively, put the files in the ``network/admin`` subdirectory of Instant
   Client, for example in ``/opt/oracle/instantclient_21_1/network/admin``.
   This is the default Oracle configuration directory for executables linked
   with this Instant Client.

Oracle Instant Client RPMs
++++++++++++++++++++++++++

To use cx_Oracle with Oracle Instant Client RPMs:

1. Download an Oracle 21,19, 18, 12, or 11.2 "Basic" or "Basic Light" RPM
   matching your Python architecture:

   - `x86-64 64-bit <https://www.oracle.com/database/technologies/instant-client/linux-x86-64-downloads.html>`__
   - `x86 32-bit <https://www.oracle.com/database/technologies/instant-client/linux-x86-32-downloads.html>`__
   - `ARM (aarch64) 64-bit <https://www.oracle.com/database/technologies/instant-client/linux-arm-aarch64-downloads.html>`__

   Oracle's yum server has convenient repositories:

   - `Instant Client 21 RPMs for Oracle Linux x86-64 8 <https://yum.oracle.com/repo/OracleLinux/OL8/oracle/instantclient21/x86_64/index.html>`__, `Older Instant Client RPMs for Oracle Linux x86-64 8 <https://yum.oracle.com/repo/OracleLinux/OL8/oracle/instantclient/x86_64/index.html>`__
   - `Instant Client 21 RPMs for Oracle Linux x86-64 7 <https://yum.oracle.com/repo/OracleLinux/OL7/oracle/instantclient21/x86_64/index.html>`__, `Older Instant Client RPMs for Oracle Linux x86-64 7 <https://yum.oracle.com/repo/OracleLinux/OL7/oracle/instantclient/x86_64/index.html>`__
   - `Instant Client RPMs for Oracle Linux x86-64 6 <https://yum.oracle.com/repo/OracleLinux/OL6/oracle/instantclient/x86_64/index.html>`__
   - `Instant Client RPMs for Oracle Linux ARM (aarch64) 8 <https://yum.oracle.com/repo/OracleLinux/OL8/oracle/instantclient/aarch64/index.html>`__
   - `Instant Client RPMs for Oracle Linux ARM (aarch64) 7 <https://yum.oracle.com/repo/OracleLinux/OL7/oracle/instantclient/aarch64/index.html>`__

   The latest version is recommended.  Oracle Instant Client 21 will connect to
   Oracle Database 12.1 or later.

2. Install the downloaded RPM with sudo or as the root user. For example:

   .. code-block:: shell

       sudo yum install oracle-instantclient-basic-21.1.0.0.0-1.x86_64.rpm

   Yum will automatically install required dependencies, such as ``libaio``.

   On recent Linux versions, such as Oracle Linux 8, you may need to manually
   install the ``libnsl`` package when using Oracle Instant Client 19.

3. For Instant Client 19, or later, the system library search path is
   automatically configured during installation.

   For older versions, if there is no other Oracle software on the machine that will be
   impacted, permanently add Instant Client to the runtime link
   path. For example, with sudo or as the root user:

   .. code-block:: shell

       sudo sh -c "echo /usr/lib/oracle/18.5/client64/lib > /etc/ld.so.conf.d/oracle-instantclient.conf"
       sudo ldconfig

   Alternatively, for version 18 and earlier, every shell running
   Python will need to have the environment variable
   ``LD_LIBRARY_PATH`` set to the appropriate directory for the
   Instant Client version. For example::

       export LD_LIBRARY_PATH=/usr/lib/oracle/18.5/client64/lib:$LD_LIBRARY_PATH

4. If you use optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` with Instant Client, then put the files
   in an accessible directory, for example in
   ``/opt/oracle/your_config_dir``. Then use:

   .. code-block:: python

       import cx_Oracle
       cx_Oracle.init_oracle_client(config_dir="/opt/oracle/your_config_dir")

   Or set the environment variable ``TNS_ADMIN`` to that directory name.

   Alternatively, put the files in the ``network/admin`` subdirectory of Instant
   Client, for example in ``/usr/lib/oracle/21/client64/lib/network/admin``.
   This is the default Oracle configuration directory for executables linked
   with this Instant Client.

Local Database or Full Oracle Client
++++++++++++++++++++++++++++++++++++

cx_Oracle applications can use Oracle Client 21, 19, 18, 12, or 11.2 libraries
from a local Oracle Database or full Oracle Client installation.

The libraries must be either 32-bit or 64-bit, matching your
Python architecture.

1. Set required Oracle environment variables by running the Oracle environment
   script. For example:

   .. code-block:: shell

       source /usr/local/bin/oraenv

   For Oracle Database Express Edition ("XE") 11.2, run:

   .. code-block:: shell

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
<https://yum.oracle.com/>`__.  Various versions of Python are easily installed.
Using the yum server makes it easy to keep up to date.

Installation instructions are at `Oracle Linux for Python
Developers <https://yum.oracle.com/oracle-linux-python.html>`__.

.. _wininstall:

Installing cx_Oracle on Windows
===============================

Install cx_Oracle
-----------------

Use Python's `Pip <https://pip.readthedocs.io/en/latest/installing/>`__
package to install cx_Oracle from `PyPI
<https://pypi.org/project/cx-Oracle/>`__::

    python -m pip install cx_Oracle --upgrade

If you are behind a proxy, specify your proxy server:

.. code-block:: shell

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

    - Alternatively, use the client libraries already available in a
      locally installed database such as the free `Oracle Database
      Express Edition ("XE")
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

   The latest version is recommended.  Oracle Instant Client 19 will
   connect to Oracle Database 11.2 or later.

   Windows 7 users: Note that Oracle 19c is not supported on Windows 7.

2. Unzip the package into a directory that is accessible to your
   application. For example unzip
   ``instantclient-basic-windows.x64-19.11.0.0.0dbru.zip`` to
   ``C:\oracle\instantclient_19_11``.

3. Oracle Instant Client libraries require a Visual Studio redistributable with
   a 64-bit or 32-bit architecture to match Instant Client's architecture.
   Each Instant Client version requires a different redistributable version:

       - For Instant Client 19 install `VS 2017 <https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads>`__.
       - For Instant Client 18 or 12.2 install `VS 2013 <https://support.microsoft.com/en-us/kb/2977003#bookmark-vs2013>`__
       - For Instant Client 12.1 install `VS 2010 <https://support.microsoft.com/en-us/kb/2977003#bookmark-vs2010>`__
       - For Instant Client 11.2 install `VS 2005 64-bit <https://www.microsoft.com/en-us/download/details.aspx?id=18471>`__ or `VS 2005 32-bit <https://www.microsoft.com/en-ca/download/details.aspx?id=3387>`__

Configure Oracle Instant Client
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. There are several alternative ways to tell cx_Oracle where your Oracle Client
   libraries are, see :ref:`initialization`.

   * With Oracle Instant Client you can use :meth:`~cx_Oracle.init_oracle_client()`
     in your application, for example:

     .. code-block:: python

         import cx_Oracle
         cx_Oracle.init_oracle_client(lib_dir=r"C:\oracle\instantclient_19_11")

     Note a 'raw' string is used because backslashes occur in the path.

   * Alternatively, add the Oracle Instant Client directory to the ``PATH``
     environment variable.  The directory must occur in ``PATH`` before any
     other Oracle directories.  Restart any open command prompt windows.

   * Another way to set ``PATH`` is to use a batch file that sets it before Python
     is executed, for example::

         REM mypy.bat
         SET PATH=C:\oracle\instantclient_19_9;%PATH%
         python %*

     Invoke this batch file every time you want to run Python.

2. If you use optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` with Instant Client, then put the files
   in an accessible directory, for example in
   ``C:\oracle\your_config_dir``. Then use:

   .. code-block:: python

       import cx_Oracle
       cx_Oracle.init_oracle_client(lib_dir=r"C:\oracle\instantclient_19_11",
                                    config_dir=r"C:\oracle\your_config_dir")

   Or set the environment variable ``TNS_ADMIN`` to that directory name.

   Alternatively, put the files in a ``network\admin`` subdirectory of
   Instant Client, for example in
   ``C:\oracle\instantclient_19_11\network\admin``.  This is the default
   Oracle configuration directory for executables linked with this
   Instant Client.


Local Database or Full Oracle Client
++++++++++++++++++++++++++++++++++++

cx_Oracle applications can use Oracle Client 19, 18, 12, or 11.2
libraries libraries from a local Oracle Database or full Oracle
Client.

The Oracle libraries must be either 32-bit or 64-bit, matching your
Python architecture.

1. Set the environment variable ``PATH`` to include the path that contains
   ``OCI.DLL``, if it is not already set.

   Restart any open command prompt windows.

2. Optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` can be placed in the
   ``network\admin`` subdirectory of the Oracle Database software
   installation.

   Alternatively, pass ``config_dir`` to :meth:`~cx_Oracle.init_oracle_client()`
   as shown in the previous section, or set ``TNS_ADMIN`` to the directory name.

Installing cx_Oracle on macOS (Intel x86)
=========================================

Install Python
--------------

Make sure you are not using the bundled Python.  This has restricted
entitlements and will fail to load Oracle client libraries.  Instead use
`Homebrew <https://brew.sh>`__ or `Python.org
<https://www.python.org/downloads>`__.

A C compiler is needed, for example Xcode and its command line tools.

Install cx_Oracle
-----------------

Use Python's `Pip <https://pip.readthedocs.io/en/latest/installing/>`__
package to install cx_Oracle from `PyPI
<https://pypi.org/project/cx-Oracle/>`__:

.. code-block:: shell

    export ARCHFLAGS="-arch x86_64"
    python -m pip install cx_Oracle --upgrade

The ``--user`` option may be useful, if you don't have permission to write to
system directories:

.. code-block:: shell

    python -m pip install cx_Oracle --upgrade --user

If you are behind a proxy, add a proxy server to the command, for example add
``--proxy=http://proxy.example.com:80``

The source will be downloaded, compiled, and the resulting binary
installed.

Install Oracle Instant Client
-----------------------------

Oracle Instant Client provides the network connectivity for accessing Oracle
Database.

Manual Installation
+++++++++++++++++++

* Download the **Basic** 64-bit DMG from `Oracle
  <https://www.oracle.com/database/technologies/instant-client/macos-intel-x86-downloads.html>`__.

* In Finder, double click on the DMG to mount it.

* Open a terminal window and run the install script in the mounted package, for example:

  .. code-block:: shell

    /Volumes/instantclient-basic-macos.x64-19.8.0.0.0dbru/install_ic.sh

  This copies the contents to ``$HOME/Downloads/instantclient_19_8``.
  Applications may not have access to the ``Downloads`` directory, so you
  should move Instant Client somewhere convenient.

* In Finder, eject the mounted Instant Client package.

If you have multiple Instant Client DMG packages mounted, you only need to run
``install_ic.sh`` once.  It will copy all mounted Instant Client DMG packages at
the same time.

Scripted Installation
+++++++++++++++++++++

Instant Client installation can alternatively be scripted, for example:

.. code-block:: shell

    cd $HOME/Downloads
    curl -O https://download.oracle.com/otn_software/mac/instantclient/198000/instantclient-basic-macos.x64-19.8.0.0.0dbru.dmg
    hdiutil mount instantclient-basic-macos.x64-19.8.0.0.0dbru.dmg
    /Volumes/instantclient-basic-macos.x64-19.8.0.0.0dbru/install_ic.sh
    hdiutil unmount /Volumes/instantclient-basic-macos.x64-19.8.0.0.0dbru

The Instant Client directory will be ``$HOME/Downloads/instantclient_19_8``.
Applications may not have access to the ``Downloads`` directory, so you should
move Instant Client somewhere convenient.


Configure Oracle Instant Client
-------------------------------

1. Call :meth:`~cx_Oracle.init_oracle_client()` once in your application:

   .. code-block:: python

        import cx_Oracle
        cx_Oracle.init_oracle_client(lib_dir="/Users/your_username/Downloads/instantclient_19_8")

2. If you use optional Oracle configuration files such as ``tnsnames.ora``,
   ``sqlnet.ora`` or ``oraaccess.xml`` with Oracle Instant Client, then put the
   files in an accessible directory, for example in
   ``/Users/your_username/oracle/your_config_dir``. Then use:

   .. code-block:: python

       import cx_Oracle
       cx_Oracle.init_oracle_client(lib_dir="/Users/your_username/Downloads/instantclient_19_8",
                                    config_dir="/Users/your_username/oracle/your_config_dir")

   Or set the environment variable ``TNS_ADMIN`` to that directory name.

   Alternatively, put the files in the ``network/admin`` subdirectory of Oracle
   Instant Client, for example in
   ``/Users/your_username/Downloads/instantclient_19_8/network/admin``.  This is the
   default Oracle configuration directory for executables linked with this
   Instant Client.

Linux Containers
================

Sample Dockerfiles are on `GitHub
<https://github.com/oracle/docker-images/tree/main/OracleLinuxDevelopers>`__.

Pre-built images for Python and cx_Oracle are in the `GitHub Container Registry
<https://github.com/orgs/oracle/packages>`__.  These are easily used. For
example, to pull an Oracle Linux 8 image with Python 3.6 and cx_Oracle,
execute::

    docker pull ghcr.io/oracle/oraclelinux7-python:3.6-oracledb


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

cx_Oracle source code is also available from opensource.oracle.com.  This can
be cloned with::

    git clone git://opensource.oracle.com/git/oracle/python-cx_Oracle.git cx_Oracle
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

Review the :ref:`release notes <releasenotes>` and :ref:`Deprecations
<deprecations>` for changes.  Modify affected code.

If you are upgrading from cx_Oracle 7 note these changes:

    - The default character set used by cx_Oracle 8 is now "UTF-8". Also, the
      character set component of the ``NLS_LANG`` environment variable is
      ignored.  If you need to change the character set, then pass ``encoding``
      and ``nendcoding`` parameters when creating a connection or connection
      pool.  See :ref:`globalization`.

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

.. _python2:

Installing cx_Oracle in Python 2
================================

cx_Oracle 7.3 was the last version with support for Python 2.

If you install cx_Oracle in Python 2 using the commands provided above, then
cx_Oracle 7.3 will be installed.  This is equivalent to using a command like::

    python -m pip install cx_Oracle==7.3 --upgrade --user

For other installation options such as installing through a proxy, see
instructions above.  Make sure the Oracle Client libraries are in the system
library search path because cx_Oracle 7 does not support the
:meth:`cx_Oracle.init_oracle_client()` method and does not support loading the
Oracle Client libraries from the directory containing the cx_Oracle module
binary.

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

      - On Windows and macOS, try using :meth:`~cx_Oracle.init_oracle_client()`.
        See :ref:`usinginitoracleclient`.

      - Check that Python and your Oracle Client libraries are both 64-bit, or
        both 32-bit.  The ``DPI-1047`` message will tell you whether the 64-bit
        or 32-bit Oracle Client is needed for your Python.

      - Set the environment variable ``DPI_DEBUG_LEVEL`` to 64 and restart
        cx_Oracle.  The trace messages will show how and where cx_Oracle is
        looking for the Oracle Client libraries.

        At a Windows command prompt, this could be done with::

            set DPI_DEBUG_LEVEL=64

        On Linux and macOS, you might use::

              export DPI_DEBUG_LEVEL=64

      - On Windows, if you used :meth:`~cx_Oracle.init_oracle_client()` and have
        a full database installation, make sure this database is the `currently
        configured database
        <https://www.oracle.com/pls/topic/lookup?ctx=dblatest&id=GUID-33D575DD-47FF-42B1-A82F-049D3F2A8791>`__.

      - On Windows, if you are not using
        :meth:`~cx_Oracle.init_oracle_client()`, then restart your command prompt
        and use ``set PATH`` to check the environment variable has the correct
        Oracle Client listed before any other Oracle directories.

      - On Windows, use the ``DIR`` command to verify that ``OCI.DLL`` exists in
        the directory passed to ``init_oracle_client()`` or set in ``PATH``.

      - On Windows, check that the correct `Windows Redistributables
        <https://oracle.github.io/odpi/doc/installation.html#windows>`__ have
        been installed.

      - On Linux, check the ``LD_LIBRARY_PATH`` environment variable contains
        the Oracle Client library directory. If you are using Oracle Instant
        Client, a preferred alternative is to ensure a file in the
        ``/etc/ld.so.conf.d`` directory contains the path to the Instant Client
        directory, and then run ``ldconfig``.

      - On macOS, make sure you are not using the bundled Python (use `Homebrew
        <https://brew.sh>`__ or `Python.org
        <https://www.python.org/downloads>`__ instead).  If you are not using
        :meth:`~cx_Oracle.init_oracle_client()`, then put the Oracle Instant
        Client libraries in ``~/lib`` or ``/usr/local/lib``.

    - If you got "``DPI-1072: the Oracle Client library version is
      unsupported``", then review the installation requirements.  cx_Oracle
      needs Oracle client libraries 11.2 or later.  Note that version 19 is not
      supported on Windows 7.  Similar steps shown above for ``DPI-1047`` may
      help.

    - If you have multiple versions of Python installed, make sure you are
      using the correct python and pip (or python3 and pip3) executables.
