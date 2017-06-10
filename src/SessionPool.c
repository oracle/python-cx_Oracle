//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SessionPool.c
//   Handles session pooling.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type "SessionPool"
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiPool *handle;
    uint32_t minSessions;
    uint32_t maxSessions;
    uint32_t sessionIncrement;
    uint32_t cacheSize;
    dpiEncodingInfo encodingInfo;
    int homogeneous;
    int externalAuth;
    PyObject *username;
    PyObject *dsn;
    PyObject *name;
    PyTypeObject *connectionType;
} udt_SessionPool;

//-----------------------------------------------------------------------------
// functions for the Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyObject *SessionPool_New(PyTypeObject*, PyObject*, PyObject*);
static int SessionPool_Init(udt_SessionPool*, PyObject*, PyObject*);
static void SessionPool_Free(udt_SessionPool*);
static PyObject *SessionPool_Acquire(udt_SessionPool*, PyObject*, PyObject*);
static PyObject *SessionPool_Drop(udt_SessionPool*, PyObject*);
static PyObject *SessionPool_Release(udt_SessionPool*, PyObject*, PyObject*);
static PyObject *SessionPool_GetBusyCount(udt_SessionPool*, void*);
static PyObject *SessionPool_GetGetMode(udt_SessionPool*, void*);
static PyObject *SessionPool_GetMaxLifetimeSession(udt_SessionPool*, void*);
static PyObject *SessionPool_GetOpenCount(udt_SessionPool*, void*);
static PyObject *SessionPool_GetStmtCacheSize(udt_SessionPool*, void*);
static PyObject *SessionPool_GetTimeout(udt_SessionPool*, void*);
static int SessionPool_SetGetMode(udt_SessionPool*, PyObject*, void*);
static int SessionPool_SetMaxLifetimeSession(udt_SessionPool*, PyObject*,
        void*);
