#------------------------------------------------------------------------------
# bind_query.py (Section 4.1)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, 2018, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

from __future__ import print_function

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

cur.prepare("select * from dept where deptno = :id order by deptno")

cur.execute(None, id = 20)
res = cur.fetchall()
print(res)

cur.execute(None, id = 10)
res = cur.fetchall()
print(res)
