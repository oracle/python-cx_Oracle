#------------------------------------------------------------------------------
# query_one.py (Section 3.2)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

cur.execute("select * from dept order by deptno")
row = cur.fetchone()
print(row)

row = cur.fetchone()
print(row)
