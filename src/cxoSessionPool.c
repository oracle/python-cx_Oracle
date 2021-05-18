//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2021, Oracle and/or its affiliates. All rights reserved.
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

// forward declarations
int cxoSessionPool_reconfigureHelper(cxoSessionPool *pool,
        const char *attrName, PyObject *value);


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
    uint32_t waitTimeoutDeprecated, maxSessionsPerShardDeprecated;
    cxoBuffer userNameBuffer, passwordBuffer, dsnBuffer, editionBuffer;
    PyObject *usernameObj, *dsnObj, *sessionCallbackObj, *passwordObj;
    PyObject *editionObj, *sessionCallbackObjDeprecated;
    dpiCommonCreateParams dpiCommonParams;
    uint32_t maxLifetimeSessionDeprecated;
    dpiPoolCreateParams dpiCreateParams;
    cxoBuffer sessionCallbackBuffer;
    int status, threaded, events;
    PyTypeObject *connectionType;
    unsigned int stmtCacheSize;
    const char *encoding;

    // define keyword arguments
    static char *keywordList[] = { "user", "password", "dsn", "min", "max",
            "increment", "connectiontype", "threaded", "getmode", "events",
            "homogeneous", "externalauth", "encoding", "nencoding", "edition",
            "timeout", "wait_timeout", "max_lifetime_session",
            "session_callback", "max_sessions_per_shard",
            "soda_metadata_cache", "stmtcachesize", "ping_interval",
            "waitTimeout", "maxLifetimeSession", "sessionCallback",
            "maxSessionsPerShard",
            NULL };

    // parse arguments and keywords
    usernameObj = passwordObj = dsnObj = editionObj = Py_None;
    sessionCallbackObj = sessionCallbackObjDeprecated = passwordObj = NULL;
    connectionType = &cxoPyTypeConnection;
    minSessions = 1;
    maxSessions = 2;
    sessionIncrement = 1;
    maxSessionsPerShard = maxSessionsPerShardDeprecated = 0;
    waitTimeoutDeprecated = maxLifetimeSessionDeprecated = 0;
    maxSessionsPerShardDeprecated = 0;
    stmtCacheSize = DPI_DEFAULT_STMT_CACHE_SIZE;
    if (cxoUtils_initializeDPI(NULL) < 0)
        return -1;
    if (dpiContext_initCommonCreateParams(cxoDpiContext, &dpiCommonParams) < 0)
        return cxoError_raiseAndReturnInt();
    if (dpiContext_initPoolCreateParams(cxoDpiContext, &dpiCreateParams) < 0)
        return cxoError_raiseAndReturnInt();
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs,
            "|OOOiiiOpbpppssOiiiOipIiiiOi", keywordList, &usernameObj,
            &passwordObj, &dsnObj, &minSessions, &maxSessions,
            &sessionIncrement, &connectionType, &threaded,
            &dpiCreateParams.getMode, &events, &dpiCreateParams.homogeneous,
            &dpiCreateParams.externalAuth, &dpiCommonParams.encoding,
            &dpiCommonParams.nencoding, &editionObj, &dpiCreateParams.timeout,
            &dpiCreateParams.waitTimeout, &dpiCreateParams.maxLifetimeSession,
            &sessionCallbackObj, &maxSessionsPerShard,
            &dpiCommonParams.sodaMetadataCache, &stmtCacheSize,
            &dpiCreateParams.pingInterval, &waitTimeoutDeprecated,
            &maxLifetimeSessionDeprecated, &sessionCallbackObjDeprecated,
            &maxSessionsPerShardDeprecated))
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
    if (threaded)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_THREADED;
    if (events)
        dpiCommonParams.createMode |= DPI_MODE_CREATE_EVENTS;

    // check duplicate parameters to ensure that both are not specified
    if (waitTimeoutDeprecated > 0) {
        if (dpiCreateParams.waitTimeout > 0) {
            cxoError_raiseFromString(cxoProgrammingErrorException,
                    "waitTimeout and wait_timeout cannot both be specified");
            return -1;
        }
        dpiCreateParams.waitTimeout = waitTimeoutDeprecated;
    }
    if (maxLifetimeSessionDeprecated > 0) {
        if (dpiCreateParams.maxLifetimeSession > 0) {
            cxoError_raiseFromString(cxoProgrammingErrorException,
                    "maxLifetimeSession and max_lifetime_session cannot both "
                    "be specified");
            return -1;
        }
        dpiCreateParams.maxLifetimeSession = maxLifetimeSessionDeprecated;
    }
    if (sessionCallbackObjDeprecated) {
        if (sessionCallbackObj > 0) {
            cxoError_raiseFromString(cxoProgrammingErrorException,
                    "sessionCallback and session_callback cannot both "
                    "be specified");
            return -1;
        }
        sessionCallbackObj = sessionCallbackObjDeprecated;
    }
    if (maxSessionsPerShardDeprecated > 0) {
        if (maxSessionsPerShard > 0) {
            cxoError_raiseFromString(cxoProgrammingErrorException,
                    "maxSessionsPerShard and max_sessions_per_shard cannot "
                    "both be specified");
            return -1;
        }
        maxSessionsPerShard = maxSessionsPerShardDeprecated;
    }

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
    dpiCommonParams.stmtCacheSize = stmtCacheSize;

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
    uint32_t closeMode;
    int status, force;

    // parse arguments
    force = 0;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|p", keywordList,
            &force))
        return NULL;
    closeMode = (force) ? DPI_MODE_POOL_CLOSE_FORCE :
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
// cxoSessionPool_reconfigure()
//   Reconfigure properties of the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_reconfigure(cxoSessionPool *pool,
        PyObject *args, PyObject *keywordArgs)
{
    PyObject *timeout, *waitTimeout, *maxLifetimeSession, *maxSessionsPerShard;
    PyObject *sodaMetadataCache, *stmtcachesize, *pingInterval, *getMode;
    uint32_t minSessions, maxSessions, sessionIncrement;

    // define keyword arguments
    static char *keywordList[] = { "min", "max", "increment", "getmode",
            "timeout", "wait_timeout", "max_lifetime_session",
            "max_sessions_per_shard", "soda_metadata_cache", "stmtcachesize",
            "ping_interval", NULL };

    // set up default values
    minSessions = pool->minSessions;
    maxSessions = pool->maxSessions;
    sessionIncrement = pool->sessionIncrement;
    timeout = waitTimeout = maxLifetimeSession = maxSessionsPerShard = NULL;
    sodaMetadataCache = stmtcachesize = pingInterval = getMode = NULL;

    // parse arguments and keywords
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|iiiOOOOOOOO",
            keywordList, &minSessions, &maxSessions, &sessionIncrement,
            &getMode, &timeout, &waitTimeout, &maxLifetimeSession,
            &maxSessionsPerShard, &sodaMetadataCache, &stmtcachesize,
            &pingInterval))
        return NULL;

    // perform reconfiguration of the pool itself if needed
    if (minSessions != pool->minSessions || maxSessions != pool->maxSessions ||
            sessionIncrement != pool->sessionIncrement) {
        if (dpiPool_reconfigure(pool->handle, minSessions, maxSessions,
                sessionIncrement) < 0)
            return cxoError_raiseAndReturnNull();
        pool->minSessions = minSessions;
        pool->maxSessions = maxSessions;
        pool->sessionIncrement = sessionIncrement;
    }

    // adjust attributes
    if (cxoSessionPool_reconfigureHelper(pool, "getmode", getMode) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "timeout", timeout) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "wait_timeout",
            waitTimeout) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "max_lifetime_session",
            maxLifetimeSession) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "max_sessions_per_shard",
            maxSessionsPerShard) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "soda_metadata_cache",
            sodaMetadataCache) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "stmtcachesize",
            stmtcachesize) < 0)
        return NULL;
    if (cxoSessionPool_reconfigureHelper(pool, "ping_interval",
            pingInterval) < 0)
        return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_reconfigureHelpe()
