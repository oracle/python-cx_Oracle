#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# ReturnLobsAsStrings.py
#   Returns all CLOB values as strings and BLOB values as bytes. The
# performance of this technique is significantly better than fetching the LOBs
# and then reading the contents of the LOBs as it avoids round-trips to the
# database. Be aware, however, that this method requires contiguous memory so
# is not usable for very large LOBs.
#
# This script requires cx_Oracle 5.0 and higher.
#------------------------------------------------------------------------------

import cx_Oracle
import sample_env

def output_type_handler(cursor, name, default_type, size, precision, scale):
    if default_type == cx_Oracle.CLOB:
        return cursor.var(cx_Oracle.LONG_STRING, arraysize=cursor.arraysize)
    if default_type == cx_Oracle.BLOB:
        return cursor.var(cx_Oracle.LONG_BINARY, arraysize=cursor.arraysize)

connection = cx_Oracle.connect(sample_env.get_main_connect_string())
connection.outputtypehandler = output_type_handler
cursor = connection.cursor()

# add some data to the tables
print("Populating tables with data...")
cursor.execute("truncate table TestClobs")
cursor.execute("truncate table TestBlobs")
long_string = ""
for i in range(10):
    char = chr(ord('A') + i)
    long_string += char * 25000
    # uncomment the line below for cx_Oracle 5.3 and earlier
    # cursor.setinputsizes(None, cx_Oracle.LONG_STRING)
    cursor.execute("insert into TestClobs values (:1, :2)",
            (i + 1, "STRING " + long_string))
    # uncomment the line below for cx_Oracle 5.3 and earlier
    # cursor.setinputsizes(None, cx_Oracle.LONG_BINARY)
    cursor.execute("insert into TestBlobs values (:1, :2)",
            (i + 1, long_string.encode("ascii")))
connection.commit()

# fetch the data and show the results
print("CLOBS returned as strings")
cursor.execute("""
        select
          IntCol,
          ClobCol
        from TestClobs
        order by IntCol""")
for int_col, value in cursor:
    print("Row:", int_col, "string of length", len(value))
print()
print("BLOBS returned as bytes")
cursor.execute("""
        select
          IntCol,
          BlobCol
        from TestBlobs
        order by IntCol""")
for int_col, value in cursor:
    print("Row:", int_col, "string of length", value and len(value) or 0)
