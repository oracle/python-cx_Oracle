-------------------------------------------------------------------------------
-- drcp_query.sql (Section 2.4 and 2.5)
-------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------
 * Copyright (c) 2017, 2021, Oracle and/or its affiliates. All rights reserved.
 *---------------------------------------------------------------------------*/

set echo off verify off feedback off linesize 80 pagesize 1000

accept pw char prompt 'Enter database password for SYSTEM: ' hide
accept connect_string char prompt 'Enter database connection string: '

-- Connect to the CDB to see pool statistics
connect system/&pw@&connect_string

col cclass_name format a40

-- Some DRCP pool statistics
select cclass_name, num_requests, num_hits, num_misses from v$cpool_cc_stats;

exit
