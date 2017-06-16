/*-----------------------------------------------------------------------------
 * Copyright 2017, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * DropTest.sql
 *   Drops database objects used for testing.
 *
 * Run this like:
 *   sqlplus / as sysdba @DropTest
 *---------------------------------------------------------------------------*/

begin
  for r in
      ( select username
        from dba_users
        where username in ('CX_ORACLE', 'CX_ORACLE_PROXY')
      ) loop
    execute immediate 'drop user ' || r.username || ' cascade';
  end loop;
end;
/

exit

