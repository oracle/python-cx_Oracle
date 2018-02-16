#------------------------------------------------------------------------------
# connect_drcp.py (Section 2.3 and 2.5)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright 2017, 2018, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

from __future__ import print_function

import cx_Oracle

con = cx_Oracle.connect("pythonhol", "welcome", "localhost/orclpdb:pooled",
                        cclass="PYTHONHOL", purity=cx_Oracle.ATTR_PURITY_SELF)
print("Database version:", con.version)
