#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# BatchErrors.py
#
# Demonstrate the use of the Oracle Database 12.1 feature that allows
# cursor.executemany() to complete successfully, even if errors take
# place during the execution of one or more of the individual
# executions. The parameter "batcherrors" must be set to True in the
# call to cursor.executemany() after which cursor.getbatcherrors() can
# be called, which will return a list of error objects.
#
# This script requires cx_Oracle 5.2 and higher.
#------------------------------------------------------------------------------

from __future__ import print_function

import cx_Oracle
import SampleEnv

connection = cx_Oracle.connect(SampleEnv.MAIN_CONNECT_STRING)
cursor = connection.cursor()

# define data to insert
dataToInsert = [
    (1016, 1, 'Child 2 of Parent 1'),
    (1017, 1, 'Child 3 of Parent 1'),
    (1018, 2, 'Child 4 of Parent 2'),
    (1018, 2, 'Child 4 of Parent 2'),       # duplicate key
    (1019, 3, 'Child 3 of Parent 3'),
    (1020, 3, 'Child 4 of Parent 4'),
    (1021, 6, 'Child 1 of Parent 6'),       # parent does not exist
    (1022, 4, 'Child 6 of Parent 4'),
]

# retrieve the number of rows in the table
cursor.execute("""
        select count(*)
        from ChildTable""")
count, = cursor.fetchone()
print("number of rows in child table:", int(count))
print("number of rows to insert:", len(dataToInsert))

# old method: executemany() with data errors results in stoppage after the
# first error takes place; the row count is updated to show how many rows
# actually succeeded
try:
    cursor.executemany("insert into ChildTable values (:1, :2, :3)",
            dataToInsert)
except cx_Oracle.DatabaseError as e:
    error, = e.args
    print("FAILED with error:", error.message)
    print("number of rows which succeeded:", cursor.rowcount)

# demonstrate that the row count is accurate
cursor.execute("""
        select count(*)
        from ChildTable""")
count, = cursor.fetchone()
print("number of rows in child table after failed insert:", int(count))

# roll back so we can perform the same work using the new method
connection.rollback()

# new method: executemany() with batch errors enabled (and array DML row counts
# also enabled) results in no immediate error being raised
cursor.executemany("insert into ChildTable values (:1, :2, :3)", dataToInsert,
        batcherrors = True, arraydmlrowcounts = True)

# where errors have taken place, the row count is 0; otherwise it is 1
rowCounts = cursor.getarraydmlrowcounts()
print("Array DML row counts:", rowCounts)

# display the errors that have taken place
errors = cursor.getbatcherrors()
print("number of errors which took place:", len(errors))
for error in errors:
    print("Error", error.message.rstrip(), "at row offset", error.offset)

# demonstrate that all of the rows without errors have been successfully
# inserted
cursor.execute("""
        select count(*)
        from ChildTable""")
count, = cursor.fetchone()
print("number of rows in child table after successful insert:", int(count))
