#------------------------------------------------------------------------------
# Copyright (c) 2019, 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# drop_test.py
#
# Drops the database objects used by the test suite.
#------------------------------------------------------------------------------

import cx_Oracle as oracledb
import test_env

def drop_tests(conn):
    print("Dropping test schemas...")
    test_env.run_sql_script(conn, "drop_test",
                            main_user=test_env.get_main_user(),
                            proxy_user=test_env.get_proxy_user())

if __name__ == "__main__":
    conn = oracledb.connect(test_env.get_admin_connect_string())
    drop_tests(conn)
    print("Done.")
