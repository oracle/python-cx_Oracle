/*-----------------------------------------------------------------------------
 * Copyright 2017, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * SetupSamples.sql
 *   Creates users and populates their schemas with the tables and packages
 * necessary for the cx_Oracle samples. An edition is also created for the
 * demonstration of PL/SQL editioning.
 *
 * Run this like:
 *   sqlplus / as sysdba @SetupSamples
 *
 * Note that the script SampleEnv.sql should be modified if you would like to
 * use something other than the default configuration.
 *---------------------------------------------------------------------------*/

whenever sqlerror exit failure

-- drop existing users and edition, if applicable
@@DropSamples.sql

alter session set nls_date_format = 'YYYY-MM-DD HH24:MI:SS';
alter session set nls_numeric_characters='.,';

create user &main_user identified by &main_password
quota unlimited on users
default tablespace users;

grant
    create session,
    create table,
    create procedure,
    create type,
    select any dictionary,
    change notification
to &main_user;

grant execute on dbms_aqadm to &main_user;
grant execute on dbms_lock to &main_user;

create user &edition_user identified by &edition_password;

grant
    create session,
    create procedure
to &edition_user;

alter user &edition_user enable editions;

create edition &edition_name;

grant use on edition &edition_name to &edition_user;

-- create types

create type &main_user..udt_SubObject as object (
    SubNumberValue                      number,
    SubStringValue                      varchar2(60)
);
/

create or replace type &main_user..udt_Building as object (
    BuildingId                          number(9),
    NumFloors                           number(3),
    Description                         varchar2(60),
    DateBuilt                           date
);
/

-- create tables

create table &main_user..TestNumbers (
    IntCol                              number(9) not null,
    NumberCol                           number(9, 2) not null,
    FloatCol                            float not null,
    UnconstrainedCol                    number not null,
    NullableCol                         number(38)
);

create table &main_user..TestStrings (
    IntCol                number(9) not null,
    StringCol             varchar2(20) not null,
    RawCol                raw(30) not null,
    FixedCharCol          char(40) not null,
    NullableCol           varchar2(50)
);

create table &main_user..TestCLOBs (
    IntCol                number(9) not null,
    CLOBCol               clob not null
);

create table &main_user..TestBLOBs (
    IntCol                number(9) not null,
    BLOBCol               blob not null
);

create table &main_user..TestTempTable (
    IntCol                number(9) not null,
    StringCol             varchar2(400),
    constraint TestTempTable_pk primary key (IntCol)
);

create table &main_user..TestUniversalRowids (
    IntCol                number(9) not null,
    StringCol             varchar2(250) not null,
    DateCol               date not null,
    constraint TestUniversalRowids_pk primary key (IntCol, StringCol, DateCol)
) organization index;

create table &main_user..TestBuildings (
    BuildingId            number(9) not null,
    BuildingObj &main_user..udt_Building not null
);

create table &main_user..BigTab (
    mycol                 varchar2(20)
);

create table &main_user..SampleQueryTab (
    id                    number not null,
    name                  varchar2(20) not null
);

create table &main_user..MyTab (
    id                    number,
    data                  varchar2(20)
);

create table &main_user..ParentTable (
    ParentId              number(9) not null,
    Description           varchar2(60) not null,
    constraint ParentTable_pk primary key (ParentId)
);

create table &main_user..ChildTable (
    ChildId               number(9) not null,
    ParentId              number(9) not null,
    Description           varchar2(60) not null,
    constraint ChildTable_pk primary key (ChildId),
    constraint ChildTable_fk foreign key (ParentId) references &main_user..ParentTable
);

create table &main_user..Ptab (
    myid                  number,
    mydata                varchar(20)
);

-- populate tables

begin
  for i in 1..20000
  loop
   insert into &main_user..BigTab (mycol) values (dbms_random.string('A',20));
  end loop;
end;
/

begin
  for i in 1..10 loop
    insert into &main_user..TestNumbers
    values (i, i + i * 0.25, i + i * .75, i * i * i + i *.5,
        decode(mod(i, 2), 0, null, power(143, i)));
  end loop;
end;
/

declare

  t_RawValue            raw(30);

  function ConvertHexDigit(a_Value number) return varchar2 is
  begin
    if a_Value between 0 and 9 then
      return to_char(a_Value);
    end if;
    return chr(ascii('A') + a_Value - 10);
  end;

  function ConvertToHex(a_Value varchar2) return varchar2 is
    t_HexValue          varchar2(60);
    t_Digit             number;
  begin
    for i in 1..length(a_Value) loop
      t_Digit := ascii(substr(a_Value, i, 1));
      t_HexValue := t_HexValue ||
          ConvertHexDigit(trunc(t_Digit / 16)) ||
          ConvertHexDigit(mod(t_Digit, 16));
    end loop;
    return t_HexValue;
  end;

