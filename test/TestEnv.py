"""Define test environment."""

import cx_Oracle
import os
import unittest

def GetValue(name, label):
    value = os.environ.get("CX_ORACLE_" + name)
    if value is None:
        value = raw_input(label + ": ")
    return value

USERNAME = GetValue("USERNAME", "user name")
PASSWORD = GetValue("PASSWORD", "password")
TNSENTRY = GetValue("TNSENTRY", "TNS entry")
ARRAY_SIZE = int(GetValue("ARRAY_SIZE", "array size"))