static int SessionPool_SetStmtCacheSize(udt_SessionPool*, PyObject*, void*);
static int SessionPool_SetTimeout(udt_SessionPool*, PyObject*, void*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyMethodDef g_SessionPoolMethods[] = {
    { "acquire", (PyCFunction) SessionPool_Acquire,
            METH_VARARGS | METH_KEYWORDS },
    { "drop", (PyCFunction) SessionPool_Drop, METH_VARARGS },
    { "release", (PyCFunction) SessionPool_Release,
            METH_VARARGS | METH_KEYWORDS },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyMemberDef g_SessionPoolMembers[] = {
    { "username", T_OBJECT, offsetof(udt_SessionPool, username), READONLY },
    { "dsn", T_OBJECT, offsetof(udt_SessionPool, dsn), READONLY },
    { "tnsentry", T_OBJECT, offsetof(udt_SessionPool, dsn), READONLY },
    { "name", T_OBJECT, offsetof(udt_SessionPool, name), READONLY },
    { "max", T_INT, offsetof(udt_SessionPool, maxSessions), READONLY },
    { "min", T_INT, offsetof(udt_SessionPool, minSessions), READONLY },
    { "increment", T_INT, offsetof(udt_SessionPool, sessionIncrement),
            READONLY },
    { "homogeneous", T_INT, offsetof(udt_SessionPool, homogeneous), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyGetSetDef g_SessionPoolCalcMembers[] = {
    { "opened", (getter) SessionPool_GetOpenCount, 0, 0, 0 },
    { "busy", (getter) SessionPool_GetBusyCount, 0, 0, 0 },
    { "timeout", (getter) SessionPool_GetTimeout,
            (setter) SessionPool_SetTimeout, 0, 0 },
    { "getmode", (getter) SessionPool_GetGetMode,
            (setter) SessionPool_SetGetMode, 0, 0 },
    { "max_lifetime_session", (getter) SessionPool_GetMaxLifetimeSession,
            (setter) SessionPool_SetMaxLifetimeSession, 0, 0 },
    { "stmtcachesize", (getter) SessionPool_GetStmtCacheSize,
            (setter) SessionPool_SetStmtCacheSize, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyTypeObject g_SessionPoolType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "OracleSessionPool",                // tp_name
    sizeof(udt_SessionPool),            // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) SessionPool_Free,      // tp_dealloc
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
                                        // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    g_SessionPoolMethods,               // tp_methods
    g_SessionPoolMembers,               // tp_members
    g_SessionPoolCalcMembers,           // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    (initproc) SessionPool_Init,        // tp_init
    0,                                  // tp_alloc
    (newfunc) SessionPool_New,          // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


#include "Connection.c"


//-----------------------------------------------------------------------------
// SessionPool_New()
//   Create a new session pool object.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_New(PyTypeObject *type, PyObject *args,
        PyObject *keywordArgs)
{
    udt_SessionPool *newObject;

    // create the object
    newObject = (udt_SessionPool*) type->tp_alloc(type, 0);
    if (!newObject)
        return NULL;

    return (PyObject*) newObject;
}


//-----------------------------------------------------------------------------
// SessionPool_Init()
//   Initialize the session pool object.
//-----------------------------------------------------------------------------
static int SessionPool_Init(udt_SessionPool *self, PyObject *args,
        PyObject *keywordArgs)
{
    udt_Buffer userNameBuffer, passwordBuffer, dsnBuffer, editionBuffer;
    PyObject *threadedObj, *eventsObj, *homogeneousObj, *passwordObj;
    uint32_t minSessions, maxSessions, sessionIncrement;
    PyObject *externalAuthObj, *editionObj;
    dpiCommonCreateParams dpiCommonParams;
    dpiPoolCreateParams dpiCreateParams;
    PyTypeObject *connectionType;
    const char *encoding;
    int status, temp;

    // define keyword arguments
    static char *keywordList[] = { "user", "password", "dsn", "min", "max",
            "increment", "connectiontype", "threaded", "getmode", "events",
            "homogeneous", "externalauth", "encoding", "nencoding", "edition",
            NULL };

    // parse arguments and keywords
    externalAuthObj = editionObj = NULL;
    threadedObj = eventsObj = homogeneousObj = passwordObj = NULL;
    connectionType = &g_ConnectionType;
    if (dpiContext_initCommonCreateParams(g_DpiContext, &dpiCommonParams) < 0)
        return Error_RaiseAndReturnInt();
    dpiCommonParams.driverName = DRIVER_NAME;
    dpiCommonParams.driverNameLength =
            (uint32_t) strlen(dpiCommonParams.driverName);
    if (dpiContext_initPoolCreateParams(g_DpiContext, &dpiCreateParams) < 0)
        return Error_RaiseAndReturnInt();
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OOOiii|OObOOOssO",
            keywordList, &self->username, &passwordObj, &self->dsn,
            &minSessions, &maxSessions, &sessionIncrement, &connectionType,
            &threadedObj, &dpiCreateParams.getMode, &eventsObj,
            &homogeneousObj, &externalAuthObj, &dpiCommonParams.encoding,
            &dpiCommonParams.nencoding, &editionObj))
        return -1;
    if (!PyType_Check(connectionType)) {
        PyErr_SetString(g_ProgrammingErrorException,
                "connectiontype must be a type");
        return -1;
    }
    if (!PyType_IsSubtype(connectionType, &g_ConnectionType)) {
        PyErr_SetString(g_ProgrammingErrorException,
                "connectiontype must be a subclass of Connection");
        return -1;
    }
    if (GetBooleanValue(threadedObj, 0, &temp) < 0)
        return -1;
    if (temp)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_THREADED;
    if (GetBooleanValue(eventsObj, 0, &temp) < 0)
        return -1;
    if (temp)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_EVENTS;
    if (GetBooleanValue(externalAuthObj, 0, &dpiCreateParams.externalAuth) < 0)
        return -1;
    if (GetBooleanValue(homogeneousObj, 1, &dpiCreateParams.homogeneous) < 0)
        return -1;

    // initialize the object's members
    Py_INCREF(connectionType);
    self->connectionType = connectionType;
    Py_INCREF(self->dsn);
    Py_INCREF(self->username);
    self->minSessions = minSessions;
    self->maxSessions = maxSessions;
    self->sessionIncrement = sessionIncrement;
    self->homogeneous = dpiCreateParams.homogeneous;
    self->externalAuth = dpiCreateParams.externalAuth;

    // populate parameters
    encoding = GetAdjustedEncoding(dpiCommonParams.encoding);
    if (cxBuffer_FromObject(&userNameBuffer, self->username, encoding) < 0)
        return -1;
    if (cxBuffer_FromObject(&passwordBuffer, passwordObj, encoding) < 0) {
        cxBuffer_Clear(&userNameBuffer);
        return -1;
    }
    if (cxBuffer_FromObject(&dsnBuffer, self->dsn, encoding) < 0) {
        cxBuffer_Clear(&userNameBuffer);
        cxBuffer_Clear(&passwordBuffer);
        return -1;
    }
    if (cxBuffer_FromObject(&editionBuffer, editionObj, encoding) < 0) {
        cxBuffer_Clear(&userNameBuffer);
        cxBuffer_Clear(&passwordBuffer);
        cxBuffer_Clear(&dsnBuffer);
        return -1;
    }
    dpiCreateParams.minSessions = minSessions;
    dpiCreateParams.maxSessions = maxSessions;
    dpiCreateParams.sessionIncrement = sessionIncrement;

    // create pool
    Py_BEGIN_ALLOW_THREADS
    status = dpiPool_create(g_DpiContext, userNameBuffer.ptr,
            userNameBuffer.size, passwordBuffer.ptr, passwordBuffer.size,
            dsnBuffer.ptr, dsnBuffer.size, &dpiCommonParams, &dpiCreateParams,
            &self->handle);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&userNameBuffer);
    cxBuffer_Clear(&passwordBuffer);
    cxBuffer_Clear(&dsnBuffer);
    cxBuffer_Clear(&editionBuffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();

    // get encodings and name
    if (dpiPool_getEncodingInfo(self->handle, &self->encodingInfo) < 0)
        return Error_RaiseAndReturnInt();
    self->encodingInfo.encoding =
            GetAdjustedEncoding(self->encodingInfo.encoding);
    self->encodingInfo.nencoding =
            GetAdjustedEncoding(self->encodingInfo.nencoding);
    self->name = cxString_FromEncodedString(dpiCreateParams.outPoolName,
            dpiCreateParams.outPoolNameLength, self->encodingInfo.encoding);
    if (!self->name)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// SessionPool_Free()
//   Deallocate the session pool.
//-----------------------------------------------------------------------------
static void SessionPool_Free(udt_SessionPool *self)
{
    if (self->handle) {
        dpiPool_release(self->handle);
        self->handle = NULL;
    }
    Py_CLEAR(self->username);
    Py_CLEAR(self->dsn);
    Py_CLEAR(self->name);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// SessionPool_Acquire()
//   Create a new connection within the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_Acquire(udt_SessionPool *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "user", "password", "cclass", "purity",
            "tag", "matchanytag", NULL };
    PyObject *createKeywordArgs, *result, *cclassObj, *purityObj, *tagObj;
    unsigned usernameLength, passwordLength;
    char *username, *password;
    PyObject *matchAnyTagObj;

    // parse arguments
    username = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|s#s#OOOO",
            keywordList, &username, &usernameLength, &password,
            &passwordLength, &cclassObj, &purityObj, &tagObj, &matchAnyTagObj))
        return NULL;
    if (self->homogeneous && username) {
        PyErr_SetString(g_ProgrammingErrorException,
                "pool is homogeneous. Proxy authentication is not possible.");
        return NULL;
    }

    // create arguments
    if (keywordArgs)
        createKeywordArgs = PyDict_Copy(keywordArgs);
    else createKeywordArgs = PyDict_New();
    if (!createKeywordArgs)
        return NULL;
    if (PyDict_SetItemString(createKeywordArgs, "pool",
            (PyObject*) self) < 0) {
        Py_DECREF(createKeywordArgs);
        return NULL;
    }

    // create the connection object
    result = PyObject_Call( (PyObject*) self->connectionType, args,
            createKeywordArgs);
    Py_DECREF(createKeywordArgs);

    return result;
}


//-----------------------------------------------------------------------------
// SessionPool_Drop()
//   Release a connection back to the session pool, dropping it so that a new
// connection will be created if needed.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_Drop(udt_SessionPool *self, PyObject *args)
{
    udt_Connection *connection;
    int status;

    // connection is expected
    if (!PyArg_ParseTuple(args, "O!", &g_ConnectionType, &connection))
        return NULL;

    // release the connection
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_close(connection->handle, DPI_MODE_CONN_CLOSE_DROP, NULL,
            0);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // mark connection as closed
    Py_CLEAR(connection->sessionPool);
    dpiConn_release(connection->handle);
    connection->handle = NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// SessionPool_Release()
//   Release a connection back to the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_Release(udt_SessionPool *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "connection", "tag", NULL };
    udt_Connection *connection;
    dpiConnCloseMode mode;
    udt_Buffer tagBuffer;
    PyObject *tagObj;
    int status;

    // parse arguments
    tagObj = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "O!|O",
            keywordList, &g_ConnectionType, &connection, &tagObj))
        return NULL;
    if (cxBuffer_FromObject(&tagBuffer, tagObj,
            self->encodingInfo.encoding) < 0)
        return NULL;
    mode = (tagBuffer.size > 0) ? DPI_MODE_CONN_CLOSE_RETAG :
            DPI_MODE_CONN_CLOSE_DEFAULT;
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_close(connection->handle, mode, (char*) tagBuffer.ptr,
            tagBuffer.size);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&tagBuffer);
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // mark connection as closed
    Py_CLEAR(connection->sessionPool);
    dpiConn_release(connection->handle);
    connection->handle = NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// SessionPool_GetAttribute()
