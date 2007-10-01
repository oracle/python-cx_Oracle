Open Source Python/Oracle Utility - cx_Oracle

cx_Oracle is a Python extension module that allows access to Oracle and 
conforms to the Python database API 2.0 specifications with a few exceptions.

See http://www.python.org/topics/database/DatabaseAPI-2.0.html for more
information on the Python database API specification.

For comments, contact Anthony Tuininga at anthony.tuininga@gmail.com or use the
mailing list at http://lists.sourceforge.net/lists/listinfo/cx-oracle-users


BINARY INSTALL:
Place the file cx_Oracle.pyd or cx_Oracle.so anywhere on your Python path.


SOURCE INSTALL:
This module has been built with Oracle 8.1.7, 9.2.0 and 10.1.0 on Linux,
Solaris, HP/UX, Tru64 Unix and Windows. It will likely build on other
platforms and other Oracle versions but I haven't tried them. Use the provided
setup.py to build and install the module which makes use of the distutils
module. Note that on Windows, I have used mingw32 (http://www.mingw.org) and
the module will not build with MSVC without modification. The commands
required to build and install the module are as follows:

	python setup.py build
	python setup.py install


USAGE EXAMPLE:

import cx_Oracle

# connect via SQL*Net string or by each segment in a separate argument
#connection = cx_Oracle.connect("user/password@TNS")
connection = cx_Oracle.connect("user", "password", "TNS")

cursor = connection.cursor()
cursor.arraysize = 50
cursor.execute("""
        select Col1, Col2, Col3
        from SomeTable
        where Col4 = :arg_1
          and Col5 between :arg_2 and :arg_3""",
        arg_1 = "VALUE",
        arg_2 = 5,
        arg_3 = 15)
for column_1, column_2, column_3 in cursor.fetchall():
    print "Values:", column_1, column_2, column_3


For more examples, please see the test suite in the test directory and the
samples in the samples directory. You can also look at the scripts in the
cx_OracleTools (http://cx-oracletools.sourceforge.net) and the modules in the
cx_PyOracleLib (http://cx-pyoraclelib.sourceforge.net) projects.


EXCEPTIONS:
The only exception to the DB API specification is the lack of a nextset()
method which is not supported by Oracle.

Please see the included documentation for additional information.

