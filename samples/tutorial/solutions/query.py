#------------------------------------------------------------------------------
# query.py (Section 1.3)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")

cur = con.cursor()
cur.execute("select * from dept order by deptno")
res = cur.fetchall()
for row in res:
    print(row)
