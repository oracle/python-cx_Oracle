/*-----------------------------------------------------------------------------
 * setup_tables.sql
 *   Creates the tables etc used by the cx_Oracle tutorial.
 *
 * Run this like:
 *
 *   sqlplus -l pythonhol/welcome@localhost/orclpdb1 @setup_tables
 *
 * Substitute your actual password and connection string.
 * You may want to run create_user.sql before running this.
 *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

whenever sqlerror exit failure

-- EMP/DEPT tables

begin execute immediate 'drop table emp'; exception when others then if sqlcode <> -942 then raise; end if; end;
/

create table emp
       (empno number(4) not null,
        ename varchar2(10),
        job varchar2(9),
        mgr number(4),
        hiredate date,
        sal number(7, 2),
        comm number(7, 2),
        deptno number(2));

insert into emp values
        (7369, 'SMITH',  'CLERK',     7902,
        to_date('17-DEC-1980', 'DD-MON-YYYY'),  800, NULL, 20);
insert into emp values
        (7499, 'ALLEN',  'SALESMAN',  7698,
        to_date('20-FEB-1981', 'DD-MON-YYYY'), 1600,  300, 30);
insert into emp values
        (7521, 'WARD',   'SALESMAN',  7698,
        to_date('22-FEB-1981', 'DD-MON-YYYY'), 1250,  500, 30);
insert into emp values
        (7566, 'JONES',  'MANAGER',   7839,
        to_date('2-APR-1981', 'DD-MON-YYYY'),  2975, NULL, 20);
insert into emp values
        (7654, 'MARTIN', 'SALESMAN',  7698,
        to_date('28-SEP-1981', 'DD-MON-YYYY'), 1250, 1400, 30);
insert into emp values
        (7698, 'BLAKE',  'MANAGER',   7839,
        to_date('1-MAY-1981', 'DD-MON-YYYY'),  2850, NULL, 30);
insert into emp values
        (7782, 'CLARK',  'MANAGER',   7839,
        to_date('9-JUN-1981', 'DD-MON-YYYY'),  2450, NULL, 10);
insert into emp values
        (7788, 'SCOTT',  'ANALYST',   7566,
        to_date('09-DEC-1982', 'DD-MON-YYYY'), 3000, NULL, 20);
insert into emp values
        (7839, 'KING',   'PRESIDENT', NULL,
        to_date('17-NOV-1981', 'DD-MON-YYYY'), 5000, NULL, 10);
insert into emp values
        (7844, 'TURNER', 'SALESMAN',  7698,
        to_date('8-SEP-1981', 'DD-MON-YYYY'),  1500,    0, 30);
insert into emp values
        (7876, 'ADAMS',  'CLERK',     7788,
        to_date('12-JAN-1983', 'DD-MON-YYYY'), 1100, NULL, 20);
insert into emp values
        (7900, 'JAMES',  'CLERK',     7698,
        to_date('3-DEC-1981', 'DD-MON-YYYY'),   950, NULL, 30);
insert into emp values
        (7902, 'FORD',   'ANALYST',   7566,
        to_date('3-DEC-1981', 'DD-MON-YYYY'),  3000, NULL, 20);
insert into emp values
        (7934, 'MILLER', 'CLERK',     7782,
        to_date('23-JAN-1982', 'DD-MON-YYYY'), 1300, NULL, 10);

begin execute immediate 'drop table dept'; exception when others then if sqlcode <> -942 then raise; end if; end;
/

create table dept
       (deptno number(2),
        dname varchar2(14),
        loc varchar2(13) );

insert into dept values (10, 'ACCOUNTING', 'NEW YORK');
insert into dept values (20, 'RESEARCH',   'DALLAS');
insert into dept values (30, 'SALES',      'CHICAGO');
insert into dept values (40, 'OPERATIONS', 'BOSTON');

commit;

-- Table for clob.py and clob_string.py

begin execute immediate 'drop table testclobs'; exception when others then if sqlcode <> -942 then raise; end if; end;
/

create table testclobs (
    id     number not null,
    myclob clob not null
);

-- Sequence for connect_pool.py

begin execute immediate 'drop sequence myseq'; exception when others then if sqlcode <> -2289 then raise; end if; end;
/

create sequence myseq;


quit
