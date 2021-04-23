This directory contains the test suite for cx_Oracle.

1.  The schemas and SQL objects that are referenced in the test suite can be
    created by running the Python script [setup_test.py][1]. The script requires
    administrative privileges and will prompt for these credentials as well as
    the names of the schemas that will be created, unless a number of
    environment variables are set, as documented in the Python script
    [test_env.py][2]. Run the script using the following command:

        python setup_test.py

    Alternatively, the [SQL script][3] can be run directly via SQL\*Plus, which
    will always prompt for the names of the schemas that will be created. Run
    the script using the following command:

        sqlplus system/systempassword@hostname/servicename @sql/setup_test.sql

2.  Run the test suite by issuing the following command in the top-level
    directory of your cx_Oracle installation:

        tox

    This will build the module in an independent environment and run the test
    suite using the module that was just built in that environment.
    Alternatively, you can use the currently installed build of cx_Oracle and
    run the following command instead:

        python -m unittest discover -v -s test

    You may also run each of the test scripts independently, as in:

        python test_1000_module.py

3.  After running the test suite, the schemas can be dropped by running the
    Python script [drop_test.py][4]. The script requires administrative
    privileges and will prompt for these credentials as well as the names of
    the schemas that will be dropped, unless a number of environment variables
    are set, as documented in the Python script [test_env.py][2]. Run the
    script using the following command:

        python drop_test.py

    Alternatively, the [SQL script][5] can be run directly via SQL\*Plus, which
    will always prompt for the names of the schemas that will be dropped. Run
    the script using the following command:

        sqlplus system/systempassword@hostname/servicename @sql/drop_test.sql

[1]: https://github.com/oracle/python-cx_Oracle/blob/master/test/setup_test.py
[2]: https://github.com/oracle/python-cx_Oracle/blob/master/test/test_env.py
[3]: https://github.com/oracle/python-cx_Oracle/blob/master/test/sql/setup_test.sql
[4]: https://github.com/oracle/python-cx_Oracle/blob/master/test/drop_test.py
[5]: https://github.com/oracle/python-cx_Oracle/blob/master/test/sql/drop_test.sql
