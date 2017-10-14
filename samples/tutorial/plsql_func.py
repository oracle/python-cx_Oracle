#------------------------------------------------------------------------------
# plsql_func.py (Section 5.1)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

res = cur.callfunc('myfunc', int, ('abc', 2))
print(res)
