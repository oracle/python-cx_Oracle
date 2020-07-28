//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoSessionPool.c
//   Handles session pooling.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoSessionPool_new()
//   Create a new session pool object.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_new(PyTypeObject *type, PyObject *args,
        PyObject *keywordArgs)
{
    return type->tp_alloc(type, 0);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_init()
//   Initialize the session pool object.
//-----------------------------------------------------------------------------
static int cxoSessionPool_init(cxoSessionPool *pool, PyObject *args,
        PyObject *keywordArgs)
{
    uint32_t minSessions, maxSessions, sessionIncrement, maxSessionsPerShard;
    cxoBuffer userNameBuffer, passwordBuffer, dsnBuffer, editionBuffer;
    PyObject *threadedObj, *eventsObj, *homogeneousObj, *passwordObj;
    PyObject *usernameObj, *dsnObj, *sessionCallbackObj;
    PyObject *externalAuthObj, *editionObj;
    dpiCommonCreateParams dpiCommonParams;
    dpiPoolCreateParams dpiCreateParams;
    cxoBuffer sessionCallbackBuffer;
    PyTypeObject *connectionType;
    const char *encoding;
    int status, temp;

    // define keyword arguments
    static char *keywordList[] = { "user", "password", "dsn", "min", "max",
            "increment", "connectiontype", "threaded", "getmode", "events",
            "homogeneous", "externalauth", "encoding", "nencoding", "edition",
            "timeout", "waitTimeout", "maxLifetimeSession", "sessionCallback",
            "maxSessionsPerShard", NULL };

    // parse arguments and keywords
    usernameObj = passwordObj = dsnObj = editionObj = Py_None;
    externalAuthObj = sessionCallbackObj = NULL;
    threadedObj = eventsObj = homogeneousObj = passwordObj = NULL;
    connectionType = &cxoPyTypeConnection;
    minSessions = 1;
    maxSessions = 2;
    sessionIncrement = 1;
    maxSessionsPerShard = 0;
    if (cxoUtils_initializeDPI(NULL) < 0)
        return -1;
    if (dpiContext_initCommonCreateParams(cxoDpiContext, &dpiCommonParams) < 0)
        return cxoError_raiseAndReturnInt();
    if (dpiContext_initPoolCreateParams(cxoDpiContext, &dpiCreateParams) < 0)
        return cxoError_raiseAndReturnInt();
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs,
            "|OOOiiiOObOOOssOiiiOi", keywordList, &usernameObj, &passwordObj,
            &dsnObj, &minSessions, &maxSessions, &sessionIncrement,
            &connectionType, &threadedObj, &dpiCreateParams.getMode,
            &eventsObj, &homogeneousObj, &externalAuthObj,
            &dpiCommonParams.encoding, &dpiCommonParams.nencoding, &editionObj,
            &dpiCreateParams.timeout, &dpiCreateParams.waitTimeout,
            &dpiCreateParams.maxLifetimeSession, &sessionCallbackObj,
            &maxSessionsPerShard))
        return -1;
    if (!PyType_Check(connectionType)) {
        cxoError_raiseFromString(cxoProgrammingErrorException,
                "connectiontype must be a type");
        return -1;
    }
    if (!PyType_IsSubtype(connectionType, &cxoPyTypeConnection)) {
        cxoError_raiseFromString(cxoProgrammingErrorException,
                "connectiontype must be a subclass of Connection");
        return -1;
    }
    if (cxoUtils_getBooleanValue(threadedObj, 0, &temp) < 0)
        return -1;
    if (temp)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_THREADED;
    if (cxoUtils_getBooleanValue(eventsObj, 0, &temp) < 0)
        return -1;
    if (temp)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_EVENTS;
    if (cxoUtils_getBooleanValue(externalAuthObj, 0,
            &dpiCreateParams.externalAuth) < 0)
        return -1;
    if (cxoUtils_getBooleanValue(homogeneousObj, 1,
            &dpiCreateParams.homogeneous) < 0)
        return -1;

    // initialize the object's members
    Py_INCREF(connectionType);
    pool->connectionType = connectionType;
    Py_INCREF(dsnObj);
    pool->dsn = dsnObj;
    Py_INCREF(usernameObj);
    pool->username = usernameObj;
    pool->minSessions = minSessions;
    pool->maxSessions = maxSessions;
    pool->sessionIncrement = sessionIncrement;
    pool->homogeneous = dpiCreateParams.homogeneous;
    pool->externalAuth = dpiCreateParams.externalAuth;
    Py_XINCREF(sessionCallbackObj);
    pool->sessionCallback = sessionCallbackObj;

    // populate parameters
    encoding = cxoUtils_getAdjustedEncoding(dpiCommonParams.encoding);
    cxoBuffer_init(&userNameBuffer);
    cxoBuffer_init(&passwordBuffer);
    cxoBuffer_init(&dsnBuffer);
    cxoBuffer_init(&editionBuffer);
    cxoBuffer_init(&sessionCallbackBuffer);
    if (sessionCallbackObj && !PyCallable_Check(sessionCallbackObj) &&
            cxoBuffer_fromObject(&sessionCallbackBuffer, sessionCallbackObj,
                    encoding) < 0)
        return -1;
    if (cxoBuffer_fromObject(&userNameBuffer, usernameObj, encoding) < 0 ||
            cxoBuffer_fromObject(&passwordBuffer, passwordObj, encoding) < 0 ||
            cxoBuffer_fromObject(&dsnBuffer, dsnObj, encoding) < 0 ||
            cxoBuffer_fromObject(&editionBuffer, editionObj, encoding) < 0) {
        cxoBuffer_clear(&userNameBuffer);
        cxoBuffer_clear(&passwordBuffer);
        cxoBuffer_clear(&dsnBuffer);
        cxoBuffer_clear(&sessionCallbackBuffer);
        return -1;
    }
    dpiCreateParams.minSessions = minSessions;
    dpiCreateParams.maxSessions = maxSessions;
    dpiCreateParams.sessionIncrement = sessionIncrement;
    dpiCreateParams.plsqlFixupCallback = sessionCallbackBuffer.ptr;
    dpiCreateParams.plsqlFixupCallbackLength = sessionCallbackBuffer.size;
    dpiCreateParams.maxSessionsPerShard = maxSessionsPerShard;
    dpiCommonParams.edition = editionBuffer.ptr;
    dpiCommonParams.editionLength = editionBuffer.size;

    // create pool
    Py_BEGIN_ALLOW_THREADS
    status = dpiPool_create(cxoDpiContext, userNameBuffer.ptr,
            userNameBuffer.size, passwordBuffer.ptr, passwordBuffer.size,
            dsnBuffer.ptr, dsnBuffer.size, &dpiCommonParams, &dpiCreateParams,
            &pool->handle);
    Py_END_ALLOW_THREADS
    cxoBuffer_clear(&userNameBuffer);
    cxoBuffer_clear(&passwordBuffer);
    cxoBuffer_clear(&dsnBuffer);
    cxoBuffer_clear(&editionBuffer);
    if (status < 0)
        return cxoError_raiseAndReturnInt();

    // get encodings and name
    if (dpiPool_getEncodingInfo(pool->handle, &pool->encodingInfo) < 0)
        return cxoError_raiseAndReturnInt();
    pool->encodingInfo.encoding =
            cxoUtils_getAdjustedEncoding(pool->encodingInfo.encoding);
    pool->encodingInfo.nencoding =
            cxoUtils_getAdjustedEncoding(pool->encodingInfo.nencoding);
    pool->name = PyUnicode_Decode(dpiCreateParams.outPoolName,
            dpiCreateParams.outPoolNameLength, pool->encodingInfo.encoding,
            NULL);
    if (!pool->name)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_free()
