#------------------------------------------------------------------------------
# plsql_proc.py (Section 5.2)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, 2018, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

from __future__ import print_function

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

myvar = cur.var(int)
cur.callproc('myproc', (123, myvar))
print(myvar.getvalue())
