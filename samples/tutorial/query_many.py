#------------------------------------------------------------------------------
# query_many.py (Section 3.3)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, 2018, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

from __future__ import print_function

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

cur.execute("select * from dept order by deptno")
res = cur.fetchmany(numRows = 3)
print(res)
