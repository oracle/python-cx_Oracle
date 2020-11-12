#------------------------------------------------------------------------------
# Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# DropTest.py
#
# Drops the database objects used for the cx_Oracle test suite.
#------------------------------------------------------------------------------

import cx_Oracle
import base

def drop_tests(conn):
    print("Dropping test schemas...")
    base.run_sql_script(conn, "DropTest",
                        main_user=base.get_main_user(),
                        proxy_user=base.get_proxy_user())

if __name__ == "__main__":
    conn = cx_Oracle.connect(base.get_admin_connect_string())
    drop_tests(conn)
    print("Done.")
