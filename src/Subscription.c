//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subscription.c
//   Defines the routines for handling Oracle subscription information.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structures used for handling subscriptions
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiSubscr *handle;
    udt_Connection *connection;
    PyObject *callback;
    uint32_t namespace;
    uint32_t protocol;
    uint32_t port;
    uint32_t timeout;
    uint32_t operations;
    uint32_t qos;
    uint32_t id;
} udt_Subscription;

typedef struct {
    PyObject_HEAD
    udt_Subscription *subscription;
    dpiEventType type;
    PyObject *dbname;
    PyObject *tables;
    PyObject *queries;
} udt_Message;

typedef struct {
    PyObject_HEAD
    PyObject *name;
    PyObject *rows;
    dpiOpCode operation;
} udt_MessageTable;

typedef struct {
    PyObject_HEAD
    PyObject *rowid;
    dpiOpCode operation;
} udt_MessageRow;

typedef struct {
    PyObject_HEAD
    uint64_t id;
    dpiOpCode operation;
    PyObject *tables;
} udt_MessageQuery;


//-----------------------------------------------------------------------------
// Declaration of subscription functions
//-----------------------------------------------------------------------------
static void Subscription_Free(udt_Subscription*);
static PyObject *Subscription_Repr(udt_Subscription*);
static PyObject *Subscription_RegisterQuery(udt_Subscription*, PyObject*);
static void Message_Free(udt_Message*);
static void MessageTable_Free(udt_MessageTable*);
static void MessageRow_Free(udt_MessageRow*);
static void MessageQuery_Free(udt_MessageQuery*);

//-----------------------------------------------------------------------------
// declaration of members for Python types
//-----------------------------------------------------------------------------
static PyMemberDef g_SubscriptionTypeMembers[] = {
    { "callback", T_OBJECT, offsetof(udt_Subscription, callback), READONLY },
    { "connection", T_OBJECT, offsetof(udt_Subscription, connection),
            READONLY },
    { "namespace", T_INT, offsetof(udt_Subscription, namespace), READONLY },
    { "protocol", T_INT, offsetof(udt_Subscription, protocol), READONLY },
    { "port", T_INT, offsetof(udt_Subscription, port), READONLY },
    { "timeout", T_INT, offsetof(udt_Subscription, timeout), READONLY },
    { "operations", T_INT, offsetof(udt_Subscription, operations), READONLY },
    { "qos", T_INT, offsetof(udt_Subscription, qos), READONLY },
    { "id", T_INT, offsetof(udt_Subscription, id), READONLY },
    { NULL }
};

static PyMemberDef g_MessageTypeMembers[] = {
    { "subscription", T_OBJECT, offsetof(udt_Message, subscription),
            READONLY },
    { "type", T_INT, offsetof(udt_Message, type), READONLY },
    { "dbname", T_OBJECT, offsetof(udt_Message, dbname), READONLY },
    { "tables", T_OBJECT, offsetof(udt_Message, tables), READONLY },
    { "queries", T_OBJECT, offsetof(udt_Message, queries), READONLY },
    { NULL }
};

static PyMemberDef g_MessageTableTypeMembers[] = {
    { "name", T_OBJECT, offsetof(udt_MessageTable, name), READONLY },
    { "rows", T_OBJECT, offsetof(udt_MessageTable, rows), READONLY },
    { "operation", T_INT, offsetof(udt_MessageTable, operation), READONLY },
    { NULL }
};

static PyMemberDef g_MessageRowTypeMembers[] = {
    { "rowid", T_OBJECT, offsetof(udt_MessageRow, rowid), READONLY },
    { "operation", T_INT, offsetof(udt_MessageRow, operation), READONLY },
    { NULL }
};

