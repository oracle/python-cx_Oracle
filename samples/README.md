This directory contains samples for cx_Oracle.

The schemas and SQL objects that are referenced in the samples can be created
by running the SQL script sql/SetupSamples.sql. The syntax is:

    sqlplus / as sysdba @sql/SetupSamples.sql

The script will create users pythondemo and pythoneditions and will create an
edition called python_e1. If you wish to change the names of the users or the
name of the edition you can edit the file sql/SampleEnv.sql. You will also
need to edit the file SampleEnv.py or set environment variables as documented
in SampleEnv.py.

After running the samples, the schemas and SQL objects can be dropped by
running the SQL script sql/DropSamples.sql. The syntax is

    sqlplus / as sysdba @sql/DropSamples.sql

