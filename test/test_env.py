#------------------------------------------------------------------------------
# Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Sets the environment used by the cx_Oracle test suite. Production
# applications should consider using External Authentication to
# avoid hard coded credentials.
#
# You can set values in environment variables to bypass having the test suite
# request the information it requires.
#
#     CX_ORACLE_TEST_MAIN_USER: user used for most test cases
#     CX_ORACLE_TEST_MAIN_PASSWORD: password of user used for most test cases
#     CX_ORACLE_TEST_PROXY_USER: user for testing proxying
#     CX_ORACLE_TEST_PROXY_PASSWORD: password of user for testing proxying
#     CX_ORACLE_TEST_CONNECT_STRING: connect string for test suite
#     CX_ORACLE_TEST_ADMIN_USER: administrative user for test suite
#     CX_ORACLE_TEST_ADMIN_PASSWORD: administrative password for test suite
#
# CX_ORACLE_TEST_CONNECT_STRING can be set to an Easy Connect string, or a
# Net Service Name from a tnsnames.ora file or external naming service,
# or it can be the name of a local Oracle database instance.
#
# If cx_Oracle is using Instant Client, then an Easy Connect string is
# generally appropriate. The syntax is:
#
#   [//]host_name[:port][/service_name][:server_type][/instance_name]
#
# Commonly just the host_name and service_name are needed
# e.g. "localhost/orclpdb1" or "localhost/XEPDB1"
#
# If using a tnsnames.ora file, the file can be in a default
# location such as $ORACLE_HOME/network/admin/tnsnames.ora or
# /etc/tnsnames.ora.  Alternatively set the TNS_ADMIN environment
# variable and put the file in $TNS_ADMIN/tnsnames.ora.
#
# The administrative user for cloud databases is ADMIN and the administrative
# user for on premises databases is SYSTEM.
#------------------------------------------------------------------------------

import getpass
import os
import sys
import unittest

import cx_Oracle as oracledb

# default values
DEFAULT_MAIN_USER = "pythontest"
DEFAULT_PROXY_USER = "pythontestproxy"
DEFAULT_CONNECT_STRING = "localhost/orclpdb1"

# dictionary containing all parameters; these are acquired as needed by the
# methods below (which should be used instead of consulting this dictionary
# directly) and then stored so that a value is not requested more than once
PARAMETERS = {}

def get_value(name, label, default_value=""):
    value = PARAMETERS.get(name)
    if value is not None:
        return value
    env_name = "CX_ORACLE_TEST_" + name
    value = os.environ.get(env_name)
    if value is None:
        if default_value:
            label += " [%s]" % default_value
        label += ": "
        if default_value:
            value = input(label).strip()
        else:
            value = getpass.getpass(label)
        if not value:
            value = default_value
    PARAMETERS[name] = value
    return value

def get_admin_connect_string():
    admin_user = get_value("ADMIN_USER", "Administrative user", "admin")
    admin_password = get_value("ADMIN_PASSWORD",
                               "Password for %s" % admin_user)
    return "%s/%s@%s" % (admin_user, admin_password, get_connect_string())

def get_charset_ratios():
    value = PARAMETERS.get("CS_RATIO")
    if value is None:
        connection = get_connection()
        cursor = connection.cursor()
        cursor.execute("""
                select
                    cast('X' as varchar2(1)),
                    cast('Y' as nvarchar2(1))
                from dual""")
        varchar_column_info, nvarchar_column_info = cursor.description
        value = (varchar_column_info[3], nvarchar_column_info[3])
        PARAMETERS["CS_RATIO"] = value
    return value

def get_client_version():
    name = "CLIENT_VERSION"
    value = PARAMETERS.get(name)
    if value is None:
        value = oracledb.clientversion()[:2]
        PARAMETERS[name] = value
    return value

def get_connection(**kwargs):
    return oracledb.connect(dsn=get_connect_string(), user=get_main_user(),
                            password=get_main_password(), **kwargs)

def get_connect_string():
    return get_value("CONNECT_STRING", "Connect String",
                     DEFAULT_CONNECT_STRING)

def get_main_password():
    return get_value("MAIN_PASSWORD", "Password for %s" % get_main_user())

def get_main_user():
    return get_value("MAIN_USER", "Main User Name", DEFAULT_MAIN_USER)

def get_pool(user=None, password=None, **kwargs):
    if user is None:
        user = get_main_user()
    if password is None:
        password = get_main_password()
    return oracledb.SessionPool(user, password, get_connect_string(),
                                **kwargs)

def get_proxy_password():
    return get_value("PROXY_PASSWORD", "Password for %s" % get_proxy_user())

def get_proxy_user():
    return get_value("PROXY_USER", "Proxy User Name", DEFAULT_PROXY_USER)

def get_sleep_proc_name():
    server_version = get_server_version()
    return "dbms_session.sleep" if server_version[0] >= 18 \
            else "dbms_lock.sleep"

def get_server_version():
    name = "SERVER_VERSION"
    value = PARAMETERS.get(name)
    if value is None:
        conn = get_connection()
        value = tuple(int(s) for s in conn.version.split("."))[:2]
        PARAMETERS[name] = value
    return value

