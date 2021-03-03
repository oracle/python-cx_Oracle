#------------------------------------------------------------------------------
# versions.py (Section 1.5)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
import db_config

con = cx_Oracle.connect(db_config.user, db_config.pw, db_config.dsn)

print(cx_Oracle.version)
print("Database version:", con.version)
print("Client version:", cx_Oracle.clientversion())
