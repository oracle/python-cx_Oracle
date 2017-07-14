This directory contains the test suite for cx_Oracle.

The schemas and SQL objects that are referenced in the test suite can be
created by running the SQL script sql/SetupTest.sql. The syntax is:

    sqlplus / as sysdba @sql/SetupTest.sql

The script will create users cx_Oracle and cx_Oracle_proxy. If you wish to
change the names of the users or the name of the edition you can edit the file
sql/TestEnv.sql. You will also need to edit the file TestEnv.py or set
environment variables as documented in TestEnv.py.

After running the test suite, the schemas and SQL objects can be dropped by
running the SQL script sql/DropTest.sql. The syntax is

    sqlplus / as sysdba @sql/DropTest.sql