static PyMemberDef g_MessageQueryTypeMembers[] = {
    { "id", T_INT, offsetof(udt_MessageQuery, id), READONLY },
    { "operation", T_INT, offsetof(udt_MessageQuery, operation), READONLY },
    { "tables", T_OBJECT, offsetof(udt_MessageQuery, tables), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of methods for Python types
//-----------------------------------------------------------------------------
static PyMethodDef g_SubscriptionTypeMethods[] = {
    { "registerquery", (PyCFunction) Subscription_RegisterQuery,
            METH_VARARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_SubscriptionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.Subscription",           // tp_name
    sizeof(udt_Subscription),           // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Subscription_Free,     // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Subscription_Repr,       // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    g_SubscriptionTypeMethods,          // tp_methods
    g_SubscriptionTypeMembers,          // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};

static PyTypeObject g_MessageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.Message",                // tp_name
    sizeof(udt_Message),                // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Message_Free,          // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    g_MessageTypeMembers,               // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};

static PyTypeObject g_MessageTableType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.MessageTable",           // tp_name
    sizeof(udt_MessageTable),           // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) MessageTable_Free,     // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    g_MessageTableTypeMembers,          // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


static PyTypeObject g_MessageRowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.MessageRow",             // tp_name
    sizeof(udt_MessageRow),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) MessageRow_Free,       // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    g_MessageRowTypeMembers,            // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


static PyTypeObject g_MessageQueryType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.MessageQuery",           // tp_name
    sizeof(udt_MessageQuery),           // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) MessageQuery_Free,     // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    0,                                  // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    g_MessageQueryTypeMembers,          // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// MessageRow_Initialize()
//   Initialize a new message row with the information from the descriptor.
//-----------------------------------------------------------------------------
static int MessageRow_Initialize(udt_MessageRow *self, const char *encoding,
        dpiSubscrMessageRow *row)
{
    self->operation = row->operation;
    self->rowid = cxString_FromEncodedString(row->rowid, row->rowidLength,
            encoding);
    if (!self->rowid)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// MessageTable_Initialize()
//   Initialize a new message table with the information from the descriptor.
//-----------------------------------------------------------------------------
static int MessageTable_Initialize(udt_MessageTable *self,
        const char *encoding, dpiSubscrMessageTable *table)
{
    udt_MessageRow *row;
    uint32_t i;

    self->operation = table->operation;
    self->name = cxString_FromEncodedString(table->name, table->nameLength,
            encoding);
    self->rows = PyList_New(table->numRows);
    if (!self->rows)
        return -1;
    for (i = 0; i < table->numRows; i++) {
        row = (udt_MessageRow*)
                g_MessageRowType.tp_alloc(&g_MessageRowType, 0);
        if (!row)
            return -1;
        PyList_SET_ITEM(self->rows, i, (PyObject*) row);
        if (MessageRow_Initialize(row, encoding, &table->rows[i]) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// MessageQuery_Initialize()
//   Initialize a new message query with the information from the descriptor.
//-----------------------------------------------------------------------------
static int MessageQuery_Initialize(udt_MessageQuery *self,
        const char *encoding, dpiSubscrMessageQuery *query)
{
    udt_MessageTable *table;
    uint32_t i;

    self->id = query->id;
    self->operation = query->operation;
    self->tables = PyList_New(query->numTables);
    if (!self->tables)
        return -1;
    for (i = 0; i < query->numTables; i++) {
        table = (udt_MessageTable*)
                g_MessageTableType.tp_alloc(&g_MessageTableType, 0);
        if (!table)
            return -1;
        PyList_SET_ITEM(self->tables, i, (PyObject*) table);
        if (MessageTable_Initialize(table, encoding, &query->tables[i]) < 0)
            return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Message_Initialize()
//   Initialize a new message with the information from the descriptor.
//-----------------------------------------------------------------------------
static int Message_Initialize(udt_Message *self,
        udt_Subscription *subscription, dpiSubscrMessage *message)
{
    udt_MessageTable *table;
    udt_MessageQuery *query;
    const char *encoding;
    uint32_t i;

    Py_INCREF(subscription);
    self->subscription = subscription;
    encoding = subscription->connection->encodingInfo.encoding;
    self->type = message->eventType;
    self->dbname = cxString_FromEncodedString(message->dbName,
            message->dbNameLength, encoding);
    if (!self->dbname)
        return -1;
    switch (message->eventType) {
        case DPI_EVENT_OBJCHANGE:
            self->tables = PyList_New(message->numTables);
            if (!self->tables)
                return -1;
            for (i = 0; i < message->numTables; i++) {
                table = (udt_MessageTable*)
                        g_MessageTableType.tp_alloc(&g_MessageTableType, 0);
                if (!table)
                    return -1;
                PyList_SET_ITEM(self->tables, i, (PyObject*) table);
                if (MessageTable_Initialize(table, encoding,
                        &message->tables[i]) < 0)
                    return -1;
            }
            break;
        case DPI_EVENT_QUERYCHANGE:
            self->queries = PyList_New(message->numQueries);
            if (!self->queries)
                return -1;
            for (i = 0; i < message->numQueries; i++) {
                query = (udt_MessageQuery*)
                        g_MessageQueryType.tp_alloc(&g_MessageQueryType, 0);
                if (!query)
                    return -1;
                PyList_SET_ITEM(self->queries, i, (PyObject*) query);
                if (MessageQuery_Initialize(query, encoding,
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
// Subscription_CallbackHandler()
//   Routine that performs the actual call.
//-----------------------------------------------------------------------------
static int Subscription_CallbackHandler(udt_Subscription *self,
        dpiSubscrMessage *message)
{
    PyObject *result, *args;
    udt_Message *messageObj;

    // create the message
    messageObj = (udt_Message*) g_MessageType.tp_alloc(&g_MessageType, 0);
    if (!messageObj)
        return -1;
    if (Message_Initialize(messageObj, self, message) < 0) {
        Py_DECREF(messageObj);
        return -1;
    }

    // create the arguments for the call
    args = PyTuple_Pack(1, messageObj);
    Py_DECREF(messageObj);
    if (!args)
        return -1;

    // make the actual call
    result = PyObject_Call(self->callback, args, NULL);
    Py_DECREF(args);
    if (!result)
        return -1;
    Py_DECREF(result);

    return 0;
}


//-----------------------------------------------------------------------------
// Subscription_Callback()
//   Routine that is called when a callback needs to be invoked.
//-----------------------------------------------------------------------------
static void Subscription_Callback(udt_Subscription *self,
        dpiSubscrMessage *message)
{
#ifdef WITH_THREAD
    PyGILState_STATE gstate = PyGILState_Ensure();
#endif

    if (message->errorInfo) {
        Error_RaiseFromInfo(message->errorInfo);
        PyErr_Print();
    } else if (Subscription_CallbackHandler(self, message) < 0)
        PyErr_Print();

#ifdef WITH_THREAD
    PyGILState_Release(gstate);
#endif
}


//-----------------------------------------------------------------------------
// Subscription_New()
//   Allocate a new subscription object.
//-----------------------------------------------------------------------------
static udt_Subscription *Subscription_New(udt_Connection *connection,
        uint32_t namespace, uint32_t protocol, uint32_t port,
        PyObject *callback, uint32_t timeout, uint32_t operations,
        uint32_t qos)
{
    dpiSubscrCreateParams params;
    udt_Subscription *self;

    self = (udt_Subscription*)
            g_SubscriptionType.tp_alloc(&g_SubscriptionType, 0);
    if (!self)
        return NULL;
    Py_INCREF(connection);
    self->connection = connection;
    Py_XINCREF(callback);
    self->callback = callback;
    self->namespace = namespace;
    self->protocol = protocol;
    self->port = port;
    self->timeout = timeout;
    self->operations = operations;
    self->qos = qos;

    if (dpiContext_initSubscrCreateParams(g_DpiContext, &params) < 0) {
        Error_RaiseAndReturnNull();
        return NULL;
    }
    params.subscrNamespace = namespace;
    params.protocol = protocol;
    params.portNumber = port;
    if (callback) {
        params.callback = (dpiSubscrCallback) Subscription_Callback;
        params.callbackContext = self;
    }
    params.timeout = timeout;
    params.operations = operations;
    params.qos = qos;
    if (dpiConn_newSubscription(connection->handle, &params, &self->handle,
            &self->id) < 0) {
        Error_RaiseAndReturnNull();
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// Subscription_Free()
//   Free the memory associated with a subscription.
//-----------------------------------------------------------------------------
static void Subscription_Free(udt_Subscription *self)
{
    if (self->handle) {
        dpiSubscr_release(self->handle);
        self->handle = NULL;
    }
    Py_CLEAR(self->connection);
    Py_CLEAR(self->callback);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Subscription_Repr()
//   Return a string representation of the subscription.
//-----------------------------------------------------------------------------
static PyObject *Subscription_Repr(udt_Subscription *subscription)
{
    PyObject *connectionRepr, *module, *name, *result, *format, *formatArgs;

    format = cxString_FromAscii("<%s.%s on %s>");
    if (!format)
        return NULL;
    connectionRepr = PyObject_Repr((PyObject*) subscription->connection);
    if (!connectionRepr) {
        Py_DECREF(format);
        return NULL;
    }
    if (GetModuleAndName(Py_TYPE(subscription), &module, &name) < 0) {
        Py_DECREF(format);
        Py_DECREF(connectionRepr);
        return NULL;
    }
    formatArgs = PyTuple_Pack(3, module, name, connectionRepr);
    Py_DECREF(module);
    Py_DECREF(name);
    Py_DECREF(connectionRepr);
    if (!formatArgs) {
        Py_DECREF(format);
        return NULL;
    }
    result = cxString_Format(format, formatArgs);
    Py_DECREF(format);
    Py_DECREF(formatArgs);
    return result;
}


//-----------------------------------------------------------------------------
// Subscription_RegisterQuery()
//   Register a query for database change notification.
//-----------------------------------------------------------------------------
static PyObject *Subscription_RegisterQuery(udt_Subscription *self,
        PyObject *args)
{
    PyObject *statement, *executeArgs;
    udt_Buffer statementBuffer;
    uint32_t numQueryColumns;
    udt_Cursor *cursor;
    uint64_t queryId;
    int status;

    // parse arguments
    executeArgs = NULL;
    if (!PyArg_ParseTuple(args, "O!|O", cxString_Type, &statement,
            &executeArgs))
        return NULL;
    if (executeArgs) {
        if (!PyDict_Check(executeArgs) && !PySequence_Check(executeArgs)) {
            PyErr_SetString(PyExc_TypeError,
                    "expecting a dictionary or sequence");
            return NULL;
        }
    }

    // create cursor to perform query
    cursor = (udt_Cursor*) PyObject_CallMethod((PyObject*) self->connection,
            "cursor", NULL);
    if (!cursor)
        return NULL;

    // prepare the statement for execution
    if (cxBuffer_FromObject(&statementBuffer, statement,
            self->connection->encodingInfo.encoding) < 0) {
        Py_DECREF(cursor);
        return NULL;
    }
    status = dpiSubscr_prepareStmt(self->handle, statementBuffer.ptr,
            statementBuffer.size, &cursor->handle);
    cxBuffer_Clear(&statementBuffer);
    if (status < 0) {
        Error_RaiseAndReturnNull();
        Py_DECREF(cursor);
        return NULL;
    }

    // perform binds
    if (executeArgs && Cursor_SetBindVariables(cursor, executeArgs, 1, 0,
            0) < 0) {
        Py_DECREF(cursor);
        return NULL;
    }
    if (Cursor_PerformBind(cursor) < 0) {
        Py_DECREF(cursor);
        return NULL;
    }

    // perform the execute (which registers the query)
    Py_BEGIN_ALLOW_THREADS
    status = dpiStmt_execute(cursor->handle, DPI_MODE_EXEC_DEFAULT,
            &numQueryColumns);
    Py_END_ALLOW_THREADS
    if (status < 0) {
        Error_RaiseAndReturnNull();
        Py_DECREF(cursor);
        return NULL;
    }

    // return the query id, if applicable
    if (self->qos & DPI_SUBSCR_QOS_QUERY) {
        if (dpiStmt_getSubscrQueryId(cursor->handle, &queryId) < 0) {
            Error_RaiseAndReturnNull();
            Py_DECREF(cursor);
            return NULL;
        }
        Py_DECREF(cursor);
        return PyInt_FromLong((long) queryId);
    }

    Py_DECREF(cursor);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Message_Free()
//   Free the memory associated with a message.
//-----------------------------------------------------------------------------
static void Message_Free(udt_Message *self)
{
    Py_CLEAR(self->subscription);
    Py_CLEAR(self->dbname);
    Py_CLEAR(self->tables);
    Py_CLEAR(self->queries);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// MessageTable_Free()
//   Free the memory associated with a table in a message.
//-----------------------------------------------------------------------------
static void MessageTable_Free(udt_MessageTable *self)
{
    Py_CLEAR(self->name);
    Py_CLEAR(self->rows);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// MessageRow_Free()
//   Free the memory associated with a row in a message.
//-----------------------------------------------------------------------------
static void MessageRow_Free(udt_MessageRow *self)
{
    Py_CLEAR(self->rowid);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// MessageQuery_Free()
//   Free the memory associated with a query in a message.
//-----------------------------------------------------------------------------
static void MessageQuery_Free(udt_MessageQuery *self)
{
    Py_CLEAR(self->tables);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

