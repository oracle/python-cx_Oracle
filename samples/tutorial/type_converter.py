#------------------------------------------------------------------------------
# type_converter.py (Section 6.2)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")
cur = con.cursor()

for value, in cur.execute("select 0.1 from dual"):
    print("Value:", value, "* 3 =", value * 3)
