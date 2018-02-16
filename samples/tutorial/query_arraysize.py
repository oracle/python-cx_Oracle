#------------------------------------------------------------------------------
# query_arraysize.py (Section 3.5)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, 2018, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

from __future__ import print_function

import cx_Oracle
import time

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")

start = time.time()

cur = con.cursor()
cur.arraysize = 10
cur.execute("select * from bigtab")
res = cur.fetchall()
# print(res)  # uncomment to display the query results

elapsed = (time.time() - start)
print(elapsed, "seconds")