def run_sql_script(conn, script_name, **kwargs):
    statement_parts = []
    cursor = conn.cursor()
    replace_values = [("&" + k + ".", v) for k, v in kwargs.items()] + \
                     [("&" + k, v) for k, v in kwargs.items()]
    script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
    file_name = os.path.join(script_dir, "sql", script_name + "_exec.sql")
    for line in open(file_name):
        if line.strip() == "/":
            statement = "".join(statement_parts).strip()
            if statement:
                for search_value, replace_value in replace_values:
                    statement = statement.replace(search_value, replace_value)
                try:
                    cursor.execute(statement)
                except:
                    print("Failed to execute SQL:", statement)
                    raise
            statement_parts = []
        else:
            statement_parts.append(line)
    cursor.execute("""
            select name, type, line, position, text
            from dba_errors
            where owner = upper(:owner)
            order by name, type, line, position""",
            owner = get_main_user())
    prev_name = prev_obj_type = None
    for name, obj_type, line_num, position, text in cursor:
        if name != prev_name or obj_type != prev_obj_type:
            print("%s (%s)" % (name, obj_type))
            prev_name = name
            prev_obj_type = obj_type
        print("    %s/%s %s" % (line_num, position, text))

def run_test_cases():
    print("Running tests for cx_Oracle version", oracledb.version,
            "built at", oracledb.buildtime)
    print("File:", oracledb.__file__)
    print("Client Version:",
            ".".join(str(i) for i in oracledb.clientversion()))
    with get_connection() as connection:
        print("Server Version:", connection.version)
        print()
    unittest.main(testRunner=unittest.TextTestRunner(verbosity=2))

def skip_client_version_old_multi(min_version1, min_version2):
    ver = get_client_version()
    return ver < min_version1 or \
            (ver[0] > min_version1[0] and ver[0] < min_version2[0]) or \
            (ver[0] == min_version2[0] and ver[1] < min_version2[1])

def skip_soda_tests():
    client = get_client_version()
    if client < (18, 3):
        return True
    server = get_server_version()
    if server < (18, 0):
        return True
    if server > (20, 1) and client < (20, 1):
        return True
    return False

class RoundTripInfo:

    def __init__(self, connection):
        self.prev_round_trips = 0
        self.admin_conn = oracledb.connect(get_admin_connect_string())
        with connection.cursor() as cursor:
            cursor.execute("select sys_context('userenv', 'sid') from dual")
            self.sid, = cursor.fetchone()
        self.get_round_trips()

    def get_round_trips(self):
        with self.admin_conn.cursor() as cursor:
            cursor.execute("""
                    select ss.value
                    from v$sesstat ss, v$statname sn
                    where ss.sid = :sid
                      and ss.statistic# = sn.statistic#
                      and sn.name like '%roundtrip%client%'""", sid=self.sid)
            current_round_trips, = cursor.fetchone()
            diff_round_trips = current_round_trips - self.prev_round_trips
            self.prev_round_trips = current_round_trips
            return diff_round_trips

class BaseTestCase(unittest.TestCase):
    requires_connection = True

    def assertRoundTrips(self, n):
        self.assertEqual(self.round_trip_info.get_round_trips(), n)

    def get_db_object_as_plain_object(self, obj):
        if obj.type.iscollection:
            element_values = []
            for value in obj.aslist():
                if isinstance(value, oracledb.Object):
                    value = self.get_db_object_as_plain_object(value)
                elif isinstance(value, oracledb.LOB):
                    value = value.read()
                element_values.append(value)
            return element_values
        attr_values = []
        for attribute in obj.type.attributes:
            value = getattr(obj, attribute.name)
            if isinstance(value, oracledb.Object):
                value = self.get_db_object_as_plain_object(value)
            elif isinstance(value, oracledb.LOB):
                value = value.read()
            attr_values.append(value)
        return tuple(attr_values)

    def get_soda_database(self, minclient=(18, 3), minserver=(18, 0),
                          message="not supported with this client/server " \
                                  "combination"):
        client = get_client_version()
        if client < minclient:
            self.skipTest(message)
        server = get_server_version()
        if server < minserver:
            self.skipTest(message)
        if server > (20, 1) and client < (20, 1):
            self.skipTest(message)
        return self.connection.getSodaDatabase()

    def is_on_oracle_cloud(self, connection=None):
        if connection is None:
            connection = self.connection
        cursor = connection.cursor()
        cursor.execute("""
                select sys_context('userenv', 'service_name')
                from dual""")
        service_name, = cursor.fetchone()
        return service_name.endswith("oraclecloud.com")

    def setUp(self):
        if self.requires_connection:
            self.connection = get_connection()
            self.cursor = self.connection.cursor()

    def setup_round_trip_checker(self):
        self.round_trip_info = RoundTripInfo(self.connection)

    def tearDown(self):
        if self.requires_connection:
            self.connection.close()
            del self.cursor
            del self.connection
