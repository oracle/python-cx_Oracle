#------------------------------------------------------------------------------
# Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# DropSamples.py
#
# Drops the database objects used for the cx_Oracle samples.
#------------------------------------------------------------------------------

import cx_Oracle
import sample_env

def drop_samples(conn):
    print("Dropping sample schemas and edition...")
    sample_env.run_sql_script(conn, "DropSamples",
                              main_user=sample_env.get_main_user(),
                              edition_user=sample_env.get_edition_user(),
                              edition_name=sample_env.get_edition_name())

if __name__ == "__main__":
    conn = cx_Oracle.connect(sample_env.get_admin_connect_string())
    drop_samples(conn)
    print("Done.")
