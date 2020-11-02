#------------------------------------------------------------------------------
# Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""
3400 - Module for testing Simple Oracle Document Access (SODA) Collections
"""

import TestEnv

import cx_Oracle
import unittest

@unittest.skipIf(TestEnv.SkipSodaTests(),
                 "unsupported client/server combination")
class TestCase(TestEnv.BaseTestCase):

    def __testSkip(self, coll, numToSkip, expectedContent):
        filterSpec = {'$orderby': [{'path': 'name', 'order': 'desc'}]}
        doc = coll.find().filter(filterSpec).skip(numToSkip).getOne()
        content = doc.getContent() if doc is not None else None
        self.assertEqual(content, expectedContent)

    def test_3400_InvalidJson(self):
        "3400 - test inserting invalid JSON value into SODA collection"
        invalidJson = "{testKey:testValue}"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoInvalidJSON")
        doc = sodaDatabase.createDocument(invalidJson)
        self.assertRaises(cx_Oracle.IntegrityError, coll.insertOne, doc)
        coll.drop()

    def test_3401_InsertDocuments(self):
        "3401 - test inserting documents into a SODA collection"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoInsertDocs")
        coll.find().remove()
        valuesToInsert = [
            { "name" : "George", "age" : 47 },
            { "name" : "Susan", "age" : 39 },
            { "name" : "John", "age" : 50 },
            { "name" : "Jill", "age" : 54 }
        ]
        insertedKeys = []
        for value in valuesToInsert:
            doc = coll.insertOneAndGet(value)
            insertedKeys.append(doc.key)
        self.connection.commit()
        self.assertEqual(coll.find().count(), len(valuesToInsert))
        for key, value in zip(insertedKeys, valuesToInsert):
            doc = coll.find().key(key).getOne()
            self.assertEqual(doc.getContent(), value)
        coll.drop()

    def test_3402_SkipDocuments(self):
        "3402 - test skipping documents in a SODA collection"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoSkipDocs")
        coll.find().remove()
        valuesToInsert = [
            { "name" : "Anna", "age" : 62 },
            { "name" : "Mark", "age" : 37 },
            { "name" : "Martha", "age" : 43 },
            { "name" : "Matthew", "age" : 28 }
        ]
        for value in valuesToInsert:
            coll.insertOne(value)
        self.connection.commit()
        self.__testSkip(coll, 0, valuesToInsert[3])
        self.__testSkip(coll, 1, valuesToInsert[2])
        self.__testSkip(coll, 3, valuesToInsert[0])
        self.__testSkip(coll, 4, None)
        self.__testSkip(coll, 125, None)

    def test_3403_ReplaceDocument(self):
        "3403 - test replace documents in SODA collection"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoReplaceDoc")
        coll.find().remove()
        content = {'name': 'John', 'address': {'city': 'Sydney'}}
        doc = coll.insertOneAndGet(content)
        newContent = {'name': 'John', 'address': {'city':'Melbourne'}}
        coll.find().key(doc.key).replaceOne(newContent)
        self.connection.commit()
        self.assertEqual(coll.find().key(doc.key).getOne().getContent(),
                newContent)
        coll.drop()

    def test_3404_SearchDocumentsWithContent(self):
        "3404 - test search documents with content using $like and $regex"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoSearchDocContent")
        coll.find().remove()
        data = [
            {'name': 'John', 'address': {'city': 'Bangalore'}},
            {'name': 'Johnson', 'address': {'city': 'Banaras'}},
            {'name': 'Joseph', 'address': {'city': 'Bangalore'}},
            {'name': 'Jibin', 'address': {'city': 'Secunderabad'}},
            {'name': 'Andrew', 'address': {'city': 'Hyderabad'}},
            {'name': 'Matthew', 'address': {'city': 'Mumbai'}}
        ]
        for value in data:
            coll.insertOne(value)
        self.connection.commit()
        filterSpecs = [
            ({'name': {'$like': 'And%'}}, 1),
            ({'name': {'$like': 'J%n'}}, 3),
            ({'name': {'$like': '%hn%'}}, 2),
            ({'address.city': {'$like': 'Ban%'}}, 3),
            ({'address.city': {'$like': '%bad'}}, 2),
            ({'address.city': {'$like': 'Hyderabad'}}, 1),
            ({'address.city': {'$like': 'China%'}}, 0),
            ({'name': {'$regex': 'Jo.*'}}, 3),
            ({'name': {'$regex': '.*[ho]n'}}, 2),
            ({'name': {'$regex': 'J.*h'}}, 1),
            ({'address.city': {'$regex': 'Ba.*'}}, 3),
            ({'address.city': {'$regex': '.*bad'}}, 2),
            ({'address.city': {'$regex': 'Hyderabad'}}, 1),
            ({'name': {'$regex': 'Js.*n'}}, 0)
        ]
        for filterSpec, expectedCount in filterSpecs:
            self.assertEqual(coll.find().filter(filterSpec).count(),
                    expectedCount, filterSpec)
        coll.drop()

    def test_3405_DocumentRemove(self):
        "3405 - test removing documents"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoRemoveDocs")
        coll.find().remove()
        data = [
            {'name': 'John', 'address': {'city': 'Bangalore'}},
            {'name': 'Johnson', 'address': {'city': 'Banaras'}},
            {'name': 'Joseph', 'address': {'city': 'Mangalore'}},
            {'name': 'Jibin', 'address': {'city': 'Secunderabad'}},
            {'name': 'Andrew', 'address': {'city': 'Hyderabad'}},
            {'name': 'Matthew', 'address': {'city': 'Mumbai'}}
        ]
        docs = [coll.insertOneAndGet(v) for v in data]
        coll.find().key(docs[3].key).remove()
        self.assertEqual(coll.find().count(), len(data) - 1)
        searchResults = coll.find().filter({'name': {'$like': 'Jibin'}})
        self.assertEqual(searchResults.count(), 0)
        coll.find().filter({'name': {'$like': 'John%'}}).remove()
        self.assertEqual(coll.find().count(), len(data) - 3)
        coll.find().filter({'name': {'$regex': 'J.*'}}).remove()
        self.assertEqual(coll.find().count(), len(data) - 4)
        self.connection.commit()
        coll.drop()

    def test_3406_CreateAndDropIndex(self):
        "3406 - test create and drop Index"
        indexName = "cxoTestIndexes_ix_1"
        indexSpec =  {
            'name': indexName,
            'fields': [
                {
                    'path': 'address.city',
                    'datatype': 'string',
                    'order': 'asc'
                }
            ]
        }
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoTestIndexes")
        coll.find().remove()
        self.connection.commit()
        coll.dropIndex(indexName)
        coll.createIndex(indexSpec)
        self.assertRaises(cx_Oracle.DatabaseError, coll.createIndex, indexSpec)
        self.assertEqual(coll.dropIndex(indexName), True)
        self.assertEqual(coll.dropIndex(indexName), False)
        coll.drop()

    def test_3407_GetDocuments(self):
        "3407 - test getting documents from Collection"
        self.connection.autocommit = True
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoTestGetDocs")
        coll.find().remove()
        data = [
            {'name': 'John', 'address': {'city': 'Bangalore'}},
            {'name': 'Johnson', 'address': {'city': 'Banaras'}},
            {'name': 'Joseph', 'address': {'city': 'Mangalore'}},
            {'name': 'Jibin', 'address': {'city': 'Secunderabad'}},
            {'name': 'Andrew', 'address': {'city': 'Hyderabad'}}
        ]
        insertedKeys = list(sorted(coll.insertOneAndGet(v).key for v in data))
        fetchedKeys = list(sorted(d.key for d in coll.find().getDocuments()))
        self.assertEqual(fetchedKeys, insertedKeys)
        coll.drop()

    def test_3408_Cursor(self):
        "3408 - test fetching documents from a cursor"
        self.connection.autocommit = True
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoFindViaCursor")
        coll.find().remove()
        data = [
            {'name': 'John', 'address': {'city': 'Bangalore'}},
            {'name': 'Johnson', 'address': {'city': 'Banaras'}},
            {'name': 'Joseph', 'address': {'city': 'Mangalore'}},
        ]
        insertedKeys = list(sorted(coll.insertOneAndGet(v).key for v in data))
        fetchedKeys = list(sorted(d.key for d in coll.find().getCursor()))
        self.assertEqual(fetchedKeys, insertedKeys)
        coll.drop()

    def test_3409_MultipleDocumentRemove(self):
        "3409 - test removing multiple documents using multiple keys"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoRemoveMultipleDocs")
        coll.find().remove()
        data = [
            {'name': 'John', 'address': {'city': 'Bangalore'}},
            {'name': 'Johnson', 'address': {'city': 'Banaras'}},
            {'name': 'Joseph', 'address': {'city': 'Mangalore'}},
            {'name': 'Jibin', 'address': {'city': 'Secunderabad'}},
            {'name': 'Andrew', 'address': {'city': 'Hyderabad'}},
            {'name': 'Matthew', 'address': {'city': 'Mumbai'}}
        ]
        docs = [coll.insertOneAndGet(v) for v in data]
        keys = [docs[i].key for i in (1, 3, 5)]
        numRemoved = coll.find().keys(keys).remove()
        self.assertEqual(numRemoved, len(keys))
        self.assertEqual(coll.find().count(), len(data) - len(keys))
        self.connection.commit()
        coll.drop()

    def test_3410_DocumentVersion(self):
        "3410 - test using version to get documents and remove them"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoDocumentVersion")
        coll.find().remove()
        content = {'name': 'John', 'address': {'city': 'Bangalore'}}
        insertedDoc = coll.insertOneAndGet(content)
        key = insertedDoc.key
        version = insertedDoc.version
        doc = coll.find().key(key).version(version).getOne()
        self.assertEqual(doc.getContent(), content)
        newContent = {'name': 'James', 'address': {'city': 'Delhi'}}
        replacedDoc = coll.find().key(key).replaceOneAndGet(newContent)
        newVersion = replacedDoc.version
        doc = coll.find().key(key).version(version).getOne()
        self.assertEqual(doc, None)
        doc = coll.find().key(key).version(newVersion).getOne()
        self.assertEqual(doc.getContent(), newContent)
        self.assertEqual(coll.find().key(key).version(version).remove(), 0)
        self.assertEqual(coll.find().key(key).version(newVersion).remove(), 1)
        self.assertEqual(coll.find().count(), 0)
        self.connection.commit()
        coll.drop()

    def test_3411_GetCursor(self):
        "3411 - test keys with GetCursor"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoKeysWithGetCursor")
        coll.find().remove()
        data = [
            {'name': 'John', 'address': {'city': 'Bangalore'}},
            {'name': 'Johnson', 'address': {'city': 'Banaras'}},
            {'name': 'Joseph', 'address': {'city': 'Mangalore'}},
            {'name': 'Jibin', 'address': {'city': 'Secunderabad'}},
            {'name': 'Andrew', 'address': {'city': 'Hyderabad'}},
            {'name': 'Matthew', 'address': {'city': 'Mumbai'}}
        ]
        docs = [coll.insertOneAndGet(v) for v in data]
        keys = [docs[i].key for i in (2, 4, 5)]
        fetchedKeys = [d.key for d in coll.find().keys(keys).getCursor()]
        self.assertEqual(list(sorted(fetchedKeys)), list(sorted(keys)))
        self.connection.commit()
        coll.drop()

    def test_3412_CreatedOn(self):
        "3412 - test createdOn attribute of Document"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoCreatedOn")
        coll.find().remove()
        data = {'name': 'John', 'address': {'city': 'Bangalore'}}
        doc = coll.insertOneAndGet(data)
        self.assertEqual(doc.createdOn, doc.lastModified)

    @unittest.skipIf(TestEnv.GetClientVersion() < (20, 1),
                     "unsupported client")
    def test_3413_SodaTruncate(self):
        "3413 - test Soda truncate"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoTruncateDocs")
        coll.find().remove()
        valuesToInsert = [
            { "name" : "George", "age" : 47 },
            { "name" : "Susan", "age" : 39 },
            { "name" : "John", "age" : 50 },
            { "name" : "Jill", "age" : 54 }
        ]
        for value in valuesToInsert:
            coll.insertOne(value)
        self.connection.commit()
        self.assertEqual(coll.find().count(), len(valuesToInsert))
        coll.truncate()
        self.assertEqual(coll.find().count(), 0)
        coll.drop()

if __name__ == "__main__":
    TestEnv.RunTestCases()
