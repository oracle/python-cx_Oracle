#------------------------------------------------------------------------------
# query_scroll.py (Section 3.4)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
import db_config

con = cx_Oracle.connect(db_config.user, db_config.pw, db_config.dsn)
cur = con.cursor(scrollable = True)

cur.execute("select * from dept order by deptno")

cur.scroll(2, mode = "absolute")  # go to second row
print(cur.fetchone())

cur.scroll(-1)                    # go back one row
print(cur.fetchone())
