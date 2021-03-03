#------------------------------------------------------------------------------
# soda.py (Section 11.1)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Copyright (c) 2019, 2021, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

import cx_Oracle
import db_config

con = cx_Oracle.connect(db_config.user, db_config.pw, db_config.dsn)

soda = con.getSodaDatabase()

# Explicit metadata is used for maximum version portability
metadata = {
               "keyColumn": {
                   "name":"ID"
               },
               "contentColumn": {
                   "name": "JSON_DOCUMENT",
                   "sqlType": "BLOB"
               },
               "versionColumn": {
                   "name": "VERSION",
                   "method": "UUID"
               },
               "lastModifiedColumn": {
                   "name": "LAST_MODIFIED"
               },
               "creationTimeColumn": {
                   "name": "CREATED_ON"
               }
           }

collection = soda.createCollection("friends", metadata)

content = {'name': 'Jared', 'age': 35, 'address': {'city': 'Melbourne'}}

doc = collection.insertOneAndGet(content)
key = doc.key

doc = collection.find().key(key).getOne()
content = doc.getContent()
print('Retrieved SODA document dictionary is:')
print(content)