//   Return the value for the attribute.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetAttribute(udt_SessionPool *self,
        int (*func)(dpiPool *pool, uint32_t *value))
{
    uint32_t value;

    if ((*func)(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
#if PY_MAJOR_VERSION >= 3
    return PyInt_FromUnsignedLong(value);
#else
    return PyInt_FromLong(value);
#endif
}


//-----------------------------------------------------------------------------
// SessionPool_SetAttribute()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int SessionPool_SetAttribute(udt_SessionPool *self, PyObject *value,
        int (*func)(dpiPool *pool, uint32_t value))
{
    uint32_t cValue;

    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "value must be an integer");
        return -1;
    }
#if PY_MAJOR_VERSION >= 3
    cValue = PyInt_AsUnsignedLong(value);
#else
    cValue = PyInt_AsLong(value);
#endif
    if (PyErr_Occurred())
        return -1;
    if ((*func)(self->handle, cValue) < 0)
        return Error_RaiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// SessionPool_GetBusyCount()
//   Return the number of busy connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetBusyCount(udt_SessionPool *pool, void *unused)
{
    return SessionPool_GetAttribute(pool, dpiPool_getBusyCount);
}


//-----------------------------------------------------------------------------
// SessionPool_GetGetMode()
//   Return the "get" mode for connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetGetMode(udt_SessionPool *pool, void *unused)
{
    dpiPoolGetMode value;

    if (dpiPool_getGetMode(pool->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// SessionPool_GetMaxLifetimeSession()
//   Return the maximum lifetime session of connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetMaxLifetimeSession(udt_SessionPool *pool,
        void *unused)
{
    return SessionPool_GetAttribute(pool, dpiPool_getMaxLifetimeSession);
}


//-----------------------------------------------------------------------------
// SessionPool_GetOpenCount()
//   Return the number of open connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetOpenCount(udt_SessionPool *pool, void *unused)
{
    return SessionPool_GetAttribute(pool, dpiPool_getOpenCount);
}


//-----------------------------------------------------------------------------
// SessionPool_GetStmtCacheSize()
//   Return the size of the statement cache to use in connections that are
// acquired from the pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetStmtCacheSize(udt_SessionPool *pool,
        void *unused)
{
    return SessionPool_GetAttribute(pool, dpiPool_getStmtCacheSize);
}


//-----------------------------------------------------------------------------
// SessionPool_GetTimeout()
//   Return the timeout for connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetTimeout(udt_SessionPool *pool, void *unused)
{
    return SessionPool_GetAttribute(pool, dpiPool_getTimeout);
}


//-----------------------------------------------------------------------------
// SessionPool_SetGetMode()
//   Set the "get" mode for connections in the session pool.
//-----------------------------------------------------------------------------
static int SessionPool_SetGetMode(udt_SessionPool *pool, PyObject *value,
        void *unused)
{
    dpiPoolGetMode cValue;

    cValue = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    if (dpiPool_setGetMode(pool->handle, cValue) < 0)
        return Error_RaiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// SessionPool_SetMaxLifetimeSession()
//   Set the maximum lifetime for connections in the session pool.
//-----------------------------------------------------------------------------
static int SessionPool_SetMaxLifetimeSession(udt_SessionPool *pool,
        PyObject *value, void *unused)
{
    return SessionPool_SetAttribute(pool, value,
            dpiPool_setMaxLifetimeSession);
}


//-----------------------------------------------------------------------------
// SessionPool_SetStmtCacheSize()
//   Set the default size of the statement cache used for connections that are
// acquired from the pool.
//-----------------------------------------------------------------------------
static int SessionPool_SetStmtCacheSize(udt_SessionPool *pool,
        PyObject *value, void *unused)
{
    return SessionPool_SetAttribute(pool, value, dpiPool_setStmtCacheSize);
}


//-----------------------------------------------------------------------------
// SessionPool_SetTimeout()
//   Set the timeout for connections in the session pool.
//-----------------------------------------------------------------------------
static int SessionPool_SetTimeout(udt_SessionPool *pool, PyObject *value,
        void *unused)
{
    return SessionPool_SetAttribute(pool, value, dpiPool_setTimeout);
}

