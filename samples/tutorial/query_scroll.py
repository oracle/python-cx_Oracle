#------------------------------------------------------------------------------
# query_scroll.py (Section 3.4)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor(scrollable = True)

cur.execute("select * from dept order by deptno")

cur.scroll(2, mode = "absolute")  # go to second row
print(cur.fetchone())

cur.scroll(-1)                    # go back one row
print(cur.fetchone())
