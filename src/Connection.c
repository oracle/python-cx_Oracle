//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Connection.c
//   Definition of the Python type OracleConnection.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type "Connection"
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiConn *handle;
    udt_SessionPool *sessionPool;
    PyObject *inputTypeHandler;
    PyObject *outputTypeHandler;
    PyObject *username;
    PyObject *dsn;
    PyObject *version;
    dpiEncodingInfo encodingInfo;
    int autocommit;
} udt_Connection;


//-----------------------------------------------------------------------------
// functions for the Python type "Connection"
//-----------------------------------------------------------------------------
static void Connection_Free(udt_Connection*);
static PyObject *Connection_New(PyTypeObject*, PyObject*, PyObject*);
static int Connection_Init(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_Repr(udt_Connection*);
static PyObject *Connection_Close(udt_Connection*, PyObject*);
static PyObject *Connection_Commit(udt_Connection*, PyObject*);
static PyObject *Connection_Begin(udt_Connection*, PyObject*);
static PyObject *Connection_Prepare(udt_Connection*, PyObject*);
static PyObject *Connection_Rollback(udt_Connection*, PyObject*);
static PyObject *Connection_NewCursor(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_Cancel(udt_Connection*, PyObject*);
static PyObject *Connection_GetVersion(udt_Connection*, void*);
static PyObject *Connection_GetEncoding(udt_Connection*, void*);
static PyObject *Connection_GetNationalEncoding(udt_Connection*, void*);
static PyObject *Connection_GetMaxBytesPerCharacter(udt_Connection*, void*);
static PyObject *Connection_ContextManagerEnter(udt_Connection*, PyObject*);
static PyObject *Connection_ContextManagerExit(udt_Connection*, PyObject*);
static PyObject *Connection_ChangePassword(udt_Connection*, PyObject*);
static PyObject *Connection_GetType(udt_Connection*, PyObject*);
static PyObject *Connection_GetStmtCacheSize(udt_Connection*, void*);
static PyObject *Connection_NewEnqueueOptions(udt_Connection*, PyObject*);
static PyObject *Connection_NewDequeueOptions(udt_Connection*, PyObject*);
static PyObject *Connection_NewMessageProperties(udt_Connection*, PyObject*);
static PyObject *Connection_Dequeue(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_Enqueue(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_Ping(udt_Connection*, PyObject*);
static PyObject *Connection_Shutdown(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_Startup(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_Subscribe(udt_Connection*, PyObject*, PyObject*);
static PyObject *Connection_GetLTXID(udt_Connection*, void*);
static PyObject *Connection_GetHandle(udt_Connection*, void*);
static PyObject *Connection_GetCurrentSchema(udt_Connection*, void*);
static PyObject *Connection_GetEdition(udt_Connection*, void*);
static PyObject *Connection_GetExternalName(udt_Connection*, void*);
static PyObject *Connection_GetInternalName(udt_Connection*, void*);
static int Connection_SetStmtCacheSize(udt_Connection*, PyObject*, void*);
static int Connection_SetAction(udt_Connection*, PyObject*, void*);
static int Connection_SetClientIdentifier(udt_Connection*, PyObject*, void*);
static int Connection_SetClientInfo(udt_Connection*, PyObject*, void*);
static int Connection_SetCurrentSchema(udt_Connection*, PyObject*, void*);
static int Connection_SetDbOp(udt_Connection*, PyObject*, void*);
static int Connection_SetExternalName(udt_Connection*, PyObject*, void*);
static int Connection_SetInternalName(udt_Connection*, PyObject*, void*);
static int Connection_SetModule(udt_Connection*, PyObject*, void*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "Connection"
//-----------------------------------------------------------------------------
static PyMethodDef g_ConnectionMethods[] = {
    { "cursor", (PyCFunction) Connection_NewCursor,
            METH_VARARGS | METH_KEYWORDS },
    { "commit", (PyCFunction) Connection_Commit, METH_NOARGS },
    { "rollback", (PyCFunction) Connection_Rollback, METH_NOARGS },
    { "begin", (PyCFunction) Connection_Begin, METH_VARARGS },
    { "prepare", (PyCFunction) Connection_Prepare, METH_NOARGS },
    { "close", (PyCFunction) Connection_Close, METH_NOARGS },
    { "cancel", (PyCFunction) Connection_Cancel, METH_NOARGS },
    { "__enter__", (PyCFunction) Connection_ContextManagerEnter, METH_NOARGS },
    { "__exit__", (PyCFunction) Connection_ContextManagerExit, METH_VARARGS },
    { "ping", (PyCFunction) Connection_Ping, METH_NOARGS },
    { "shutdown", (PyCFunction) Connection_Shutdown,
            METH_VARARGS | METH_KEYWORDS},
    { "startup", (PyCFunction) Connection_Startup,
            METH_VARARGS | METH_KEYWORDS},
    { "subscribe", (PyCFunction) Connection_Subscribe,
            METH_VARARGS | METH_KEYWORDS},
    { "changepassword", (PyCFunction) Connection_ChangePassword,
            METH_VARARGS },
    { "gettype", (PyCFunction) Connection_GetType, METH_VARARGS },
    { "deqoptions", (PyCFunction) Connection_NewDequeueOptions, METH_NOARGS },
    { "enqoptions", (PyCFunction) Connection_NewEnqueueOptions, METH_NOARGS },
    { "msgproperties", (PyCFunction) Connection_NewMessageProperties,
            METH_NOARGS },
    { "deq", (PyCFunction) Connection_Dequeue, METH_VARARGS | METH_KEYWORDS },
    { "enq", (PyCFunction) Connection_Enqueue, METH_VARARGS | METH_KEYWORDS },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "Connection"
//-----------------------------------------------------------------------------
static PyMemberDef g_ConnectionMembers[] = {
    { "username", T_OBJECT, offsetof(udt_Connection, username), READONLY },
    { "dsn", T_OBJECT, offsetof(udt_Connection, dsn), READONLY },
    { "tnsentry", T_OBJECT, offsetof(udt_Connection, dsn), READONLY },
    { "autocommit", T_INT, offsetof(udt_Connection, autocommit), 0 },
    { "inputtypehandler", T_OBJECT,
            offsetof(udt_Connection, inputTypeHandler), 0 },
    { "outputtypehandler", T_OBJECT,
            offsetof(udt_Connection, outputTypeHandler), 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "Connection"
//-----------------------------------------------------------------------------
static PyGetSetDef g_ConnectionCalcMembers[] = {
    { "version", (getter) Connection_GetVersion, 0, 0, 0 },
    { "encoding", (getter) Connection_GetEncoding, 0, 0, 0 },
    { "nencoding", (getter) Connection_GetNationalEncoding, 0, 0, 0 },
    { "maxBytesPerCharacter", (getter) Connection_GetMaxBytesPerCharacter,
            0, 0, 0 },
    { "stmtcachesize", (getter) Connection_GetStmtCacheSize,
            (setter) Connection_SetStmtCacheSize, 0, 0 },
    { "module", 0, (setter) Connection_SetModule, 0, 0 },
    { "action", 0, (setter) Connection_SetAction, 0, 0 },
    { "clientinfo", 0, (setter) Connection_SetClientInfo, 0, 0 },
    { "client_identifier", 0, (setter) Connection_SetClientIdentifier, 0, 0 },
    { "current_schema", (getter) Connection_GetCurrentSchema,
            (setter) Connection_SetCurrentSchema, 0, 0 },
    { "external_name", (getter) Connection_GetExternalName,
            (setter) Connection_SetExternalName, 0, 0 },
    { "internal_name", (getter) Connection_GetInternalName,
            (setter) Connection_SetInternalName, 0, 0 },
    { "dbop", 0, (setter) Connection_SetDbOp, 0, 0 },
    { "edition", (getter) Connection_GetEdition, 0, 0, 0 },
    { "ltxid", (getter) Connection_GetLTXID, 0, 0, 0 },
    { "handle", (getter) Connection_GetHandle, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type "Connection"
//-----------------------------------------------------------------------------
static PyTypeObject g_ConnectionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.Connection",             // tp_name
    sizeof(udt_Connection),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Connection_Free,       // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Connection_Repr,         // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                        // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    g_ConnectionMethods,                // tp_methods
    g_ConnectionMembers,                // tp_members
    g_ConnectionCalcMembers,            // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    (initproc) Connection_Init,         // tp_init
    0,                                  // tp_alloc
    (newfunc) Connection_New,           // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// structure used to help in establishing a connection
//-----------------------------------------------------------------------------
typedef struct {
    udt_Buffer userNameBuffer;
    udt_Buffer passwordBuffer;
    udt_Buffer newPasswordBuffer;
    udt_Buffer dsnBuffer;
    udt_Buffer connectionClassBuffer;
    udt_Buffer editionBuffer;
    udt_Buffer tagBuffer;
    uint32_t numAppContext;
    dpiAppContext *appContext;
    udt_Buffer *ctxNamespaceBuffers;
    udt_Buffer *ctxNameBuffers;
    udt_Buffer *ctxValueBuffers;
} udt_ConnectionParams;


//-----------------------------------------------------------------------------
// ConnectionParams_Initialize()
//   Initialize the parameters to default values.
//-----------------------------------------------------------------------------
static void ConnectionParams_Initialize(udt_ConnectionParams *params)
{
    cxBuffer_Init(&params->userNameBuffer);
    cxBuffer_Init(&params->passwordBuffer);
    cxBuffer_Init(&params->newPasswordBuffer);
    cxBuffer_Init(&params->dsnBuffer);
    cxBuffer_Init(&params->connectionClassBuffer);
    cxBuffer_Init(&params->editionBuffer);
    cxBuffer_Init(&params->tagBuffer);
    params->numAppContext = 0;
    params->appContext = NULL;
    params->ctxNamespaceBuffers = NULL;
    params->ctxNameBuffers = NULL;
    params->ctxValueBuffers = NULL;
}


//-----------------------------------------------------------------------------
// ConnectionParams_ProcessContext()
//   Process context for the connection parameters. This validates that the
// context passed in is a list of 3-tuples (namespace, name, value) and
// populates the parametrs with buffers for each of these.
//-----------------------------------------------------------------------------
static int ConnectionParams_ProcessContext(udt_ConnectionParams *params,
        PyObject *context, const char *encoding)
{
    uint32_t numEntries, i;
    dpiAppContext *entry;
    PyObject *entryObj;
    size_t memorySize;

    // validate context is a list with at least one entry in it
    if (!context)
        return 0;
    if (!PyList_Check(context)) {
        PyErr_SetString(PyExc_TypeError,
                "appcontext should be a list of 3-tuples");
        return -1;
    }
    numEntries = (uint32_t) PyList_GET_SIZE(context);
    if (numEntries == 0)
        return 0;

    // allocate memory for the buffers used to communicate with DPI
    params->appContext = PyMem_Malloc(numEntries * sizeof(dpiAppContext));
    memorySize = numEntries * sizeof(udt_Buffer);
    params->ctxNamespaceBuffers = PyMem_Malloc(memorySize);
    params->ctxNameBuffers = PyMem_Malloc(memorySize);
    params->ctxValueBuffers = PyMem_Malloc(memorySize);
    if (!params->appContext || !params->ctxNamespaceBuffers ||
            !params->ctxNameBuffers || !params->ctxValueBuffers) {
        PyErr_NoMemory();
        return -1;
    }

    // initialize buffers
    for (i = 0; i < numEntries; i++) {
        cxBuffer_Init(&params->ctxNamespaceBuffers[i]);
        cxBuffer_Init(&params->ctxNameBuffers[i]);
        cxBuffer_Init(&params->ctxValueBuffers[i]);
    }
    params->numAppContext = numEntries;

    // process each entry
    for (i = 0; i < numEntries; i++) {
        entryObj = PyList_GET_ITEM(context, i);
        if (!PyTuple_Check(entryObj) || PyTuple_GET_SIZE(entryObj) != 3) {
            PyErr_SetString(PyExc_TypeError,
                    "appcontext should be a list of 3-tuples");
            return -1;
        }
        if (cxBuffer_FromObject(&params->ctxNamespaceBuffers[i],
                PyTuple_GET_ITEM(entryObj, 0), encoding) < 0)
            return -1;
        if (cxBuffer_FromObject(&params->ctxNameBuffers[i],
                PyTuple_GET_ITEM(entryObj, 1), encoding) < 0)
            return -1;
        if (cxBuffer_FromObject(&params->ctxValueBuffers[i],
                PyTuple_GET_ITEM(entryObj, 2), encoding) < 0)
            return -1;
        entry = &params->appContext[i];
        entry->namespaceName = params->ctxNamespaceBuffers[i].ptr;
        entry->namespaceNameLength = params->ctxNamespaceBuffers[i].size;
        entry->name = params->ctxNameBuffers[i].ptr;
        entry->nameLength = params->ctxNameBuffers[i].size;
        entry->value = params->ctxValueBuffers[i].ptr;
        entry->valueLength = params->ctxValueBuffers[i].size;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// ConnectionParams_Finalize()
//   Finalize the parameters, freeing any resources that were allocated. The
// return value is a convenience to the caller.
//-----------------------------------------------------------------------------
static int ConnectionParams_Finalize(udt_ConnectionParams *params)
{
    uint32_t i;

    cxBuffer_Clear(&params->userNameBuffer);
    cxBuffer_Clear(&params->passwordBuffer);
    cxBuffer_Clear(&params->newPasswordBuffer);
    cxBuffer_Clear(&params->dsnBuffer);
    cxBuffer_Clear(&params->connectionClassBuffer);
    cxBuffer_Clear(&params->editionBuffer);
    cxBuffer_Clear(&params->tagBuffer);
    for (i = 0; i < params->numAppContext; i++) {
        cxBuffer_Clear(&params->ctxNamespaceBuffers[i]);
        cxBuffer_Clear(&params->ctxNameBuffers[i]);
        cxBuffer_Clear(&params->ctxValueBuffers[i]);
    }
    params->numAppContext = 0;
    if (params->appContext) {
        PyMem_Free(params->appContext);
        params->appContext = NULL;
    }
    if (params->ctxNamespaceBuffers) {
        PyMem_Free(params->ctxNamespaceBuffers);
        params->ctxNamespaceBuffers = NULL;
    }
    if (params->ctxNameBuffers) {
        PyMem_Free(params->ctxNameBuffers);
        params->ctxNameBuffers = NULL;
    }
    if (params->ctxValueBuffers) {
        PyMem_Free(params->ctxValueBuffers);
        params->ctxValueBuffers = NULL;
    }
    return -1;
}


//-----------------------------------------------------------------------------
// Connection_IsConnected()
//   Determines if the connection object is connected to the database. If not,
// a Python exception is raised.
//-----------------------------------------------------------------------------
static int Connection_IsConnected(udt_Connection *self)
{
    if (!self->handle) {
        PyErr_SetString(g_InterfaceErrorException, "not connected");
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Connection_GetAttrText()
//   Get the value of the attribute returned from the given function. The value
// is assumed to be a text value.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetAttrText(udt_Connection *self,
        int (*func)(dpiConn *conn, const char **value, uint32_t *valueLength))
{
    uint32_t valueLength;
    const char *value;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    if ((*func)(self->handle, &value, &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return cxString_FromEncodedString(value, valueLength,
            self->encodingInfo.encoding);
}


//-----------------------------------------------------------------------------
// Connection_SetAttrText()
//   Set the value of the attribute using the given function. The value is
// assumed to be a text value.
//-----------------------------------------------------------------------------
static int Connection_SetAttrText(udt_Connection *self, PyObject *value,
        int (*func)(dpiConn *conn, const char *value, uint32_t valueLength))
{
    udt_Buffer buffer;
    int status;

    if (Connection_IsConnected(self) < 0)
        return -1;
    if (cxBuffer_FromObject(&buffer, value, self->encodingInfo.encoding))
        return -1;
    status = (*func)(self->handle, buffer.ptr, buffer.size);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// Connection_ChangePassword()
//   Change the password for the given connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_ChangePassword(udt_Connection *self,
        PyObject *args)
{
    udt_Buffer usernameBuffer, oldPasswordBuffer, newPasswordBuffer;
    PyObject *oldPasswordObj, *newPasswordObj;
    int status;

    // parse the arguments
    if (!PyArg_ParseTuple(args, "OO", &oldPasswordObj, &newPasswordObj))
        return NULL;

    // populate buffers
    cxBuffer_Init(&usernameBuffer);
    cxBuffer_Init(&oldPasswordBuffer);
    cxBuffer_Init(&newPasswordBuffer);
    if (cxBuffer_FromObject(&usernameBuffer, self->username,
                    self->encodingInfo.encoding) < 0 ||
            cxBuffer_FromObject(&oldPasswordBuffer, oldPasswordObj,
                    self->encodingInfo.encoding) < 0 ||
            cxBuffer_FromObject(&newPasswordBuffer, newPasswordObj,
                    self->encodingInfo.encoding) < 0) {
        cxBuffer_Clear(&usernameBuffer);
        cxBuffer_Clear(&oldPasswordBuffer);
        cxBuffer_Clear(&newPasswordBuffer);
        return NULL;
    }

    // change the password
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_changePassword(self->handle, usernameBuffer.ptr,
            usernameBuffer.size, oldPasswordBuffer.ptr, oldPasswordBuffer.size,
            newPasswordBuffer.ptr, newPasswordBuffer.size);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&usernameBuffer);
    cxBuffer_Clear(&oldPasswordBuffer);
    cxBuffer_Clear(&newPasswordBuffer);
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


#include "LOB.c"
#include "Cursor.c"
#include "Subscription.c"
#include "DeqOptions.c"
#include "EnqOptions.c"
#include "MsgProps.c"


//-----------------------------------------------------------------------------
// Connection_New()
//   Create a new connection object and return it.
//-----------------------------------------------------------------------------
static PyObject* Connection_New(PyTypeObject *type, PyObject *args,
        PyObject *keywordArgs)
{
    udt_Connection *self;

    // create the object
    self = (udt_Connection*) type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Connection_SplitComponent()
//   Split the component out of the source and replace the source with the
// characters up to the split string and put the characters after the split
// string in to the target.
//-----------------------------------------------------------------------------
static int Connection_SplitComponent(PyObject **sourceObj,
        PyObject **targetObj, const char *splitString)
{
    PyObject *temp, *posObj;
    Py_ssize_t size, pos;

    if (!*sourceObj || *targetObj)
        return 0;
    posObj = PyObject_CallMethod(*sourceObj, "find", "s", splitString);
    if (!posObj)
        return -1;
    pos = PyInt_AsLong(posObj);
    Py_DECREF(posObj);
    if (PyErr_Occurred())
        return -1;
    if (pos >= 0) {
        size = PySequence_Size(*sourceObj);
        if (PyErr_Occurred())
            return -1;
        *targetObj = PySequence_GetSlice(*sourceObj, pos + 1, size);
        if (!*targetObj)
            return -1;
        temp = PySequence_GetSlice(*sourceObj, 0, pos);
        if (!temp)
            return -1;
        *sourceObj = temp;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Connection_Init()
//   Initialize the connection members.
//-----------------------------------------------------------------------------
static int Connection_Init(udt_Connection *self, PyObject *args,
        PyObject *keywordArgs)
{
    PyObject *tagObj, *matchAnyTagObj, *threadedObj, *eventsObj, *contextObj;
    PyObject *usernameObj, *passwordObj, *dsnObj, *cclassObj, *editionObj;
    dpiCommonCreateParams dpiCommonParams;
    dpiConnCreateParams dpiCreateParams;
    udt_ConnectionParams params;
    PyObject *newPasswordObj;
    udt_SessionPool *pool;
    const char *encoding;
    int status, temp;

    // define keyword arguments
    static char *keywordList[] = { "user", "password", "dsn", "mode",
            "handle", "pool", "threaded", "events", "cclass", "purity",
            "newpassword", "encoding", "nencoding", "edition", "appcontext",
            "tag", "matchanytag", NULL };

    // parse arguments
    pool = NULL;
    threadedObj = eventsObj = newPasswordObj = usernameObj = NULL;
    passwordObj = dsnObj = cclassObj = editionObj = tagObj = NULL;
    matchAnyTagObj = contextObj = NULL;
    if (dpiContext_initCommonCreateParams(g_DpiContext, &dpiCommonParams) < 0)
        return Error_RaiseAndReturnInt();
    dpiCommonParams.driverName = DRIVER_NAME;
    dpiCommonParams.driverNameLength =
            (uint32_t) strlen(dpiCommonParams.driverName);
    if (dpiContext_initConnCreateParams(g_DpiContext, &dpiCreateParams) < 0)
        return Error_RaiseAndReturnInt();
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs,
            "|OOOikO!OOOiOssOOOO", keywordList, &usernameObj, &passwordObj,
            &dsnObj, &dpiCreateParams.authMode,
            &dpiCreateParams.externalHandle, &g_SessionPoolType, &pool,
            &threadedObj, &eventsObj, &cclassObj, &dpiCreateParams.purity,
            &newPasswordObj, &dpiCommonParams.encoding,
            &dpiCommonParams.nencoding, &editionObj, &contextObj, &tagObj,
            &matchAnyTagObj))
        return -1;
    if (GetBooleanValue(threadedObj, 0, &temp) < 0)
        return -1;
    if (temp)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_THREADED;
    if (GetBooleanValue(eventsObj, 0, &temp) < 0)
        return -1;
    if (temp)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_EVENTS;
    if (GetBooleanValue(matchAnyTagObj, 0, &dpiCreateParams.matchAnyTag) < 0)
        return -1;

    // keep a copy of the user name and connect string (DSN)
    Py_XINCREF(usernameObj);
    self->username = usernameObj;
    Py_XINCREF(dsnObj);
    self->dsn = dsnObj;

    // perform some parsing, if necessary
    if (Connection_SplitComponent(&self->username, &passwordObj, "/") < 0)
        return -1;
    if (Connection_SplitComponent(&passwordObj, &self->dsn, "@") < 0)
        return -1;

    // setup parameters
    if (pool) {
        dpiCreateParams.pool = pool->handle;
        encoding = pool->encodingInfo.encoding;
    } else encoding = GetAdjustedEncoding(dpiCommonParams.encoding);
    ConnectionParams_Initialize(&params);
    if (ConnectionParams_ProcessContext(&params, contextObj, encoding) < 0)
        return ConnectionParams_Finalize(&params);
    if (cxBuffer_FromObject(&params.userNameBuffer, self->username,
                    encoding) < 0 ||
            cxBuffer_FromObject(&params.passwordBuffer, passwordObj,
                    encoding) < 0 ||
            cxBuffer_FromObject(&params.dsnBuffer, self->dsn, encoding) < 0 ||
            cxBuffer_FromObject(&params.connectionClassBuffer, cclassObj,
                    encoding) < 0 ||
            cxBuffer_FromObject(&params.newPasswordBuffer, newPasswordObj,
                    encoding) < 0 ||
            cxBuffer_FromObject(&params.editionBuffer, editionObj,
                    encoding) < 0 ||
            cxBuffer_FromObject(&params.tagBuffer, tagObj, encoding) < 0)
        return ConnectionParams_Finalize(&params);
    if (params.userNameBuffer.size == 0 && params.passwordBuffer.size == 0)
        dpiCreateParams.externalAuth = 1;
    dpiCreateParams.connectionClass = params.connectionClassBuffer.ptr;
    dpiCreateParams.connectionClassLength = params.connectionClassBuffer.size;
    dpiCreateParams.newPassword = params.newPasswordBuffer.ptr;
    dpiCreateParams.newPasswordLength = params.newPasswordBuffer.size;
    dpiCommonParams.edition = params.editionBuffer.ptr;
    dpiCommonParams.editionLength = params.editionBuffer.size;
    dpiCreateParams.tag = params.tagBuffer.ptr;
    dpiCreateParams.tagLength = params.tagBuffer.size;
    dpiCreateParams.appContext = params.appContext;
    dpiCreateParams.numAppContext = params.numAppContext;
    if (pool && !pool->homogeneous && pool->username && self->username) {
        temp = PyObject_RichCompareBool(self->username, pool->username, Py_EQ);
        if (temp < 0)
            return ConnectionParams_Finalize(&params);
        if (temp)
            params.userNameBuffer.size = 0;
    }

    // create connection
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_create(g_DpiContext, params.userNameBuffer.ptr,
            params.userNameBuffer.size, params.passwordBuffer.ptr,
            params.passwordBuffer.size, params.dsnBuffer.ptr,
            params.dsnBuffer.size, &dpiCommonParams, &dpiCreateParams,
            &self->handle);
    Py_END_ALLOW_THREADS
    ConnectionParams_Finalize(&params);
    if (status < 0)
        return Error_RaiseAndReturnInt();

    // determine encodings to use
    if (pool)
        self->encodingInfo = pool->encodingInfo;
    else {
        if (dpiConn_getEncodingInfo(self->handle, &self->encodingInfo) < 0)
            return Error_RaiseAndReturnInt();
        self->encodingInfo.encoding =
                GetAdjustedEncoding(self->encodingInfo.encoding);
        self->encodingInfo.nencoding =
                GetAdjustedEncoding(self->encodingInfo.nencoding);
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Connection_Free()
//   Deallocate the connection, disconnecting from the database if necessary.
//-----------------------------------------------------------------------------
static void Connection_Free(udt_Connection *self)
{
    if (self->handle) {
        Py_BEGIN_ALLOW_THREADS
        dpiConn_release(self->handle);
        Py_END_ALLOW_THREADS
        self->handle = NULL;
    }
    Py_CLEAR(self->sessionPool);
    Py_CLEAR(self->username);
    Py_CLEAR(self->dsn);
    Py_CLEAR(self->version);
    Py_CLEAR(self->inputTypeHandler);
    Py_CLEAR(self->outputTypeHandler);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Connection_Repr()
//   Return a string representation of the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_Repr(udt_Connection *connection)
{
    PyObject *module, *name, *result, *format, *formatArgs = NULL;

    if (GetModuleAndName(Py_TYPE(connection), &module, &name) < 0)
        return NULL;
    if (connection->username && connection->username != Py_None &&
            connection->dsn && connection->dsn != Py_None) {
        format = cxString_FromAscii("<%s.%s to %s@%s>");
        if (format)
            formatArgs = PyTuple_Pack(4, module, name, connection->username,
                    connection->dsn);
    } else if (connection->username && connection->username != Py_None) {
        format = cxString_FromAscii("<%s.%s to user %s@local>");
        if (format)
            formatArgs = PyTuple_Pack(3, module, name, connection->username);
    } else {
        format = cxString_FromAscii("<%s.%s to externally identified user>");
        if (format)
            formatArgs = PyTuple_Pack(2, module, name);
    }
    Py_DECREF(module);
    Py_DECREF(name);
    if (!format)
        return NULL;
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
// Connection_GetStmtCacheSize()
//   Return the Oracle statement cache size.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetStmtCacheSize(udt_Connection* self, void* arg)
{
    uint32_t cacheSize;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    if (dpiConn_getStmtCacheSize(self->handle, &cacheSize) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(cacheSize);
}


//-----------------------------------------------------------------------------
// Connection_SetStmtCacheSize()
//   Set the Oracle statement cache size.
//-----------------------------------------------------------------------------
static int Connection_SetStmtCacheSize(udt_Connection* self, PyObject *value,
        void* arg)
{
    uint32_t cacheSize;

    if (Connection_IsConnected(self) < 0)
        return -1;
    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "value must be an integer");
        return -1;
    }
    cacheSize = (uint32_t) PyInt_AsLong(value);
    if (dpiConn_setStmtCacheSize(self->handle, cacheSize) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// Connection_GetType()
//   Return a type object given its name.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetType(udt_Connection *self, PyObject *args)
{
    PyObject *nameObj = NULL;

    // parse the arguments
    if (!PyArg_ParseTuple(args, "O", &nameObj))
        return NULL;

    return (PyObject*) ObjectType_NewByName(self, nameObj);
}


//-----------------------------------------------------------------------------
// Connection_GetVersion()
//   Retrieve the version of the database and return it. Note that this
// function also places the result in the associated dictionary so it is only
// calculated once.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetVersion(udt_Connection *self, void *unused)
{
    uint32_t releaseStringLength;
    dpiVersionInfo versionInfo;
    const char *releaseString;
    char buffer[25];

    if (dpiConn_getServerVersion(self->handle, &releaseString,
            &releaseStringLength, &versionInfo) < 0)
        return Error_RaiseAndReturnNull();
    snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d.%d", versionInfo.versionNum,
            versionInfo.releaseNum, versionInfo.updateNum,
            versionInfo.portReleaseNum, versionInfo.portUpdateNum);
    return cxString_FromAscii(buffer);
}


//-----------------------------------------------------------------------------
// Connection_GetEncoding()
//   Return the encoding associated with the environment of the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetEncoding(udt_Connection *self, void *unused)
{
    return cxString_FromAscii(self->encodingInfo.encoding);
}


//-----------------------------------------------------------------------------
// Connection_GetLTXID()
//   Return the logical transaction id used with Transaction Guard.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetLTXID(udt_Connection *self, void *unused)
{
    uint32_t ltxidLength;
    const char *ltxid;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    if (dpiConn_getLTXID(self->handle, &ltxid, &ltxidLength) < 0)
        return Error_RaiseAndReturnNull();
    return PyBytes_FromStringAndSize(ltxid, ltxidLength);
}


//-----------------------------------------------------------------------------
// Connection_GetHandle()
//   Return the OCI handle used by the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetHandle(udt_Connection *self, void *unused)
{
    void *handle;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    if (dpiConn_getHandle(self->handle, &handle) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong((long) handle);
}


//-----------------------------------------------------------------------------
// Connection_GetNationalEncoding()
//   Return the national encoding associated with the environment of the
// connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetNationalEncoding(udt_Connection *self,
        void *unused)
{
    return cxString_FromAscii(self->encodingInfo.nencoding);
}


//-----------------------------------------------------------------------------
// Connection_GetMaxBytesPerCharacter()
//   Return the maximum number of bytes per character.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetMaxBytesPerCharacter(udt_Connection *self,
        void *unused)
{
    return PyInt_FromLong(self->encodingInfo.maxBytesPerCharacter);
}


//-----------------------------------------------------------------------------
// Connection_Close()
//   Close the connection, disconnecting from the database.
//-----------------------------------------------------------------------------
static PyObject *Connection_Close(udt_Connection *self, PyObject *args)
{
    int status;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_close(self->handle, DPI_MODE_CONN_CLOSE_DEFAULT, NULL, 0);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_Commit()
//   Commit the transaction on the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_Commit(udt_Connection *self, PyObject *args)
{
    int status;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_commit(self->handle);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_Begin()
//   Begin a new transaction on the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_Begin(udt_Connection *self, PyObject *args)
{
    uint32_t transactionIdLength, branchIdLength;
    const char *transactionId, *branchId;
    int formatId, status;

    // parse the arguments
    formatId = -1;
    transactionId = branchId = NULL;
    transactionIdLength = branchIdLength = 0;
    if (!PyArg_ParseTuple(args, "|is#s#", &formatId, &transactionId,
            &transactionIdLength,  &branchId, &branchIdLength))
        return NULL;

    // make sure we are actually connected
    if (Connection_IsConnected(self) < 0)
        return NULL;

    // begin the distributed transaction
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_beginDistribTrans(self->handle, formatId, transactionId,
            transactionIdLength, branchId, branchIdLength);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_Prepare()
//   Commit the transaction on the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_Prepare(udt_Connection *self, PyObject *args)
{
    int status, commitNeeded;

    // make sure we are actually connected
    if (Connection_IsConnected(self) < 0)
        return NULL;

    // perform the prepare
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_prepareDistribTrans(self->handle, &commitNeeded);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // return whether a commit is needed in order to allow for avoiding the
    // call to commit() which will fail with ORA-24756 (transaction does not
    // exist)
    return PyBool_FromLong(commitNeeded);
}


//-----------------------------------------------------------------------------
// Connection_Rollback()
//   Rollback the transaction on the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_Rollback(udt_Connection *self, PyObject *args)
{
    int status;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_rollback(self->handle);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_NewCursor()
//   Create a new cursor (statement) referencing the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_NewCursor(udt_Connection *self, PyObject *args,
        PyObject *keywordArgs)
{
    PyObject *createArgs, *result, *arg;
    Py_ssize_t numArgs = 0, i;

    if (args)
        numArgs = PyTuple_GET_SIZE(args);
    createArgs = PyTuple_New(1 + numArgs);
    if (!createArgs)
        return NULL;
    Py_INCREF(self);
    PyTuple_SET_ITEM(createArgs, 0, (PyObject*) self);
    for (i = 0; i < numArgs; i++) {
        arg = PyTuple_GET_ITEM(args, i);
        Py_INCREF(arg);
        PyTuple_SET_ITEM(createArgs, i + 1, arg);
    }
    result = PyObject_Call( (PyObject*) &g_CursorType, createArgs,
            keywordArgs);
    Py_DECREF(createArgs);
    return result;
}


//-----------------------------------------------------------------------------
// Connection_Cancel()
//   Cause Oracle to issue an immediate (asynchronous) abort of any currently
// executing statement.
//-----------------------------------------------------------------------------
static PyObject *Connection_Cancel(udt_Connection *self, PyObject *args)
{
    if (Connection_IsConnected(self) < 0)
        return NULL;
    if (dpiConn_breakExecution(self->handle) < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_NewEnqueueOptions()
//   Creates a new enqueue options object and returns it.
//-----------------------------------------------------------------------------
static PyObject *Connection_NewEnqueueOptions(udt_Connection *self,
        PyObject *args)
{
    return (PyObject*) EnqOptions_New(self);
}


//-----------------------------------------------------------------------------
// Connection_NewDequeueOptions()
//   Creates a new dequeue options object and returns it.
//-----------------------------------------------------------------------------
static PyObject *Connection_NewDequeueOptions(udt_Connection *self,
        PyObject *args)
{
    return (PyObject*) DeqOptions_New(self);
}


//-----------------------------------------------------------------------------
// Connection_NewMessageProperties()
//   Creates a new message properties object and returns it.
//-----------------------------------------------------------------------------
static PyObject *Connection_NewMessageProperties(udt_Connection *self,
        PyObject *args)
{
    return (PyObject*) MsgProps_New(self);
}


//-----------------------------------------------------------------------------
// Connection_Dequeue()
//   Dequeues a message using Advanced Queuing capabilities. The message ID is
// returned if a message is available or None if no message is available.
//-----------------------------------------------------------------------------
static PyObject *Connection_Dequeue(udt_Connection *self, PyObject* args,
        PyObject* keywordArgs)
{
    static char *keywordList[] = { "name", "options", "msgproperties",
            "payload", NULL };
    udt_MsgProps *propertiesObj;
    const char *messageIdValue;
    udt_DeqOptions *optionsObj;
    uint32_t messageIdLength;
    udt_Object *payloadObj;
    udt_Buffer nameBuffer;
    PyObject *nameObj;
    int status;

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO!O!O!", keywordList,
            &nameObj, &g_DeqOptionsType, &optionsObj, &g_MessagePropertiesType,
            &propertiesObj, &g_ObjectType, &payloadObj))
        return NULL;
    if (cxBuffer_FromObject(&nameBuffer, nameObj,
            self->encodingInfo.encoding) < 0)
        return NULL;

    // dequeue payload
    status = dpiConn_deqObject(self->handle, nameBuffer.ptr, nameBuffer.size,
            optionsObj->handle, propertiesObj->handle, payloadObj->handle,
            &messageIdValue, &messageIdLength);
    cxBuffer_Clear(&nameBuffer);
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // return message id
    if (!messageIdValue)
        Py_RETURN_NONE;
    return PyBytes_FromStringAndSize(messageIdValue, messageIdLength);
}


//-----------------------------------------------------------------------------
// Connection_Enqueue()
//   Enqueues a message using Advanced Queuing capabilities. The message ID is
// returned.
//-----------------------------------------------------------------------------
static PyObject *Connection_Enqueue(udt_Connection *self, PyObject* args,
        PyObject* keywordArgs)
{
    static char *keywordList[] = { "name", "options", "msgproperties",
            "payload", NULL };
    udt_MsgProps *propertiesObj;
    const char *messageIdValue;
    udt_EnqOptions *optionsObj;
    uint32_t messageIdLength;
    udt_Object *payloadObj;
    udt_Buffer nameBuffer;
    PyObject *nameObj;
    int status;

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO!O!O!", keywordList,
            &nameObj, &g_EnqOptionsType, &optionsObj, &g_MessagePropertiesType,
            &propertiesObj, &g_ObjectType, &payloadObj))
        return NULL;
    if (cxBuffer_FromObject(&nameBuffer, nameObj,
            self->encodingInfo.encoding) < 0)
        return NULL;

    // enqueue payload
    status = dpiConn_enqObject(self->handle, nameBuffer.ptr, nameBuffer.size,
            optionsObj->handle, propertiesObj->handle, payloadObj->handle,
            &messageIdValue, &messageIdLength);
    cxBuffer_Clear(&nameBuffer);
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // return message id
    return PyBytes_FromStringAndSize(messageIdValue, messageIdLength);
}


//-----------------------------------------------------------------------------
// Connection_ContextManagerEnter()
//   Called when the connection is used as a context manager and simply returns
// itself as a convenience to the caller.
//-----------------------------------------------------------------------------
static PyObject *Connection_ContextManagerEnter(udt_Connection *self,
        PyObject* args)
{
    Py_INCREF(self);
    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Connection_ContextManagerExit()
//   Called when the connection is used as a context manager and if any
// exception a rollback takes place; otherwise, a commit takes place.
//-----------------------------------------------------------------------------
static PyObject *Connection_ContextManagerExit(udt_Connection *self,
        PyObject* args)
{
    PyObject *excType, *excValue, *excTraceback, *result;
    char *methodName;

    if (!PyArg_ParseTuple(args, "OOO", &excType, &excValue, &excTraceback))
        return NULL;
    if (excType == Py_None && excValue == Py_None && excTraceback == Py_None)
        methodName = "commit";
    else methodName = "rollback";
    result = PyObject_CallMethod((PyObject*) self, methodName, "");
    if (!result)
        return NULL;
    Py_DECREF(result);

    Py_INCREF(Py_False);
    return Py_False;
}


//-----------------------------------------------------------------------------
// Connection_Ping()
//   Makes a round trip call to the server to confirm that the connection and
// server are active.
//-----------------------------------------------------------------------------
static PyObject *Connection_Ping(udt_Connection *self, PyObject* args)
{
    int status;

    if (Connection_IsConnected(self) < 0)
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_ping(self->handle);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_Shutdown()
//   Shuts down the database. Note that this must be done in two phases except
// in the situation where the instance is aborted.
//-----------------------------------------------------------------------------
static PyObject *Connection_Shutdown(udt_Connection *self, PyObject* args,
        PyObject* keywordArgs)
{
    static char *keywordList[] = { "mode", NULL };
    dpiShutdownMode mode;

    // parse arguments
    mode = DPI_MODE_SHUTDOWN_DEFAULT;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|i", keywordList,
            &mode))
        return NULL;

    // make sure we are actually connected
    if (Connection_IsConnected(self) < 0)
        return NULL;

    // perform the work
    if (dpiConn_shutdownDatabase(self->handle, mode) < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_Startup()
//   Starts up the database, equivalent to "startup nomount" in SQL*Plus.
//-----------------------------------------------------------------------------
static PyObject *Connection_Startup(udt_Connection *self, PyObject* args,
        PyObject* keywordArgs)
{
    static char *keywordList[] = { "force", "restrict", NULL };
    PyObject *forceObj, *restrictObj;
    dpiStartupMode mode;
    int temp;

    // parse arguments
    forceObj = restrictObj = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|OO", keywordList,
            &forceObj, &restrictObj))
        return NULL;

    // set the flags to use during startup
    mode = DPI_MODE_STARTUP_DEFAULT;
    if (GetBooleanValue(forceObj, 0, &temp) < 0)
        return NULL;
    if (temp)
        mode |= DPI_MODE_STARTUP_FORCE;
    if (GetBooleanValue(restrictObj, 0, &temp) < 0)
        return NULL;
    if (temp)
        mode |= DPI_MODE_STARTUP_RESTRICT;

    // make sure we are actually connected
    if (Connection_IsConnected(self) < 0)
        return NULL;

    // perform the work
    if (dpiConn_startupDatabase(self->handle, mode) < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Connection_Subscribe()
//   Create a subscription to events that take place in the database.
//-----------------------------------------------------------------------------
static PyObject *Connection_Subscribe(udt_Connection *self, PyObject* args,
        PyObject* keywordArgs)
{
    static char *keywordList[] = { "namespace", "protocol", "callback",
            "timeout", "operations", "port", "qos", NULL };
    uint32_t namespace, protocol, port, timeout, operations, qos;
    PyObject *callback;

    callback = NULL;
    timeout = port = qos = 0;
    namespace = DPI_SUBSCR_NAMESPACE_DBCHANGE;
    protocol = DPI_SUBSCR_PROTO_CALLBACK;
    operations = DPI_OPCODE_ALL_OPS;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|iiOiiii",
            keywordList, &namespace, &protocol, &callback, &timeout,
            &operations, &port, &qos))
        return NULL;
    return (PyObject*) Subscription_New(self, namespace, protocol, port,
            callback, timeout, operations, qos);
}


//-----------------------------------------------------------------------------
// Connection_GetCurrentSchema()
//   Return the current schema associated with the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetCurrentSchema(udt_Connection* self,
        void* unused)
{
    return Connection_GetAttrText(self, dpiConn_getCurrentSchema);
}


//-----------------------------------------------------------------------------
// Connection_GetEdition()
//   Return the edition associated with the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetEdition(udt_Connection* self, void* unused)
{
    return Connection_GetAttrText(self, dpiConn_getEdition);
}


//-----------------------------------------------------------------------------
// Connection_GetExternalName()
//   Return the external name associated with the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetExternalName(udt_Connection* self, void* unused)
{
    return Connection_GetAttrText(self, dpiConn_getExternalName);
}


//-----------------------------------------------------------------------------
// Connection_GetInternalName()
//   Return the internal name associated with the connection.
//-----------------------------------------------------------------------------
static PyObject *Connection_GetInternalName(udt_Connection* self, void* unused)
{
    return Connection_GetAttrText(self, dpiConn_getInternalName);
}


//-----------------------------------------------------------------------------
// Connection_SetAction()
//   Set the action associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetAction(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setAction);
}


//-----------------------------------------------------------------------------
// Connection_SetClientIdentifier()
//   Set the client identifier associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetClientIdentifier(udt_Connection* self,
        PyObject *value, void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setClientIdentifier);
}


//-----------------------------------------------------------------------------
// Connection_SetClientInfo()
//   Set the client info associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetClientInfo(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setClientInfo);
}


//-----------------------------------------------------------------------------
// Connection_SetCurrentSchema()
//   Set the current schema associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetCurrentSchema(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setCurrentSchema);
}


//-----------------------------------------------------------------------------
// Connection_SetDbOp()
//   Set the database operation associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetDbOp(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setDbOp);
}


//-----------------------------------------------------------------------------
// Connection_SetExternalName()
//   Set the external name associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetExternalName(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setExternalName);
}


//-----------------------------------------------------------------------------
// Connection_SetInternalName()
//   Set the internal name associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetInternalName(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setInternalName);
}


//-----------------------------------------------------------------------------
// Connection_SetModule()
//   Set the module associated with the connection.
//-----------------------------------------------------------------------------
static int Connection_SetModule(udt_Connection* self, PyObject *value,
        void* unused)
{
    return Connection_SetAttrText(self, value, dpiConn_setModule);
}

