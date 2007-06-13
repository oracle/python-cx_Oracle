/*-----------------------------------------------------------------------------
 * SetupTest_9i.sql
 *   Additional setup for Oracle 9i databases only.
 *---------------------------------------------------------------------------*/

alter session set nls_date_format = 'YYYY-MM-DD HH24:MI:SS';
alter session set nls_numeric_characters='.,';

create table cx_Oracle.TestTimestamps (
  IntCol			number(9) not null,
  TimestampCol			timestamp not null,
  NullableCol			timestamp
) tablespace users;

begin
  for i in 1..10 loop
    insert into cx_Oracle.TestTimestamps
    values (i, to_timestamp('20021209', 'YYYYMMDD') +
            to_dsinterval(to_char(i) || ' 00:00:' || to_char(i * 2) || '.' ||
                    to_char(i * 50)),
        decode(mod(i, 2), 0, to_timestamp(null, 'YYYYMMDD'),
        to_timestamp('20021209', 'YYYYMMDD') +
            to_dsinterval(to_char(i + 1) || ' 00:00:' ||
                    to_char(i * 3) || '.' || to_char(i * 125))));
  end loop;
end;
/

commit;

