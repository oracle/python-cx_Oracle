set serveroutput on;

declare
  v_base_user   varchar2(100) := 'pythonhol';
  v_user        varchar2(100);
  v_pw          varchar2(100) := 'welcome';
  v_count       number := 50;
begin
--Drop Samples
  begin
    for r in (select username
              from dba_users
             where username like ( upper(v_base_user) || '\_%' ) ESCAPE '\') loop
      execute immediate 'drop user ' || r.username || ' cascade';
      dbms_output.put_line(r.username || ' dropped');
    end loop;
  end;

-- Setup Samples

  for i in 1..v_count loop
    v_user := v_base_user || '_' || i;

    execute immediate 'alter session set nls_date_format = ''YYYY-MM-DD HH24:MI:SS''';
    execute immediate 'alter session set nls_numeric_characters=''.,''';
    execute immediate 'create user ' || v_user || ' identified by ' || v_pw || ' quota unlimited on users default tablespace users';
    execute immediate 'grant
    create session,
    create table,
    create procedure,
    create type,
    select any dictionary,
    change notification to '|| v_user;

    execute immediate 'grant execute on dbms_aqadm to ' || v_user;
    execute immediate 'grant execute on dbms_lock to ' || v_user;
    execute immediate 'create table ' || v_user || '.testclobs (id number not null, myclob clob not null)';

-- Sequence for connect_pool.py
    execute immediate 'create sequence ' || v_user || '.myseq';

-- EMP/DEPT tables
    execute immediate 'CREATE TABLE ' || v_user || '.EMP
       (EMPNO NUMBER(4) NOT NULL,
        ENAME VARCHAR2(10),
        JOB VARCHAR2(9),
        MGR NUMBER(4),
        HIREDATE DATE,
        SAL NUMBER(7, 2),
        COMM NUMBER(7, 2),
        DEPTNO NUMBER(2))';
    execute immediate 'CREATE TABLE ' || v_user || '.DEPT
       (DEPTNO NUMBER(2),
        DNAME VARCHAR2(14),
        LOC VARCHAR2(13) )';

    execute immediate 'INSERT INTO ' || v_user || '.DEPT VALUES (10, ''ACCOUNTING'', ''NEW YORK'')';
    execute immediate 'INSERT INTO ' || v_user || '.DEPT VALUES (20, ''RESEARCH'',   ''DALLAS'')';
    execute immediate 'INSERT INTO ' || v_user || '.DEPT VALUES (30, ''SALES'',      ''CHICAGO'')';
    execute immediate 'INSERT INTO ' || v_user || '.DEPT VALUES (40, ''OPERATIONS'', ''BOSTON'')';

    execute immediate 'CREATE TABLE ' || v_user || '.BONUS
        (ENAME VARCHAR2(10),
         JOB   VARCHAR2(9),
         SAL   NUMBER,
         COMM  NUMBER)';

    execute immediate 'CREATE TABLE ' || v_user || '.SALGRADE
        (GRADE NUMBER,
         LOSAL NUMBER,
         HISAL NUMBER)';

    execute immediate 'INSERT INTO ' || v_user || '.SALGRADE VALUES (1,  700, 1200)';
    execute immediate 'INSERT INTO ' || v_user || '.SALGRADE VALUES (2, 1201, 1400)';
    execute immediate 'INSERT INTO ' || v_user || '.SALGRADE VALUES (3, 1401, 2000)';
    execute immediate 'INSERT INTO ' || v_user || '.SALGRADE VALUES (4, 2001, 3000)';
    execute immediate 'INSERT INTO ' || v_user || '.SALGRADE VALUES (5, 3001, 9999)';

    commit;
    dbms_output.put_line(v_user || ' Created');
  end loop;

end;
