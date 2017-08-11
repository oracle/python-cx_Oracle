# Open Source Python/Oracle Utility - cx_Oracle

cx_Oracle is a Python extension module that enables access to Oracle Database.
It conforms to the [Python database API 2.0 specification][1] with a
considerable number of additions and a couple of exclusions.

cx_Oracle is licensed under a BSD license which you can find [here][3].

cx_Oracle 6 has been tested with Python version 2.7, and with versions
3.4 and higher. You can use cx_Oracle with Oracle 11.2, 12.1 and 12.2
client libraries. Oracle's standard client-server version
interoperability allows connection to both older and newer databases,
for example Oracle 12.2 client libraries can connect to Oracle
Database 11.2 or later.

## Documentation

See the [cx_Oracle Documentation][2] and [Release Notes][14].

## Help

Issues and questions can be raised with the cx_Oracle community on
[GitHub][9] or on the [mailing list][5].

## Installation

See [cx_Oracle Installation][15] for detailed instructions.

- The simplest way to install cx_Oracle 6 is with pip:

```
  python -m pip install cx_Oracle --upgrade
```

  If a binary wheel package is not available on [PyPI][6] for your platform, the
  source package will be used.

  Note that if you download a source zip file directly from GitHub
  that you will also need to download an [ODPI-C][10] source zip file
  and extract it inside the directory called "odpi".

- After cx_Oracle is installed, Oracle client libraries must also be installed
  and configured. These can be from Oracle Instant Client, from a local Oracle
  Database, or from a full Oracle Client installation.

  If you need the libraries, download and unzip the [Oracle Instant Client][4]
  'Basic' package for your platform and set PATH, LD_LIBRARY_PATH, or similar
  platform-specific library path loading environment.  See
  the [installation notes for ODPI-C][13] for help.

  Versions 11.2, 12.1 and 12.2 of the Oracle Client libraries on Linux,
  Windows and macOS are supported.  Users have also reported success
  with other platforms.

If you require cx_Oracle 5.3, download a Windows installer from
[PyPI][16] or use `python -m pip install cx-oracle==5.3` to
install from source.

Very old versions of cx_Oracle can be found in the files section at
[SourceForce][17].

## Example

```python
from __future__ import print_function

import cx_Oracle

connection = cx_Oracle.connect("hr", "welcome", "localhost/orclpdb")

cursor = connection.cursor()
cursor.execute("""
    SELECT first_name, last_name
    FROM employees
    WHERE department_id = :did AND employee_id > :eid""",
    did = 50,
    eid = 190)
for fname, lname in cursor:
print("Values:", fname, lname)
```

For more examples, please see the [test suite][11] and the
[samples][12]. You can also look at the scripts in [cx_OracleTools][7] and
the modules in [cx_PyOracleLib][8].

## Features

- Easily installed from PyPI.

- Support for Python 2 and 3.

- Support for Oracle Client 11.2, 12.1 and 12.2.  Oracle's standard
  cross-version interoperability, allows easy upgrades and
  connectivity to different Oracle Database versions.

- Connect to Oracle Database 9.2, 10, 11 or 12 (depending on the
  Oracle Client version used).

- SQL and PL/SQL Execution. The underlying Oracle Client libraries
  have significant optimizations including compressed fetch,
  pre-fetching, client and server result set caching, and statement
  caching with auto-tuning.

- Full use of Oracle Network Service infrastructure, including
  encrypted network traffic and security features.

- Extensive Oracle data type support, including large object support (CLOB
  and BLOB).

- Direct binding to SQL objects.  One great use case is binding Python
  objects to Oracle Spatial SDO objects.

- Array operations for efficient INSERT and UPDATEs.

- Array row counts and batch error handling for array operations.

- Fetching of large result sets.

- REF CURSOR support.

- Support for scrollable cursors. Go back and forth through your query
  results.

- Fetch PL/SQL Implicit Results. Easily return query results from
  PL/SQL.

- Row Prefetching.  Efficient use of the network.

- Client Result Caching.  Improve performance of frequently executed
  look-up statements.

- Support for Advanced Queuing. Use database notifications to build
  micro-service applications.

- Continuous Query Notification.  Get notified when data changes.

- Support for Edition Based Redefinition.  Easily switch applications
  to use updated PL/SQL logic.

- Support for setting application context during the creation of a
  connection, making application metadata more accessible to the
  database, including in LOGON triggers.

- End-to-end monitoring and tracing.

- Transaction Management.

- Session Pooling.

- Database Resident Connection Pooling (DRCP).

- Privileged Connections.

- External Authentication.

- Database startup and shutdown.

- Oracle Database High Availability Features, such as FAN notifications and Transaction Guard support.

**DB API specification exclusions**: The time data type is not
supported by Oracle and is therefore not implemented. The method
`cursor.nextset()` is not implemented either as the DB API specification assumes
an implementation of cursors that does not fit well with Oracle's implementation
of cursors and implicit results. See the method `cursor.getimplicitresults()`
for more information.

[1]: https://www.python.org/dev/peps/pep-0249
[2]: http://cx-oracle.readthedocs.io
[3]: https://github.com/oracle/python-cx_Oracle/blob/master/LICENSE.txt
[4]: http://www.oracle.com/technetwork/database/features/instant-client/index.html
[5]: http://lists.sourceforge.net/lists/listinfo/cx-oracle-users
[6]: https://pypi.python.org/pypi/cx_Oracle
[7]: http://cx-oracletools.sourceforge.net
[8]: http://cx-pyoraclelib.sourceforge.net
[9]: https://github.com/oracle/python-cx_Oracle/issues
[10]: https://oracle.github.io/odpi
[11]: https://github.com/oracle/python-cx_Oracle/tree/master/test
[12]: https://github.com/oracle/python-cx_Oracle/tree/master/samples
[13]: https://oracle.github.io/odpi/doc/installation.html
[14]: http://cx-oracle.readthedocs.io/en/latest/releasenotes.html
[15]: http://cx-oracle.readthedocs.io/en/latest/installation.html
[16]: https://pypi.python.org/pypi/cx_Oracle/5.3
[17]: https://sourceforge.net/projects/cx-oracle/files/
