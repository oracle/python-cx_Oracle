-------------------------------------------------------------------------------
-- bind_insert.sql (Section 4.2)
-------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

set echo off
@@db_config.sql
set echo on

connect &user/&pw@&connect_string

begin
  execute immediate 'drop table mytab';
exception
when others then
  if sqlcode not in (-00942) then
    raise;
  end if;
end;
/

create table mytab (id number, data varchar2(20), constraint my_pk primary key (id));

exit
