/*-----------------------------------------------------------------------------
 * create_user.sql
 *   Creates a database user for the cx_Oracle tutorial
 *
 * Then run this like:
 *
 *   sqlplus -l system/systempassword@localhost/orclpdb1 @create_user
 *
 * Substitute your actual password and connection string.
 * For Oracle Autonmous Database, use 'admin' instead of system.
 * You will be prompted for the new username and the new password to use.
 *
 * When you no longer need this user, run drop_user.sql to drop the user
 *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

whenever sqlerror exit failure
set verify off feedback off

accept user char prompt 'Enter new database username: '

create user &user;

grant
    create session,
    create table,
    create procedure,
    create type,
    create sequence,
    select any dictionary,
    unlimited tablespace
to &user;

begin

    for r in
            ( select role
              from dba_roles
              where role in ('SODA_APP', 'AQ_ADMINISTRATOR_ROLE')
            ) loop
        execute immediate 'grant ' || r.role || ' to &user';
    end loop;

end;
/

accept pw char prompt 'Enter password for &user: ' hide
alter user &user identified by "&pw";

prompt
prompt Database user &user has been created.
prompt Now you should run setup_tables.sql
prompt

quit