begin
  for i in 1..10 loop
    t_RawValue := hextoraw(ConvertToHex('Raw ' || to_char(i)));
    insert into &main_user..TestStrings
    values (i, 'String ' || to_char(i), t_RawValue,
        'Fixed Char ' || to_char(i),
        decode(mod(i, 2), 0, null, 'Nullable ' || to_char(i)));
  end loop;
end;
/

insert into &main_user..ParentTable values (1, 'Parent 1');
insert into &main_user..ParentTable values (2, 'Parent 2');
insert into &main_user..ParentTable values (3, 'Parent 3');
insert into &main_user..ParentTable values (4, 'Parent 4');
insert into &main_user..ParentTable values (5, 'Parent 5');

insert into &main_user..ChildTable values (1001, 1, 'Child 1 of Parent 1');
insert into &main_user..ChildTable values (1002, 2, 'Child 1 of Parent 2');
insert into &main_user..ChildTable values (1003, 2, 'Child 2 of Parent 2');
insert into &main_user..ChildTable values (1004, 2, 'Child 3 of Parent 2');
insert into &main_user..ChildTable values (1005, 3, 'Child 1 of Parent 3');
insert into &main_user..ChildTable values (1006, 3, 'Child 2 of Parent 3');
insert into &main_user..ChildTable values (1007, 4, 'Child 1 of Parent 4');
insert into &main_user..ChildTable values (1008, 4, 'Child 2 of Parent 4');
insert into &main_user..ChildTable values (1009, 4, 'Child 3 of Parent 4');
insert into &main_user..ChildTable values (1010, 4, 'Child 4 of Parent 4');
insert into &main_user..ChildTable values (1011, 4, 'Child 5 of Parent 4');
insert into &main_user..ChildTable values (1012, 5, 'Child 1 of Parent 5');
insert into &main_user..ChildTable values (1013, 5, 'Child 2 of Parent 5');
insert into &main_user..ChildTable values (1014, 5, 'Child 3 of Parent 5');
insert into &main_user..ChildTable values (1015, 5, 'Child 4 of Parent 5');

insert into &main_user..SampleQueryTab values (1, 'Anthony');
insert into &main_user..SampleQueryTab values (2, 'Barbie');
insert into &main_user..SampleQueryTab values (3, 'Chris');
insert into &main_user..SampleQueryTab values (4, 'Dazza');
insert into &main_user..SampleQueryTab values (5, 'Erin');
insert into &main_user..SampleQueryTab values (6, 'Frankie');
insert into &main_user..SampleQueryTab values (7, 'Gerri');

commit;

--
-- For PL/SQL Examples
--

create or replace function &main_user..myfunc (
    a_Data                              varchar2,
    a_Id                                number
) return number as
begin
    insert into &main_user..ptab (mydata, myid) values (a_Data, a_Id);
    return (a_Id * 2);
end;
/

create or replace procedure &main_user..myproc (
    a_Value1                            number,
    a_Value2                            out number
) as
begin
   a_Value2 := a_Value1 * 2;
end;
/


--
-- Create package for demoing PL/SQL collections and records.
--

create or replace package &main_user..pkg_Demo as

    type udt_StringList is table of varchar2(100) index by binary_integer;

    type udt_DemoRecord is record (
        NumberValue                     number,
        StringValue                     varchar2(30),
        DateValue                       date,
        BooleanValue                    boolean
    );

    procedure DemoCollectionOut (
        a_Value                         out nocopy udt_StringList
    );

    procedure DemoRecordsInOut (
        a_Value                         in out nocopy udt_DemoRecord
    );

end;
/

create or replace package body &main_user..pkg_Demo as

    procedure DemoCollectionOut (
        a_Value                         out nocopy udt_StringList
    ) is
    begin
        a_Value(-1048576) := 'First element';
        a_Value(-576) := 'Second element';
        a_Value(284) := 'Third element';
        a_Value(8388608) := 'Fourth element';
    end;

    procedure DemoRecordsInOut (
        a_Value                         in out nocopy udt_DemoRecord
    ) is
    begin
        a_Value.NumberValue := a_Value.NumberValue * 2;
        a_Value.StringValue := a_Value.StringValue || ' (Modified)';
        a_Value.DateValue := a_Value.DateValue + 5;
        a_Value.BooleanValue := not a_Value.BooleanValue;
    end;

end;
/

