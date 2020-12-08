#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# PLSQLFunction.py
#
# Demonstrate how to call a PL/SQL function and get its return value.
#------------------------------------------------------------------------------

import cx_Oracle
import sample_env

connection = cx_Oracle.connect(sample_env.get_main_connect_string())

cursor = connection.cursor()
res = cursor.callfunc('myfunc', int, ('abc', 2))
print(res)
