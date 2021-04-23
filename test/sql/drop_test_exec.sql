/*-----------------------------------------------------------------------------
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * drop_test_exec.sql
 *   This script performs the actual work of dropping the database schemas used
 * by the cx_Oracle test suite. It is called by the drop_test.sql and
 * setup_test.sql scripts after acquiring the necessary parameters and also by
 * the Python script drop_test.py.
 *---------------------------------------------------------------------------*/

begin

    for r in
            ( select username
              from dba_users
              where username in (upper('&main_user'), upper('&proxy_user'))
            ) loop
        execute immediate 'drop user ' || r.username || ' cascade';
    end loop;

end;
/
