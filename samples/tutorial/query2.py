#------------------------------------------------------------------------------
# query2.py (Section 3.1)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")

cur = con.cursor()
cur.execute('select * from dept order by deptno')
for deptno, dname, loc in cur:
    print("Department number: ", deptno)
    print("Department name: ", dname)
    print("Department location:", loc)
