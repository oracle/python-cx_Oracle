.. _installation:

************************
cx_Oracle 5 Installation
************************

Overview
========

Before cx_Oracle can be installed, an installation of
`Python <https://www.python.org/downloads>`__ is needed first. Python 2.7 and
Python 3.4 and higher are supported.

You must also install an Oracle Client, if that has not been done already.
Oracle Client versions 12.2, 12.1 and 11.2 are supported. The simplest Oracle
Client is the free `Oracle Instant Client
<http://www.oracle.com/technetwork/database/features/instant-client/
index.html>`__. Only the "Basic" or "Basic Light" package is required at
run-time. If you plan to build cx_Oracle from source, you will also need the
"SDK" package. Oracle Client libraries and the SDK are also available in any
Oracle Database installation or full Oracle Client installation.

Prebuilt binaries are available on
`PyPI <https://pypi.python.org/pypi/cx_Oracle/5.3>`__ for Windows and Linux.
These can be downloaded and installed directly.

If no prebuilt binaries are available for your platform or you prefer to build
from source you can download the source package from PyPI instead and run the
following commands:

    python setup.py build
    python setup.py install

You can also use pip, the generic tool for installing Python packages. It will
download the source, compile and install the module for you. The command to use
pip is the following:

    python -m pip install cx_Oracle==5.3

If you run into any difficulty building from source, the
`BUILD.txt
<https://github.com/oracle/python-cx_Oracle/blob/v5.x/BUILD.txt>`__ file can
be consulted for hints on how to build. The `Troubleshooting`_ section below
may also be of help. On Linux, if cx_Oracle needs to be compiled for the
default python package, you will need the ``python-devel`` package or
equivalent, which provides the ``Python.h`` header file.

Finally, you need an `Oracle Database`_ for Python to connect to. Oracle's
standard client-server version interoperability allows cx_Oracle to connect to
both older and newer databases. Python can be local or remote to the database.


Oracle Database
===============

Oracle Client libraries allow connection to older and newer databases.
In summary, Oracle Client 12.2 can connect to Oracle Database 11.2 or
greater. Oracle Client 12.1 can connect to Oracle Database 10.2 or
greater. Oracle Client 11.2 can connect to Oracle Database 9.2 or
greater. For additional information on which Oracle Database releases
are supported by which Oracle client versions, please see `Doc ID 207303.1
<https://support.oracle.com/epmos/faces/DocumentDisplay?id=207303.1>`__.

Newer Oracle clients support new features, such as the `oraaccess.xml
<https://docs.oracle.com/database/122/LNOCI/
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

If importing cx_Oracle fails:

    - Check the ``PATH`` environment variable on Windows. Ensure that you
      have restarted your command prompt if you have modified environment
      variables.
    - Check the ``LD_LIBRARY_PATH`` environment variable on Linux.
    - On macOS, make sure Oracle Instant Client is in `~/lib` or
      `/usr/local/lib` and that you are not using the bundled Python (use
      `Homebrew <https://brew.sh>`__ or `Python.org
      <https://www.python.org/downloads>`__ instead).
    - Check that Python, cx_Oracle and your Oracle Client libraries are all
      64-bit or all 32-bit.
    - on Windows, check that the correct Windows redistributables have been
      installed.

      - 11.2 : `VS 2005 64-bit <https://www.microsoft.com/en-us/download/details.aspx?id=18471>`__ or `VS 2005 32-bit <https://www.microsoft.com/en-ca/download/details.aspx?id=3387>`__
      - 12.1 : `VS 2010 <https://support.microsoft.com/en-us/kb/2977003#bookmark-vs2010>`__
      - 12.2 : `VS 2013 <https://support.microsoft.com/en-us/kb/2977003#bookmark-vs2013>`__

    - If you have both Python 2 and 3 installed, make sure you are
      using the correct python and pip (or python3 and pip3)
      executables.

