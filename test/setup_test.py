#------------------------------------------------------------------------------
# Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# setup_test.py
#
# Creates users and populates their schemas with the tables and packages
# necessary for the test suite.
#------------------------------------------------------------------------------

import cx_Oracle as oracledb
import drop_test
import test_env

# connect as administrative user (usually SYSTEM or ADMIN)
conn = oracledb.connect(test_env.get_admin_connect_string())

# drop existing users and editions, if applicable
drop_test.drop_tests(conn)

# create test schemas
print("Creating test schemas...")
test_env.run_sql_script(conn, "setup_test",
                        main_user=test_env.get_main_user(),
                        main_password=test_env.get_main_password(),
                        proxy_user=test_env.get_proxy_user(),
                        proxy_password=test_env.get_proxy_password())
print("Done.")
