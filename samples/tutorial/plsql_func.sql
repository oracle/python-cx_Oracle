-------------------------------------------------------------------------------
-- plsql_func.sql (Section 5.1)
-------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------
 * Copyright 2017, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

set echo on

connect pythonhol/welcome@localhost/orclpdb

create table ptab (mydata varchar(20), myid number);

create or replace function myfunc(d_p in varchar2, i_p in number) return number as
  begin
    insert into ptab (mydata, myid) values (d_p, i_p);
    return (i_p * 2);
  end;
/
show errors

exit
