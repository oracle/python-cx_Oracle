# Python cx_Oracle interface to Oracle Database

cx_Oracle is a Python extension module that enables access to Oracle Database.
It conforms to the [Python database API 2.0 specification][1] with a
considerable number of additions and a couple of exclusions.

cx_Oracle is licensed under a BSD license which you can find [here][3].

cx_Oracle 6 has been tested with Python version 2.7, and with versions
3.4 and higher. You can use cx_Oracle with Oracle 11.2, 12.1 and 12.2
client libraries. Oracle's standard client-server version
interoperability allows connection to both older and newer databases,
for example Oracle 12.2 client libraries can connect to Oracle
Database 11.2.

## Getting Started

Install Python from [python.org][4].

Install cx_Oracle using [Quick Start cx_Oracle Installation][6].

Download cx_Oracle [samples][12] or create a script like the one
below.

Locate your Oracle Database username and password, and the database
connection string.  The connection string is commonly of the format
`hostname/servicename`, using the hostname where the database is
running, and the service name of the Oracle Database instance.

Substitute your username, password and connection string in the code.
For downloaded examples, put these in [`SampleEnv.py`][13] and
[`SampleEnv.sql`][10], and then follow [`sample/README`][16] to create
the cx_Oracle sample schema.  SQL scripts to create Oracle Database's
common sample schemas can be found at
[github.com/oracle/db-sample-schemas][17].

Run the Python script, for example:

```
python myscript.py
```

### Sample cx_Oracle Script

```python
# myscript.py

from __future__ import print_function

import cx_Oracle

# Connect as user "hr" with password "welcome" to the "oraclepdb" service running on this computer.
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

## Installation

See [cx_Oracle Installation][15].

## Examples

See the [samples][12] directory and the [test suite][11]. You can also
look at the scripts in [cx_OracleTools][7] and the modules in
[cx_PyOracleLib][8].

## Documentation

See the [cx_Oracle Documentation][2].

## Changes

See [What's New][18] and the [Release Notes][14].

## Tests

See the [test suite][11].

## Help

Issues and questions can be raised with the cx_Oracle community on
[GitHub][9] or on the [mailing list][5].

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

- Sharded Databases

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
[4]: https://www.python.org/downloads/
[5]: http://lists.sourceforge.net/lists/listinfo/cx-oracle-users
[6]: http://cx-oracle.readthedocs.io/en/latest/installation.html#quick-start-cx-oracle-installation
[7]: http://cx-oracletools.sourceforge.net
[8]: http://cx-pyoraclelib.sourceforge.net
[9]: https://github.com/oracle/python-cx_Oracle/issues
[10]: https://github.com/oracle/python-cx_Oracle/blob/master/samples/sql/SampleEnv.sql
[11]: https://github.com/oracle/python-cx_Oracle/tree/master/test
[12]: https://github.com/oracle/python-cx_Oracle/tree/master/samples
[13]: https://github.com/oracle/python-cx_Oracle/tree/master/samples/SampleEnv.py
[14]: http://cx-oracle.readthedocs.io/en/latest/releasenotes.html
[15]: http://cx-oracle.readthedocs.io/en/latest/installation.html
[16]: https://github.com/oracle/python-cx_Oracle/tree/master/samples/README.md
[17]: https://github.com/oracle/db-sample-schemas
[18]: http://cx-oracle.readthedocs.io/en/latest/whatsnew.html
