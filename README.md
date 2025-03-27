# Python cx_Oracle

**cx_Oracle was obsoleted by
[python-oracledb](https://oracle.github.io/python-oracledb/) in 2022.**

Python-oracledb uses the same Python DB API as cx_Oracle, and has many new
features.

Install with:

```
python -m pip install oracledb
```

Usage is like:

```
import getpass
import oracledb

un = 'scott'
cs = 'localhost/orclpdb1'
pw = getpass.getpass(f'Enter password for {un}@{cs}: ')

with oracledb.connect(user=un, password=pw, dsn=cs) as connection:
    with connection.cursor() as cursor:
        sql = 'select systimestamp from dual'
        for r in cursor.execute(sql):
            print(r)
```

The source code for python-oracledb is at
[github.com/oracle/python-oracledb](https://github.com/oracle/python-oracledb).
