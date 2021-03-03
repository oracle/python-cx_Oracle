#------------------------------------------------------------------------------
# bind_query.py (Section 4.1)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
import db_config

con = cx_Oracle.connect(db_config.user, db_config.pw, db_config.dsn)
cur = con.cursor()

sql = "select * from dept where deptno = :id order by deptno"

cur.execute(sql, id = 20)
res = cur.fetchall()
print(res)

cur.execute(sql, id = 10)
res = cur.fetchall()
print(res)
