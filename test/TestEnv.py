#------------------------------------------------------------------------------
# Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
#
# Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
#
# Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
# Canada. All rights reserved.
#------------------------------------------------------------------------------

"""Define test environment."""

import cx_Oracle
import os
import sys
import unittest

if sys.version_info[0] < 3:
    input = raw_input

def GetValue(name, label, defaultValue = None):
    value = os.environ.get("CX_ORACLE_" + name, defaultValue)
    if value is None:
        value = input(label + ": ")
    return value

USERNAME = GetValue("USERNAME", "user name")
PASSWORD = GetValue("PASSWORD", "password")
TNSENTRY = GetValue("TNSENTRY", "TNS entry")
ENCODING = GetValue("ENCODING", "encoding", "UTF-8")
NENCODING = GetValue("NENCODING", "national encoding", "UTF-8")
ARRAY_SIZE = int(GetValue("ARRAY_SIZE", "array size"))

