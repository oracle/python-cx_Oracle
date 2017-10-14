-------------------------------------------------------------------------------
-- bind_insert.sql (Section 4.2)
-------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------
 * Copyright 2017, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

set echo on

connect pythonhol/welcome@localhost/orclpdb

drop table mytab;

create table mytab (id number, data varchar2(20), constraint my_pk primary key (id));

exit
