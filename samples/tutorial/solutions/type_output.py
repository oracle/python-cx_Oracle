#------------------------------------------------------------------------------
# type_output.py (Section 6.1)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb")

cur = con.cursor()

print("Standard output...")
for row in cur.execute("select * from dept"):
    print(row)

def ReturnNumbersAsStrings(cursor, name, defaultType, size, precision, scale):
    if defaultType == cx_Oracle.NUMBER:
        return cursor.var(str, 9, cursor.arraysize)

print("Output type handler output...")
cur = con.cursor()
cur.outputtypehandler = ReturnNumbersAsStrings
for row in cur.execute("select * from dept"):
    print(row)