//   Deallocate the session pool.
//-----------------------------------------------------------------------------
static void cxoSessionPool_free(cxoSessionPool *pool)
{
    if (pool->handle) {
        dpiPool_release(pool->handle);
        pool->handle = NULL;
    }
    Py_CLEAR(pool->username);
    Py_CLEAR(pool->dsn);
    Py_CLEAR(pool->name);
    Py_CLEAR(pool->sessionCallback);
    Py_TYPE(pool)->tp_free((PyObject*) pool);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_acquire()
//   Create a new connection within the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_acquire(cxoSessionPool *pool, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "user", "password", "cclass", "purity",
            "tag", "matchanytag", "shardingkey", "supershardingkey", NULL };
    PyObject *createKeywordArgs, *result, *cclassObj, *purityObj, *tagObj;
    PyObject *shardingKeyObj, *superShardingKeyObj;
    Py_ssize_t usernameLength, passwordLength;
    char *username, *password;
    PyObject *matchAnyTagObj;

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|s#s#OOOOOO",
            keywordList, &username, &usernameLength, &password,
            &passwordLength, &cclassObj, &purityObj, &tagObj, &matchAnyTagObj,
            &shardingKeyObj, &superShardingKeyObj))
        return NULL;

    // create arguments
    if (keywordArgs)
        createKeywordArgs = PyDict_Copy(keywordArgs);
    else createKeywordArgs = PyDict_New();
    if (!createKeywordArgs)
        return NULL;
    if (PyDict_SetItemString(createKeywordArgs, "pool",
            (PyObject*) pool) < 0) {
        Py_DECREF(createKeywordArgs);
        return NULL;
    }

    // create the connection object
    result = PyObject_Call( (PyObject*) pool->connectionType, args,
            createKeywordArgs);
    Py_DECREF(createKeywordArgs);

    return result;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_close()
