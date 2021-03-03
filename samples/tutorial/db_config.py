#------------------------------------------------------------------------------
# Copyright (c) 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
import sys
import os
import getpass

######################################################################

#
# Oracle Client library configuration
#

# On Linux this must be None.
# Instead, the Oracle environment must be set before Python starts.
instant_client_dir = None

# On Windows, if your database is on the same machine, comment these lines out
# and let instant_client_dir be None.  Otherwise, set this to your Instant
# Client directory.  Note the use of the raw string r"..."  so backslashes can
# be used as directory separators.
if sys.platform.startswith("win"):
    instant_client_dir = r"c:\oracle\instantclient_19_10"

# On macOS (Intel x86) set the directory to your Instant Client directory
if sys.platform.startswith("darwin"):
    instant_client_dir = os.environ.get("HOME")+"/Downloads/instantclient_19_8"

# This can be called at most once per process.
if instant_client_dir is not None:
    cx_Oracle.init_oracle_client(lib_dir=instant_client_dir)

######################################################################

#
# Tutorial credentials and connection string.
# Environment variable values are used, if they are defined.
#

user = os.environ.get("PYTHON_USER", "pythonhol")

dsn = os.environ.get("PYTHON_CONNECT_STRING", "localhost/orclpdb1")

pw = os.environ.get("PYTHON_PASSWORD")
if pw is None:
    pw = getpass.getpass("Enter password for %s: " % user)
