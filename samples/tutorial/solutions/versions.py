#------------------------------------------------------------------------------
# versions.py (Section 1.5)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")

print(cx_Oracle.version)
print("Database version:", con.version)
print("Client version:", cx_Oracle.clientversion())
