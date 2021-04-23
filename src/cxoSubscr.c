//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoSubscr.c
//   Defines the routines for handling Oracle subscription information.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoMessageRow_initialize()
//   Initialize a new message row with the information from the descriptor.
//-----------------------------------------------------------------------------
static int cxoMessageRow_initialize(cxoMessageRow *rowObj,
        const char *encoding, dpiSubscrMessageRow *row)
{
    rowObj->operation = row->operation;
    rowObj->rowid = PyUnicode_Decode(row->rowid, row->rowidLength, encoding,
            NULL);
    if (!rowObj->rowid)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// cxoMessageTable_initialize()
//   Initialize a new message table with the information from the descriptor.
//-----------------------------------------------------------------------------
static int cxoMessageTable_initialize(cxoMessageTable *tableObj,
        const char *encoding, dpiSubscrMessageTable *table)
{
    cxoMessageRow *row;
    uint32_t i;

    tableObj->operation = table->operation;
    tableObj->name = PyUnicode_Decode(table->name, table->nameLength, encoding,
            NULL);
    tableObj->rows = PyList_New(table->numRows);
    if (!tableObj->rows)
        return -1;
    for (i = 0; i < table->numRows; i++) {
        row = (cxoMessageRow*)
                cxoPyTypeMessageRow.tp_alloc(&cxoPyTypeMessageRow, 0);
        if (!row)
            return -1;
        PyList_SET_ITEM(tableObj->rows, i, (PyObject*) row);
        if (cxoMessageRow_initialize(row, encoding, &table->rows[i]) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// cxoMessageQuery_initialize()
//   Initialize a new message query with the information from the descriptor.
//-----------------------------------------------------------------------------
static int cxoMessageQuery_initialize(cxoMessageQuery *queryObj,
        const char *encoding, dpiSubscrMessageQuery *query)
{
    cxoMessageTable *table;
    uint32_t i;

    queryObj->id = query->id;
    queryObj->operation = query->operation;
    queryObj->tables = PyList_New(query->numTables);
    if (!queryObj->tables)
        return -1;
    for (i = 0; i < query->numTables; i++) {
        table = (cxoMessageTable*)
                cxoPyTypeMessageTable.tp_alloc(&cxoPyTypeMessageTable, 0);
        if (!table)
            return -1;
        PyList_SET_ITEM(queryObj->tables, i, (PyObject*) table);
        if (cxoMessageTable_initialize(table, encoding, &query->tables[i]) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// cxoMessage_initialize()
//   Initialize a new message with the information from the descriptor.
//-----------------------------------------------------------------------------
static int cxoMessage_initialize(cxoMessage *messageObj,
        cxoSubscr *subscription, dpiSubscrMessage *message)
{
    cxoMessageTable *table;
    cxoMessageQuery *query;
    const char *encoding;
    uint32_t i;

    Py_INCREF(subscription);
    messageObj->subscription = subscription;
    encoding = subscription->connection->encodingInfo.encoding;
    messageObj->type = message->eventType;
    messageObj->registered = message->registered;
    messageObj->dbname = PyUnicode_Decode(message->dbName,
            message->dbNameLength, encoding, NULL);
    if (!messageObj->dbname)
        return -1;
    if (message->txId) {
        messageObj->txId = PyBytes_FromStringAndSize(message->txId,
                message->txIdLength);
        if (!messageObj->txId)
            return -1;
    }
    if (message->queueName) {
        messageObj->queueName = PyUnicode_Decode(message->queueName,
                message->queueNameLength, encoding, NULL);
        if (!messageObj->queueName)
            return -1;
    }
    if (message->consumerName) {
        messageObj->consumerName = PyUnicode_Decode(message->consumerName,
                message->consumerNameLength, encoding, NULL);
        if (!messageObj->consumerName)
            return -1;
    }
    switch (message->eventType) {
        case DPI_EVENT_OBJCHANGE:
            messageObj->tables = PyList_New(message->numTables);
            if (!messageObj->tables)
                return -1;
            for (i = 0; i < message->numTables; i++) {
                table = (cxoMessageTable*)
                        cxoPyTypeMessageTable.tp_alloc(&cxoPyTypeMessageTable,
                                0);
                if (!table)
                    return -1;
                PyList_SET_ITEM(messageObj->tables, i, (PyObject*) table);
                if (cxoMessageTable_initialize(table, encoding,
                        &message->tables[i]) < 0)
                    return -1;
            }
            break;
        case DPI_EVENT_QUERYCHANGE:
            messageObj->queries = PyList_New(message->numQueries);
            if (!messageObj->queries)
                return -1;
            for (i = 0; i < message->numQueries; i++) {
                query = (cxoMessageQuery*)
                        cxoPyTypeMessageQuery.tp_alloc(&cxoPyTypeMessageQuery,
                                0);
                if (!query)
                    return -1;
                PyList_SET_ITEM(messageObj->queries, i, (PyObject*) query);
                if (cxoMessageQuery_initialize(query, encoding,
                        &message->queries[i]) < 0)
                    return -1;
            }
            break;
        default:
            break;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// cxoSubscr_callbackHandler()
//   Routine that performs the actual call.
//-----------------------------------------------------------------------------
static int cxoSubscr_callbackHandler(cxoSubscr *subscr,
        dpiSubscrMessage *message)
{
    PyObject *result, *args;
    cxoMessage *messageObj;

    // create the message
    messageObj = (cxoMessage*) cxoPyTypeMessage.tp_alloc(&cxoPyTypeMessage, 0);
    if (!messageObj)
        return -1;
    if (cxoMessage_initialize(messageObj, subscr, message) < 0) {
        Py_DECREF(messageObj);
        return -1;
    }

    // create the arguments for the call
    args = PyTuple_Pack(1, messageObj);
    Py_DECREF(messageObj);
    if (!args)
        return -1;

    // make the actual call
    result = PyObject_Call(subscr->callback, args, NULL);
    Py_DECREF(args);
    if (!result)
        return -1;
    Py_DECREF(result);

    return 0;
}


//-----------------------------------------------------------------------------
// cxoSubscr_callback()
//   Routine that is called when a callback needs to be invoked.
//-----------------------------------------------------------------------------
void cxoSubscr_callback(cxoSubscr *subscr, dpiSubscrMessage *message)
{
#ifdef WITH_THREAD
    PyGILState_STATE gstate = PyGILState_Ensure();
#endif

    if (message->errorInfo) {
        cxoError_raiseFromInfo(message->errorInfo);
        PyErr_Print();
    } else if (cxoSubscr_callbackHandler(subscr, message) < 0)
        PyErr_Print();

#ifdef WITH_THREAD
    PyGILState_Release(gstate);
#endif
}


//-----------------------------------------------------------------------------
// cxoSubscr_free()
//   Free the memory associated with a subscription.
//-----------------------------------------------------------------------------
static void cxoSubscr_free(cxoSubscr *subscr)
{
    if (subscr->handle) {
        dpiSubscr_release(subscr->handle);
        subscr->handle = NULL;
    }
    Py_CLEAR(subscr->connection);
    Py_CLEAR(subscr->callback);
    Py_CLEAR(subscr->name);
    Py_CLEAR(subscr->ipAddress);
    Py_TYPE(subscr)->tp_free((PyObject*) subscr);
}


//-----------------------------------------------------------------------------
// cxoSubscr_repr()
//   Return a string representation of the subscription.
//-----------------------------------------------------------------------------
static PyObject *cxoSubscr_repr(cxoSubscr *subscription)
{
    PyObject *connectionRepr, *module, *name, *result;

    connectionRepr = PyObject_Repr((PyObject*) subscription->connection);
    if (!connectionRepr)
        return NULL;
    if (cxoUtils_getModuleAndName(Py_TYPE(subscription), &module, &name) < 0) {
        Py_DECREF(connectionRepr);
        return NULL;
    }
    result = cxoUtils_formatString("<%s.%s on %s>",
            PyTuple_Pack(3, module, name, connectionRepr));
    Py_DECREF(module);
    Py_DECREF(name);
    Py_DECREF(connectionRepr);
    return result;
}


//-----------------------------------------------------------------------------
// cxoSubscr_registerQuery()
//   Register a query for database change notification.
//-----------------------------------------------------------------------------
static PyObject *cxoSubscr_registerQuery(cxoSubscr *subscr,
        PyObject *args)
{
    PyObject *statement, *executeArgs;
    cxoBuffer statementBuffer;
    uint32_t numQueryColumns;
    cxoCursor *cursor;
    uint64_t queryId;
    int status;

    // parse arguments
    executeArgs = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &statement, &executeArgs))
        return NULL;
    if (executeArgs) {
        if (!PyDict_Check(executeArgs) && !PySequence_Check(executeArgs)) {
            PyErr_SetString(PyExc_TypeError,
                    "expecting a dictionary or sequence");
            return NULL;
        }
    }

    // create cursor to perform query
    cursor = (cxoCursor*) PyObject_CallMethod((PyObject*) subscr->connection,
            "cursor", NULL);
    if (!cursor)
        return NULL;

    // prepare the statement for execution
    if (cxoBuffer_fromObject(&statementBuffer, statement,
            subscr->connection->encodingInfo.encoding) < 0) {
        Py_DECREF(cursor);
        return NULL;
    }
    status = dpiSubscr_prepareStmt(subscr->handle, statementBuffer.ptr,
            statementBuffer.size, &cursor->handle);
    cxoBuffer_clear(&statementBuffer);
    if (status < 0) {
        cxoError_raiseAndReturnNull();
        Py_DECREF(cursor);
        return NULL;
    }

    // perform binds
    if (executeArgs && cxoCursor_setBindVariables(cursor, executeArgs, 1, 0,
            0) < 0) {
        Py_DECREF(cursor);
        return NULL;
    }
    if (cxoCursor_performBind(cursor) < 0) {
        Py_DECREF(cursor);
        return NULL;
    }

    // perform the execute (which registers the query)
    Py_BEGIN_ALLOW_THREADS
    status = dpiStmt_execute(cursor->handle, DPI_MODE_EXEC_DEFAULT,
            &numQueryColumns);
    Py_END_ALLOW_THREADS
    if (status < 0) {
        cxoError_raiseAndReturnNull();
        Py_DECREF(cursor);
        return NULL;
    }

    // return the query id, if applicable
    if (subscr->qos & DPI_SUBSCR_QOS_QUERY) {
        if (dpiStmt_getSubscrQueryId(cursor->handle, &queryId) < 0) {
            cxoError_raiseAndReturnNull();
            Py_DECREF(cursor);
            return NULL;
        }
        Py_DECREF(cursor);
        return PyLong_FromLong((long) queryId);
    }

    Py_DECREF(cursor);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoMessage_free()
//   Free the memory associated with a message.
//-----------------------------------------------------------------------------
static void cxoMessage_free(cxoMessage *message)
{
    Py_CLEAR(message->subscription);
    Py_CLEAR(message->dbname);
    Py_CLEAR(message->txId);
    Py_CLEAR(message->tables);
    Py_CLEAR(message->queries);
    Py_CLEAR(message->queueName);
    Py_CLEAR(message->consumerName);
    Py_TYPE(message)->tp_free((PyObject*) message);
}


//-----------------------------------------------------------------------------
// cxoMessageQuery_free()
//   Free the memory associated with a query in a message.
//-----------------------------------------------------------------------------
static void cxoMessageQuery_free(cxoMessageQuery *query)
{
    Py_CLEAR(query->tables);
    Py_TYPE(query)->tp_free((PyObject*) query);
}


//-----------------------------------------------------------------------------
// cxoMessageRow_free()
//   Free the memory associated with a row in a message.
//-----------------------------------------------------------------------------
static void cxoMessageRow_free(cxoMessageRow *row)
{
    Py_CLEAR(row->rowid);
    Py_TYPE(row)->tp_free((PyObject*) row);
}


//-----------------------------------------------------------------------------
// cxoMessageTable_free()
//   Free the memory associated with a table in a message.
//-----------------------------------------------------------------------------
static void cxoMessageTable_free(cxoMessageTable *table)
{
    Py_CLEAR(table->name);
    Py_CLEAR(table->rows);
    Py_TYPE(table)->tp_free((PyObject*) table);
}


//-----------------------------------------------------------------------------
// declaration of members for Python types
//-----------------------------------------------------------------------------
static PyMemberDef cxoSubscrTypeMembers[] = {
    { "callback", T_OBJECT, offsetof(cxoSubscr, callback), READONLY },
    { "connection", T_OBJECT, offsetof(cxoSubscr, connection),
            READONLY },
    { "namespace", T_UINT, offsetof(cxoSubscr, namespace), READONLY },
    { "name", T_OBJECT, offsetof(cxoSubscr, name), READONLY },
    { "protocol", T_UINT, offsetof(cxoSubscr, protocol), READONLY },
    { "ip_address", T_OBJECT, offsetof(cxoSubscr, ipAddress), READONLY },
    { "port", T_UINT, offsetof(cxoSubscr, port), READONLY },
    { "timeout", T_UINT, offsetof(cxoSubscr, timeout), READONLY },
    { "operations", T_UINT, offsetof(cxoSubscr, operations), READONLY },
    { "qos", T_UINT, offsetof(cxoSubscr, qos), READONLY },
    { "id", T_ULONG, offsetof(cxoSubscr, id), READONLY },
    // deprecated
    { "ipAddress", T_OBJECT, offsetof(cxoSubscr, ipAddress), READONLY },
    { NULL }
};

static PyMemberDef cxoMessageTypeMembers[] = {
    { "subscription", T_OBJECT, offsetof(cxoMessage, subscription),
            READONLY },
    { "type", T_INT, offsetof(cxoMessage, type), READONLY },
    { "dbname", T_OBJECT, offsetof(cxoMessage, dbname), READONLY },
    { "txid", T_OBJECT, offsetof(cxoMessage, txId), READONLY },
    { "tables", T_OBJECT, offsetof(cxoMessage, tables), READONLY },
    { "queries", T_OBJECT, offsetof(cxoMessage, queries), READONLY },
    { "queue_name", T_OBJECT, offsetof(cxoMessage, queueName), READONLY },
    { "consumer_name", T_OBJECT, offsetof(cxoMessage, consumerName),
            READONLY },
    { "registered", T_BOOL, offsetof(cxoMessage, registered), READONLY },
    // deprecated
    { "queueName", T_OBJECT, offsetof(cxoMessage, queueName), READONLY },
    { "consumerName", T_OBJECT, offsetof(cxoMessage, consumerName), READONLY },
    { NULL }
};

static PyMemberDef cxoMessageTableTypeMembers[] = {
    { "name", T_OBJECT, offsetof(cxoMessageTable, name), READONLY },
    { "rows", T_OBJECT, offsetof(cxoMessageTable, rows), READONLY },
    { "operation", T_INT, offsetof(cxoMessageTable, operation), READONLY },
    { NULL }
};

static PyMemberDef cxoMessageRowTypeMembers[] = {
    { "rowid", T_OBJECT, offsetof(cxoMessageRow, rowid), READONLY },
    { "operation", T_INT, offsetof(cxoMessageRow, operation), READONLY },
    { NULL }
};

static PyMemberDef cxoMessageQueryTypeMembers[] = {
    { "id", T_INT, offsetof(cxoMessageQuery, id), READONLY },
    { "operation", T_INT, offsetof(cxoMessageQuery, operation), READONLY },
    { "tables", T_OBJECT, offsetof(cxoMessageQuery, tables), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of methods for Python types
//-----------------------------------------------------------------------------
static PyMethodDef cxoSubscrTypeMethods[] = {
    { "registerquery", (PyCFunction) cxoSubscr_registerQuery,
            METH_VARARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeSubscr = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.Subscription",
    .tp_basicsize = sizeof(cxoSubscr),
    .tp_dealloc = (destructor) cxoSubscr_free,
    .tp_repr = (reprfunc) cxoSubscr_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = cxoSubscrTypeMethods,
    .tp_members = cxoSubscrTypeMembers
};

PyTypeObject cxoPyTypeMessage = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.Message",
    .tp_basicsize = sizeof(cxoMessage),
    .tp_dealloc = (destructor) cxoMessage_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMessageTypeMembers
};

PyTypeObject cxoPyTypeMessageTable = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.MessageTable",
    .tp_basicsize = sizeof(cxoMessageTable),
    .tp_dealloc = (destructor) cxoMessageTable_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMessageTableTypeMembers
};


PyTypeObject cxoPyTypeMessageRow = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.MessageRow",
    .tp_basicsize = sizeof(cxoMessageRow),
    .tp_dealloc = (destructor) cxoMessageRow_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMessageRowTypeMembers
};


PyTypeObject cxoPyTypeMessageQuery = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.MessageQuery",
    .tp_basicsize = sizeof(cxoMessageQuery),
    .tp_dealloc = (destructor) cxoMessageQuery_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMessageQueryTypeMembers
};