//   Helper function that calls the setter for the session pool's property,
// after first checking that a value was supplied and not None.
//-----------------------------------------------------------------------------
int cxoSessionPool_reconfigureHelper(cxoSessionPool *pool,
        const char *attrName, PyObject *value)
{
    if (value != NULL && value != Py_None) {
        if (PyObject_SetAttrString((PyObject*) pool, attrName, value) < 0)
            return cxoError_raiseAndReturnInt();
    }
    return 0;
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
// cxoSessionPool_getMaxSessionsPerShard()
//   Return the maximum sessions per shard in the session pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getMaxSessionsPerShard(cxoSessionPool *pool,
        void *unused)
{
    return cxoSessionPool_getAttribute(pool, dpiPool_getMaxSessionsPerShard);
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
// cxoSessionPool_getPingInterval()
//   Return the current value of the ping interval set for the pool.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getPingInterval(cxoSessionPool *pool,
        void *unused)
{
    int value;

    if (dpiPool_getPingInterval(pool->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_getSodaMetadataCache()
//   Return a boolean indicating if the SODA metadata cache is enabled or not.
//-----------------------------------------------------------------------------
static PyObject *cxoSessionPool_getSodaMetadataCache(cxoSessionPool *pool,
        void *unused)
{
    int enabled;

    if (dpiPool_getSodaMetadataCache(pool->handle, &enabled) < 0)
        return cxoError_raiseAndReturnNull();
    return PyBool_FromLong(enabled);
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
// cxoSessionPool_setMaxSessionsPerShard()
//   Set the maximum lifetime for connections in the session pool.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setMaxSessionsPerShard(cxoSessionPool *pool,
        PyObject *value, void *unused)
{
    return cxoSessionPool_setAttribute(pool, value,
            dpiPool_setMaxSessionsPerShard);
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setPingInterval()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setPingInterval(cxoSessionPool *pool,
        PyObject *value, void *unused)
{
    long cValue;

    if (!PyLong_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "value must be an integer");
        return -1;
    }
    cValue = PyLong_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    if (dpiPool_setPingInterval(pool->handle, (int) cValue) < 0)
        return cxoError_raiseAndReturnInt();

    return 0;
}


//-----------------------------------------------------------------------------
// cxoSessionPool_setSodaMetadataCache()
//   Set whether the SODA metadata cache is enabled or not.
//-----------------------------------------------------------------------------
static int cxoSessionPool_setSodaMetadataCache(cxoSessionPool *pool,
        PyObject *value, void *unused)
{
    int cValue;

    if (value == Py_True) {
        cValue = 1;
    } else if (value == Py_False) {
        cValue = 0;
    } else {
        PyErr_SetString(PyExc_TypeError, "value must be a boolean");
        return -1;
    }
    if (dpiPool_setSodaMetadataCache(pool->handle, cValue) < 0)
        return cxoError_raiseAndReturnInt();

    return 0;
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
    { "reconfigure", (PyCFunction) cxoSessionPool_reconfigure,
            METH_VARARGS | METH_KEYWORDS },
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
    { "max_sessions_per_shard", (getter) cxoSessionPool_getMaxSessionsPerShard,
            (setter) cxoSessionPool_setMaxSessionsPerShard, 0, 0 },
    { "ping_interval", (getter) cxoSessionPool_getPingInterval,
            (setter) cxoSessionPool_setPingInterval, 0, 0 },
    { "soda_metadata_cache", (getter) cxoSessionPool_getSodaMetadataCache,
            (setter) cxoSessionPool_setSodaMetadataCache, 0, 0 },
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
