This directory contains samples for cx_Oracle.

1. The schemas and SQL objects that are referenced in the samples can be
   created by running the SQL script
   [sql/SetupSamples.sql][1]. The
   syntax is:

        sqlplus sys/syspassword@hostname/servicename as sysdba @sql/SetupSamples.sql

   The script will create users `pythondemo` and `pythoneditions` and
   will create an edition called `python_e1`.

   If you wish to change the names of the users or the name of the
   edition you can edit the file [sql/SampleEnv.sql][2]. You will also
   need to edit the file [SampleEnv.py][4] or set environment variables
   as documented in it.

2. Run a Python script, for example:

        python Query.py

3. After running cx_Oracle samples, the schemas and SQL objects can be
   dropped by running the SQL script [sql/DropSamples.sql][3]. The
   syntax is

        sqlplus sys/syspassword@hostname/servicename as sysdba @sql/DropSamples.sql

[1]: https://github.com/oracle/python-cx_Oracle/blob/master/samples/sql/SetupSamples.sql
[2]: https://github.com/oracle/python-cx_Oracle/blob/master/samples/sql/SampleEnv.sql
[3]: https://github.com/oracle/python-cx_Oracle/blob/master/samples/sql/DropSamples.sql
[4]: https://github.com/oracle/python-cx_Oracle/blob/master/samples/SampleEnv.py
