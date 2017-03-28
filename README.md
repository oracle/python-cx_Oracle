# Open Source Python/Oracle Utility - cx_Oracle

cx_Oracle is a Python extension module that enables access to Oracle Database
and conforms to the Python database API 2.0 specifications with a considerable
number of additions and a couple of exclusions. The time data type is not
supported by Oracle and is therefore not implemented. The method
cursor.nextset() is not implemented either as the DB API specification assumes
an implementation of cursors that does not fit well with Oracle's
implementation of cursors and implicit results. See the method
cursor.getimplicitresults() for more information.

See [PEP 249][1] for more information on the Python database API specification.
See the [documentation][2] for a complete description of the module's
capabilities.

cx_Oracle is licensed under a BSD license which you can find [here][3].

cx_Oracle has been tested with Python version 2.7, and with versions 3.4 and
higher. You can use cx_Oracle with Oracle 11.2, 12.1 and 12.2 client libraries,
allowing connection to multiple Oracle Database versions. Oracle's standard
client-server version interoperability allows connection to both older and
newer databases, for example Oracle 11.2 client libraries can connect to Oracle
Database 10.2 or later.

Please note that an Oracle client (or server) installation is required in order
to use cx_Oracle. If you do not require the tools that come with a full client
installation, it is recommended to install the [Instant Client][4].
which is far easier to install.

## Help

Issues and questions can be raised with the cx_Oracle community on
[GitHub][9] or on the [mailing list][5].


## Installation

Binaries for some platforms are available at [PyPI][6]. If you prefer to build
your own you can use this command

    python -m pip install cx_Oracle

which will download the source package, build and install it. Otherwise, you
can download the source package directly from PyPI, extract it and run these
commands instead

    python setup.py build
    python setup.py install

This module is built with [ODPI-C][10] which supports versions 11.2, 12.1 and
12.2 of the Oracle Client libraries on Linux, Windows and macOS. Others have
reported success with other platforms.


## Usage Example


```python
from __future__ import print_function   # needed for Python 2.x

import cx_Oracle

# connect via SQL*Net string or by each segment in a separate argument
#connection = cx_Oracle.connect("user/password@TNS")
connection = cx_Oracle.connect("user", "password", "TNS")

cursor = connection.cursor()
cursor.execute("""
        select Col1, Col2, Col3
        from SomeTable
        where Col4 = :arg_1
          and Col5 between :arg_2 and :arg_3""",
        arg_1 = "VALUE",
        arg_2 = 5,
        arg_3 = 15)
for column_1, column_2, column_3 in cursor:
    print("Values:", column_1, column_2, column_3)
```


For more examples, please see the test suite in the test directory and the
samples in the samples directory. You can also look at the scripts in the
[cx_OracleTools][7] and the modules in the [cx_PyOracleLib][8] projects.

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

