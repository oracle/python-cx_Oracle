#------------------------------------------------------------------------------
# query_many.py (Section 3.3)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

cur.execute("select * from dept order by deptno")
res = cur.fetchmany(numRows=3)
print(res)

print(res[0])    # first row
print(res[0][1]) # second element of first row