//   Close the session pool and make it unusable.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_close(cxoSessionPool *pool, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "force", NULL };
    PyObject *forceObj;
    uint32_t closeMode;
    int temp, status;

    // parse arguments
    forceObj = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|O", keywordList,
            &forceObj))
        return NULL;
    if (cxoUtils_getBooleanValue(forceObj, 0, &temp) < 0)
        return NULL;
    closeMode = (temp) ? DPI_MODE_POOL_CLOSE_FORCE :
            DPI_MODE_POOL_CLOSE_DEFAULT;

    // close pool
    Py_BEGIN_ALLOW_THREADS
    status = dpiPool_close(pool->handle, closeMode);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return cxoError_raiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_drop()
//   Release a connection back to the session pool, dropping it so that a new
// connection will be created if needed.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_drop(cxoSessionPool *pool, PyObject *args)
{
    cxoConnection *connection;
    int status;

    // connection is expected
    if (!PyArg_ParseTuple(args, "O!", &cxoPyTypeConnection, &connection))
        return NULL;

    // release the connection
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_close(connection->handle, DPI_MODE_CONN_CLOSE_DROP, NULL,
            0);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return cxoError_raiseAndReturnNull();

    // mark connection as closed
    Py_CLEAR(connection->sessionPool);
    dpiConn_release(connection->handle);
    connection->handle = NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_release()
//   Release a connection back to the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_release(cxoSessionPool *pool, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "connection", "tag", NULL };
    cxoConnection *conn;
    cxoBuffer tagBuffer;
    PyObject *tagObj;
    uint32_t mode;
    int status;

    // parse arguments
    tagObj = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "O!|O",
            keywordList, &cxoPyTypeConnection, &conn, &tagObj))
        return NULL;
    if (!tagObj)
        tagObj = conn->tag;
    if (cxoBuffer_fromObject(&tagBuffer, tagObj,
            pool->encodingInfo.encoding) < 0)
        return NULL;
    mode = DPI_MODE_CONN_CLOSE_DEFAULT;
    if (tagObj && tagObj != Py_None)
        mode |= DPI_MODE_CONN_CLOSE_RETAG;
    Py_BEGIN_ALLOW_THREADS
    status = dpiConn_close(conn->handle, mode, (char*) tagBuffer.ptr,
            tagBuffer.size);
    Py_END_ALLOW_THREADS
    cxoBuffer_clear(&tagBuffer);
    if (status < 0)
        return cxoError_raiseAndReturnNull();

    // mark connection as closed
    Py_CLEAR(conn->sessionPool);
    dpiConn_release(conn->handle);
    conn->handle = NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getAttribute()
