This directory contains the test suite for cx_Oracle.

1. The schemas and SQL objects that are referenced in the test suite can be
   created by running the Python script [SetupTest.py][1]. The script requires
   SYSDBA privileges and will prompt for these credentials as well as the
   names of the schemas that will be created, unless a number of environment
   variables are set as documented in the Python script [TestEnv.py][2]. Run
   the script using the following command:

       python SetupTest.py

   Alternatively, the [SQL script][3] can be run directly via SQL\*Plus, which
   will always prompt for the names of the schemas that will be created. Run
   the script using the following command:

       sqlplus sys/syspassword@hostname/servicename @sql/SetupTest.sql

2. Run the test suite by issuing the following command in the top-level
   directory of your cx_Oracle installation:

       python setup.py test

   Alternatively, you can run the test suite directly within this directory:

       python test.py

3. After running the test suite, the schemas can be dropped by running the
   Python script [DropTest.py][4]. The script requires SYSDBA privileges and
   will prompt for these credentials as well as the names of the schemas
   that will be dropped, unless a number of environment variables are set as
   documented in the Python script [TestEnv.py][2]. Run the script using the
   following command:

       python DropTest.py

   Alternatively, the [SQL script][5] can be run directly via SQL\*Plus, which
   will always prompt for the names of the schemas that will be dropped. Run
   the script using the following command:

       sqlplus sys/syspassword@hostname/servicename @sql/DropTest.sql

[1]: https://github.com/oracle/python-cx_Oracle/blob/master/test/SetupTest.py
[2]: https://github.com/oracle/python-cx_Oracle/blob/master/test/TestEnv.py
[3]: https://github.com/oracle/python-cx_Oracle/blob/master/test/sql/SetupTest.sql
[4]: https://github.com/oracle/python-cx_Oracle/blob/master/test/DropTest.py
[5]: https://github.com/oracle/python-cx_Oracle/blob/master/test/sql/DropTest.sql

