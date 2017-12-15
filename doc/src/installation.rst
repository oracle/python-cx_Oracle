.. _installation:

************************
cx_Oracle 6 Installation
************************

Overview
========

To connect Python and Oracle Database you need:

- Python 2.7 or 3.4 and higher.

- The cx_Oracle module, which is available on `PyPI
  <https://pypi.python.org/pypi/cx_Oracle>`__.

- Oracle client libraries. These can be from the free `Oracle Instant
  Client
  <http://www.oracle.com/technetwork/database/features/instant-client/
  index.html>`__, or those included in Oracle Database if Python is on
  the same machine as the database.  Oracle client libraries versions
  12.2, 12.1 and 11.2 are supported on Linux, Windows and macOS.
  Users have also reported success with other platforms.

- An Oracle Database. Oracle's standard client-server version
  interoperability allows cx_Oracle to connect to both older and newer
  databases.

Quick Start cx_Oracle Installation
==================================

- An installation of `Python <https://www.python.org/downloads>`__ is
  needed. Python 2.7 and Python 3.4 and higher are supported.

- Install cx_Oracle from `PyPI
  <https://pypi.python.org/pypi/cx_Oracle>`__ with::

      python -m pip install cx_Oracle --upgrade

  If a binary wheel package is not available for your platform, the
  source package will be downloaded, compiled, and the resulting
  binary installed.

- Add Oracle 12.2, 12.1 or 11.2 client libraries to your operating
  system library search path such as ``PATH`` on Windows or
  ``LD_LIBRARY_PATH`` on Linux.  On macOS move the files to ``~/lib``
  or ``/usr/local/lib``.

    - If your database is remote, then download and unzip the client
      libraries from the free `Oracle Instant Client
      <http://www.oracle.com/technetwork/database/features/instant-client/
      index.html>`__ "Basic" or "Basic Light" package for your
      operating system architecture.

      Instant Client on Windows requires an appropriate `Microsoft
      Windows Redistributables
      <https://oracle.github.io/odpi/doc/installation.html#windows>`__.
      On Linux, the ``libaio`` (sometimes called ``libaio1``) package
      is needed.

    - Alternatively use the client libraries already available in a
      locally installed database such as the free `Oracle XE
      <http://www.oracle.com/technetwork/database/database-technologies/express-edition/overview/index.html>`__
      release.

  Version 12.2 client libraries can connect to Oracle Database 11.2 or
  greater. Version 12.1 client libraries can connect to Oracle Database
  10.2 or greater. Version 11.2 client libraries can connect to Oracle
  Database 9.2 or greater.

If you run into trouble, check out the section on `Troubleshooting`_.

The database abstraction layer in cx_Oracle is `ODPI-C
<https://github.com/oracle/odpi>`__, which means that the `ODPI-C
installation instructions
<https://oracle.github.io/odpi/doc/installation.html>`__ can be useful
to review.


Upgrading from cx_Oracle 5
==========================

If you are upgrading from cx_Oracle 5 note these installation changes:

    - When using Oracle Instant Client, you should not set ``ORACLE_HOME``.

    - On Linux, cx_Oracle 6 no longer uses Instant Client RPMs automatically.
      You must set ``LD_LIBRARY_PATH`` or use ``ldconfig`` to locate the Oracle
      client library.

    - PyPI no longer allows Windows installers or Linux RPMs to be
      hosted.  Use the supplied cx_Oracle Wheels instead.

Install Using Pip
=================

Pip is the generic tool for installing Python packages. If you do not have pip,
see the `pip installation documentation
<http://pip.readthedocs.io/en/latest/installing/>`__.

The command to install cx_Oracle 6 using pip on all platforms is::

    python -m pip install cx_Oracle --upgrade

This will download and install a pre-compiled binary matching your platform
and architecture automatically, if one is available. Pre-compiled binaries are
available for Windows and Linux. See
`PyPI <https://pypi.python.org/pypi/cx_Oracle>`__.

If a pre-compiled binary is not available, the source will be
downloaded, compiled, and the resulting binary installed. On Linux if
cx_Oracle needs to be compiled for the default python package, you
will need the ``python-devel`` package or equivalent, which provides
the `Python.h` header file.

On macOS make sure you are not using the bundled Python - use `Homebrew
<https://brew.sh>`__ or `Python.org <https://www.python.org/downloads>`__
instead.


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


Install Using Source from PyPI
==============================

The source package can be downloaded manually from
`PyPI <https://pypi.python.org/pypi/cx_Oracle>`__ and extracted, after
which the following commands should be run::

    python setup.py build
    python setup.py install


