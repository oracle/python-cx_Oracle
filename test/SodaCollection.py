#------------------------------------------------------------------------------
# Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
#------------------------------------------------------------------------------

"""Module for testing Simple Oracle Document Access (SODA) Collections"""

class TestSodaDocuments(BaseTestCase):

    def testInvalidJson(self):
        "test inserting invalid JSON value into SODA collection"
        invalidJson = "{testKey:testValue}"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoInvalidJSON")
        doc = sodaDatabase.createDocument(invalidJson)
        self.assertRaises(cx_Oracle.IntegrityError, coll.insertOne, doc)
        coll.drop()

    def testInsertDocuments(self):
        "test inserting documents into a SODA collection"
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

    def testSkipDocuments(self):
        "test skip documents from SODA collection"
        sodaDatabase = self.connection.getSodaDatabase()
        coll = sodaDatabase.createCollection("cxoSkipDocs")
        coll.find().remove()
        valuesToInsert = [
            { "name" : "Matthew", "age" : 28 },
            { "name" : "Martha", "age" : 43 },
            { "name" : "Mark", "age" : 37 },
            { "name" : "Anna", "age" : 62 }
        ]
        for value in valuesToInsert:
            coll.insertOne(value)
        self.connection.commit()
        self.assertEqual(coll.find().skip(1).getOne().getContent(),
                valuesToInsert[1])
        self.assertEqual(coll.find().skip(3).getOne().getContent(),
                valuesToInsert[3])
        self.assertEqual(coll.find().skip(4).getOne(), None)
        self.assertEqual(coll.find().skip(125).getOne(), None)

    def testReplaceDocument(self):
        "test replace documents in SODA collection"
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

    def testSearchDocumentsWithContent(self):
        "test search documents with content using $like and $regex"
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

    def testDocumentRemove(self):
        "test removing documents"
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

    def testCreateAndDropIndex(self):
        "test create and drop Index"
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

    def testGetDocuments(self):
        "test getting documents from Collection"
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

    def testCursor(self):
        "test fetching documents from a cursor"
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

