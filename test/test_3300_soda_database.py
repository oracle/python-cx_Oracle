#------------------------------------------------------------------------------
# Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
3300 - Module for testing Simple Oracle Document Access (SODA) Database
"""

import json
import unittest

import cx_Oracle as oracledb
import test_env

@unittest.skipIf(test_env.skip_soda_tests(),
                 "unsupported client/server combination")
class TestCase(test_env.BaseTestCase):

    def __drop_existing_collections(self, soda_db):
        for name in soda_db.getCollectionNames():
            soda_db.openCollection(name).drop()

    def __verify_doc(self, doc, raw_content, str_content=None, content=None,
                     key=None, media_type='application/json'):
        self.assertEqual(doc.getContentAsBytes(), raw_content)
        if str_content is not None:
            self.assertEqual(doc.getContentAsString(), str_content)
        if content is not None:
            self.assertEqual(doc.getContent(), content)
        self.assertEqual(doc.key, key)
        self.assertEqual(doc.mediaType, media_type)

    def test_3300_create_document_with_json(self):
        "3300 - test creating documents with JSON data"
        soda_db = self.connection.getSodaDatabase()
        val = {"testKey1": "testValue1", "testKey2": "testValue2"}
        str_val = json.dumps(val)
        bytes_val = str_val.encode()
        key = "MyKey"
        media_type = "text/plain"
        doc = soda_db.createDocument(val)
        self.__verify_doc(doc, bytes_val, str_val, val)
        doc = soda_db.createDocument(str_val, key)
        self.__verify_doc(doc, bytes_val, str_val, val, key)
        doc = soda_db.createDocument(bytes_val, key, media_type)
        self.__verify_doc(doc, bytes_val, str_val, val, key, media_type)

    def test_3301_create_document_with_raw(self):
        "3301 - test creating documents with raw data"
        soda_db = self.connection.getSodaDatabase()
        val = b"<html/>"
        key = "MyRawKey"
        media_type = "text/html"
        doc = soda_db.createDocument(val)
        self.__verify_doc(doc, val)
        doc = soda_db.createDocument(val, key)
        self.__verify_doc(doc, val, key=key)
        doc = soda_db.createDocument(val, key, media_type)
        self.__verify_doc(doc, val, key=key, media_type=media_type)

    def test_3302_get_collection_names(self):
        "3302 - test getting collection names from the database"
        soda_db = self.connection.getSodaDatabase()
        self.__drop_existing_collections(soda_db)
        self.assertEqual(soda_db.getCollectionNames(), [])
        names = ["zCol", "dCol", "sCol", "aCol", "gCol"]
        sorted_names = list(sorted(names))
        for name in names:
            soda_db.createCollection(name)
        self.assertEqual(soda_db.getCollectionNames(), sorted_names)
        self.assertEqual(soda_db.getCollectionNames(limit=2), sorted_names[:2])
        self.assertEqual(soda_db.getCollectionNames("a"), sorted_names)
        self.assertEqual(soda_db.getCollectionNames("C"), sorted_names)
        self.assertEqual(soda_db.getCollectionNames("b", limit=3),
                                                    sorted_names[1:4])
        self.assertEqual(soda_db.getCollectionNames("z"), sorted_names[-1:])

    def test_3303_open_collection(self):
        "3303 - test opening a collection"
        soda_db = self.connection.getSodaDatabase()
        self.__drop_existing_collections(soda_db)
        coll = soda_db.openCollection("CollectionThatDoesNotExist")
        self.assertEqual(coll, None)
        created_coll = soda_db.createCollection("TestOpenCollection")
        coll = soda_db.openCollection(created_coll.name)
        self.assertEqual(coll.name, created_coll.name)
        coll.drop()

    def test_3304_repr(self):
        "3304 - test SodaDatabase representation"
        con1 = self.connection
        con2 = test_env.get_connection()
        soda_db1 = self.connection.getSodaDatabase()
        soda_db2 = con1.getSodaDatabase()
        soda_db3 = con2.getSodaDatabase()
        self.assertEqual(str(soda_db1), str(soda_db2))
        self.assertEqual(str(soda_db2), str(soda_db3))

    def test_3305_negative(self):
        "3305 - test negative cases for SODA database methods"
        soda_db = self.connection.getSodaDatabase()
        self.assertRaises(TypeError, soda_db.createCollection)
        self.assertRaises(TypeError, soda_db.createCollection, 1)
        self.assertRaises(oracledb.DatabaseError, soda_db.createCollection,
                          None)
        self.assertRaises(TypeError, soda_db.getCollectionNames, 1)

if __name__ == "__main__":
    test_env.run_test_cases()