Install Oracle Client
=====================

Using cx_Oracle requires Oracle Client libraries to be installed. The libraries
provide the necessary network connectivity allowing applications to access an
Oracle Database instance. They also provide basic and advanced connection
management and data features to cx_Oracle. cx_Oracle uses the shared library
loading mechanism available on each supported platform to load the Oracle
Client library at runtime. Oracle Client versions 12.2, 12.1 and 11.2 are
supported.

The simplest Oracle Client is the free `Oracle Instant Client
<http://www.oracle.com/technetwork/database/features/instant-client/
index.html>`__. Only the "Basic" or "Basic Light" package is required. Oracle
Client libraries are also available in any Oracle Database installation or
full Oracle Client installation.

Make sure your library loading path, such as ``PATH`` on Windows, or
``LD_LIBRARY_PATH`` on Linux, is set to the location of the Oracle
Client libraries.  On macOS the libraries should be in ``~/lib`` or
``/usr/local/lib``.

On Windows, `Microsoft Windows Redistributables
<https://oracle.github.io/odpi/doc/installation.html#windows>`__
matching the version of the Oracle client libraries need to be
installed.

See `ODPI-C installation instructions
<https://oracle.github.io/odpi/doc/installation.html>`__ for details
on configuration.

Oracle Database
===============

Oracle's standard client-server network interoperability allows
connections between different versions of Oracle Client and Oracle
Database.  For certified configurations see Oracle Support's `Doc ID
207303.1 <https://support.oracle.com/epmos/faces/DocumentDisplay?id=207303.1>`__.
In summary, Oracle Client 12.2 can connect to Oracle Database 11.2 or
greater. Oracle Client 12.1 can connect to Oracle Database 10.2 or
greater. Oracle Client 11.2 can connect to Oracle Database 9.2 or
greater.  The technical restrictions on creating connections may be more
flexible.  For example Oracle Client 12.2 can successfully connect to Oracle
Database 10.2.

Since a single cx_Oracle binary can use multiple client versions and access
multiple database versions, it is important your application is tested in your
intended release environments. Newer Oracle clients support new features, such
as the `oraaccess.xml <https://docs.oracle.com/database/122/LNOCI/
more-oci-advanced-topics.htm#LNOCI73052>`__ external configuration file
available with 12.1 or later clients, and `session pool enhancements
<http://docs.oracle.com/database/122/LNOCI/release-changes.htm#LNOCI005>`__
to dead connection detection in 12.2 clients.

The function :func:`~cx_Oracle.clientversion()` can be used to determine
which Oracle Client version is in use and the attribute
:attr:`Connection.version` can be used to determine which Oracle
Database version a connection is accessing. These can then be used to adjust
application behavior accordingly. Attempts to use some Oracle features that are
not supported by a particular client/server combination may result in runtime
errors. These include:

    - when attempting to access attributes that are not supported by the
      current Oracle Client library you will get the error "ORA-24315: illegal
      attribute type"

    - when attempting to use implicit results with Oracle Client 11.2
      against Oracle Database 12c you will get the error "ORA-29481:
      Implicit results cannot be returned to client"

    - when attempting to get array DML row counts with Oracle Client
      11.2 you will get the error "DPI-1013: not supported"


Installing cx_Oracle 5.3
========================

If you require cx_Oracle 5.3, download a Windows installer from `PyPI
<https://pypi.python.org/pypi/cx_Oracle>`__ or use ``python -m pip
install cx-oracle==5.3`` to install from source.

Very old versions of cx_Oracle can be found in the files section at
`SourceForce <https://sourceforge.net/projects/cx-oracle/files/>`__.


Troubleshooting
===============

If installation fails:

    - Use option ``-v`` with pip. Review your output and logs. Try to install
      using a different method. **Google anything that looks like an error.**
      Try some potential solutions.

    - Was there a network connection error? Do you need to see the environment
      variables ``http_proxy`` and/or ``https_proxy``?

    - Do you get the error "``No module named pip``"? The pip module is builtin
      to Python from version 2.7.9 but is sometimes removed by the OS. Use the
      venv module (builtin to Python 3.x) or virtualenv module (Python 2.x)
      instead.

    - Do you get the error "``fatal error: dpi.h: No such file or directory``"
      when building from source code? Ensure that your source installation has a
      subdirectory called "odpi" containing files. If missing, review the
      section on `Install Using GitHub`_.

If importing cx_Oracle fails:

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

    - If you have both Python 2 and 3 installed, make sure you are
      using the correct python and pip (or python3 and pip3)
      executables.
