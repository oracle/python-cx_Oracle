# Python cx_Oracle

# News

**cx_Oracle has a major new release under a new name and homepage
[python-oracledb](https://oracle.github.io/python-oracledb/).**

**The source code has moved to
[github.com/oracle/python-oracledb](https://github.com/oracle/python-oracledb).**

New projects should install python-oracledb instead of cx_Oracle.  Critical
patches and binary packages for new Python releases may continue to be made in
the cx_Oracle namespace for a limited time, subject to demand.

# About

cx_Oracle is a Python extension module that enables access to Oracle
Database.  It conforms to the [Python database API 2.0
specification][1] with a considerable number of additions and a couple
of exclusions.  See the
[homepage](https://oracle.github.io/python-cx_Oracle/index.html) for a
feature list.

cx_Oracle 8.3 was tested with Python versions 3.6 through 3.10. You can
use cx_Oracle with Oracle 11.2, 12c, 18c, 19c and 21c client libraries.
Oracle's standard client-server version interoperability allows connection to
both older and newer databases. For example Oracle 19c client libraries can
connect to Oracle Database 11.2.   Older versions of cx_Oracle may work with
older versions of Python.

## Installation

See [cx_Oracle Installation][15].

## Documentation

See the [cx_Oracle Documentation][2] and [Release Notes][14].

## Samples

See the [/samples][12] directory and the [tutorial][6].  You can also
look at the scripts in [cx_OracleTools][7] and the modules in
[cx_PyOracleLib][8].

## Help

Issues and questions can be raised with the cx_Oracle community on
[GitHub][9] or on the [mailing list][5].

## Tests

See [/test][11].

## Contributing

See [CONTRIBUTING](https://github.com/oracle/python-cx_Oracle/blob/main/CONTRIBUTING.md)

## License

cx_Oracle is licensed under a BSD license which you can find [here][3].

[1]: https://peps.python.org/pep-0249/
[2]: https://cx-oracle.readthedocs.io
[3]: https://github.com/oracle/python-cx_Oracle/blob/main/LICENSE.txt
[5]: https://sourceforge.net/projects/cx-oracle/lists/cx-oracle-users
[6]: https://github.com/oracle/python-cx_Oracle/tree/main/samples/tutorial
[7]: http://cx-oracletools.sourceforge.net
[8]: http://cx-pyoraclelib.sourceforge.net
[9]: https://github.com/oracle/python-cx_Oracle/issues
[11]: https://github.com/oracle/python-cx_Oracle/tree/main/test
[12]: https://github.com/oracle/python-cx_Oracle/tree/main/samples
[14]: https://cx-oracle.readthedocs.io/en/latest/release_notes.html
[15]: https://cx-oracle.readthedocs.io/en/latest/user_guide/installation.html
