/*-----------------------------------------------------------------------------
 * drop_user.sql
 *   Drops the database user used for the cx_Oracle tutorial
 *
 * Run this like:
 *
 *   sqlplus -l system/systempassword@localhost/orclpdb1 @drop_user
 *
 * Substitute your actual password and connection string.
 * For Oracle Autonmous Database, use 'admin' instead of system.
 * You will be prompted for the user to drop.
 *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

whenever sqlerror exit failure
set verify off feedback off

prompt WARNING: this scripts drops a user from the database
accept user char prompt 'Enter username to drop: '

begin
  dbms_aqadm.stop_queue('BOOKS');
  dbms_aqadm.drop_queue('BOOKS');
  dbms_aqadm.drop_queue_table('BOOK_QUEUE_TABLE');
  exception when others then
    if sqlcode <> -24010 then
      raise;
    end if;
  end;
/

begin

    for r in
            ( select username
              from dba_users
              where username in (upper('&user'))
            ) loop
        execute immediate 'drop user ' || r.username || ' cascade';
    end loop;
end;
/

prompt
prompt User &user has been dropped
prompt

quit