//   Return the value for the attribute.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getAttribute(cxoSessionPool *pool,
        int (*func)(dpiPool *pool, uint32_t *value))
{
    uint32_t value;

    if ((*func)(pool->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromUnsignedLong(value);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setAttribute()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setAttribute(cxoSessionPool *pool, PyObject *value,
        int (*func)(dpiPool *pool, uint32_t value))
{
    uint32_t cValue;

    if (!PyLong_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "value must be an integer");
        return -1;
    }
    cValue = PyLong_AsUnsignedLong(value);
    if (PyErr_Occurred())
        return -1;
    if ((*func)(pool->handle, cValue) < 0)
        return cxoError_raiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getBusyCount()
//   Return the number of busy connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getBusyCount(cxoSessionPool *pool,
        void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getBusyCount);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getGetMode()
//   Return the "get" mode for connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getGetMode(cxoSessionPool *pool, void *unused)
{
    dpiPoolGetMode value;

    if (dpiPool_getGetMode(pool->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getMaxLifetimeSession()
//   Return the maximum lifetime session of connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getMaxLifetimeSession(cxoSessionPool *pool,
        void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getMaxLifetimeSession);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getOpenCount()
//   Return the number of open connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getOpenCount(cxoSessionPool *pool, void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getOpenCount);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getStmtCacheSize()
//   Return the size of the statement cache to use in connections that are
// acquired from the pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getStmtCacheSize(cxoSessionPool *pool,
        void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getStmtCacheSize);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getTimeout()
//   Return the timeout for connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getTimeout(cxoSessionPool *pool, void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getTimeout);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getWaitTimeout()
//   Return the wait timeout for connections in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getWaitTimeout(cxoSessionPool *pool,
        void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getWaitTimeout);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setGetMode()
//   Set the "get" mode for connections in the session pool.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setGetMode(cxoSessionPool *pool, PyObject *value,
        void *unused)
{
    dpiPoolGetMode cValue;

    cValue = PyLong_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    if (dpiPool_setGetMode(pool->handle, cValue) < 0)
        return cxoError_raiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setMaxLifetimeSession()
//   Set the maximum lifetime for connections in the session pool.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setMaxLifetimeSession(cxoSessionPool *pool,
        PyObject *value, void *unused)
{
    return cxoSessionPool_setAttribute(pool, value,
            dpiPool_setMaxLifetimeSession);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setStmtCacheSize()
//   Set the default size of the statement cache used for connections that are
// acquired from the pool.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setStmtCacheSize(cxoSessionPool *pool,
        PyObject *value, void *unused)
{
    return cxoSessionPool_setAttribute(pool, value, dpiPool_setStmtCacheSize);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setTimeout()
//   Set the timeout for connections in the session pool.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setTimeout(cxoSessionPool *pool, PyObject *value,
        void *unused)
{
    return cxoSessionPool_setAttribute(pool, value, dpiPool_setTimeout);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setWaitTimeout()
//   Set the wait timeout for connections in the session pool.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setWaitTimeout(cxoSessionPool *pool, PyObject *value,
        void *unused)
{
    return cxoSessionPool_setAttribute(pool, value, dpiPool_setWaitTimeout);
}


//-----------------------------------------------------------------------------
// declaration of methods for Python type
//-----------------------------------------------------------------------------
static PyMethodDef cxoMethods[] = {
    { "acquire", (PyCFunction) cxoSessionPool_acquire,
            METH_VARARGS | METH_KEYWORDS },
    { "close", (PyCFunction) cxoSessionPool_close,
            METH_VARARGS | METH_KEYWORDS },
    { "drop", (PyCFunction) cxoSessionPool_drop, METH_VARARGS },
    { "release", (PyCFunction) cxoSessionPool_release,
            METH_VARARGS | METH_KEYWORDS },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type
//-----------------------------------------------------------------------------
static PyMemberDef cxoMembers[] = {
    { "username", T_OBJECT, offsetof(cxoSessionPool, username), READONLY },
    { "dsn", T_OBJECT, offsetof(cxoSessionPool, dsn), READONLY },
    { "tnsentry", T_OBJECT, offsetof(cxoSessionPool, dsn), READONLY },
    { "name", T_OBJECT, offsetof(cxoSessionPool, name), READONLY },
    { "max", T_INT, offsetof(cxoSessionPool, maxSessions), READONLY },
    { "min", T_INT, offsetof(cxoSessionPool, minSessions), READONLY },
    { "increment", T_INT, offsetof(cxoSessionPool, sessionIncrement),
            READONLY },
    { "homogeneous", T_INT, offsetof(cxoSessionPool, homogeneous), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type
//-----------------------------------------------------------------------------
static PyGetSetDef cxoCalcMembers[] = {
    { "opened", (getter) cxoSessionPool_getOpenCount, 0, 0, 0 },
    { "busy", (getter) cxoSessionPool_getBusyCount, 0, 0, 0 },
    { "timeout", (getter) cxoSessionPool_getTimeout,
            (setter) cxoSessionPool_setTimeout, 0, 0 },
    { "getmode", (getter) cxoSessionPool_getGetMode,
            (setter) cxoSessionPool_setGetMode, 0, 0 },
    { "max_lifetime_session", (getter) cxoSessionPool_getMaxLifetimeSession,
            (setter) cxoSessionPool_setMaxLifetimeSession, 0, 0 },
    { "stmtcachesize", (getter) cxoSessionPool_getStmtCacheSize,
            (setter) cxoSessionPool_setStmtCacheSize, 0, 0 },
    { "wait_timeout", (getter) cxoSessionPool_getWaitTimeout,
            (setter) cxoSessionPool_setWaitTimeout, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeSessionPool = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.SessionPool",
    .tp_basicsize = sizeof(cxoSessionPool),
    .tp_dealloc = (destructor) cxoSessionPool_free,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = cxoMethods,
    .tp_members = cxoMembers,
    .tp_getset = cxoCalcMembers,
    .tp_init = (initproc) cxoSessionPool_init,
    .tp_new = (newfunc) cxoSessionPool_new
};
