//-----------------------------------------------------------------------------
// SessionPool.c
//   Handles session pooling (only available in Oracle 9i and up).
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type "SessionPool"
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    OCISPool *handle;
    ub4 minSessions;
    ub4 maxSessions;
    ub4 sessionIncrement;
    ub4 cacheSize;
    PyObject *name;
    PyObject *username;
    PyObject *password;
    PyObject *dsn;
    udt_Environment *environment;
    PyTypeObject *connectionType;
} udt_SessionPool;

//-----------------------------------------------------------------------------
// constants for the OCI attributes
//-----------------------------------------------------------------------------
static ub4 gc_OpenAttribute = OCI_ATTR_SPOOL_OPEN_COUNT;
static ub4 gc_BusyAttribute = OCI_ATTR_SPOOL_BUSY_COUNT;
static ub4 gc_TimeoutAttribute = OCI_ATTR_SPOOL_TIMEOUT;
static ub4 gc_GetModeAttribute = OCI_ATTR_SPOOL_GETMODE;

//-----------------------------------------------------------------------------
// functions for the Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyObject *SessionPool_New(PyTypeObject*, PyObject*, PyObject*);
static int SessionPool_Init(udt_SessionPool*, PyObject*, PyObject*);
static void SessionPool_Free(udt_SessionPool*);
static PyObject *SessionPool_Acquire(udt_SessionPool*, PyObject*);
static PyObject *SessionPool_Drop(udt_SessionPool*, PyObject*);
static PyObject *SessionPool_Release(udt_SessionPool*, PyObject*);
static PyObject *SessionPool_GetOCIAttr(udt_SessionPool*, ub4*);
static int SessionPool_SetOCIAttr(udt_SessionPool*, PyObject*, ub4*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyMethodDef g_SessionPoolMethods[] = {
    { "acquire", (PyCFunction) SessionPool_Acquire, METH_NOARGS },
    { "drop", (PyCFunction) SessionPool_Drop, METH_VARARGS },
    { "release", (PyCFunction) SessionPool_Release, METH_VARARGS },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of members for Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyMemberDef g_SessionPoolMembers[] = {
    { "username", T_OBJECT, offsetof(udt_SessionPool, username), READONLY },
    { "password", T_OBJECT, offsetof(udt_SessionPool, password), READONLY },
    { "dsn", T_OBJECT, offsetof(udt_SessionPool, dsn), READONLY },
    { "tnsentry", T_OBJECT, offsetof(udt_SessionPool, dsn), READONLY },
    { "name", T_OBJECT, offsetof(udt_SessionPool, name), READONLY },
    { "max", T_INT, offsetof(udt_SessionPool, maxSessions), READONLY },
    { "min", T_INT, offsetof(udt_SessionPool, minSessions), READONLY },
    { "increment", T_INT, offsetof(udt_SessionPool, sessionIncrement),
            READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyGetSetDef g_SessionPoolCalcMembers[] = {
    { "opened", (getter) SessionPool_GetOCIAttr, 0, 0, &gc_OpenAttribute },
    { "busy", (getter) SessionPool_GetOCIAttr, 0, 0, &gc_BusyAttribute },
    { "timeout", (getter) SessionPool_GetOCIAttr,
            (setter) SessionPool_SetOCIAttr, 0, &gc_TimeoutAttribute },
    { "getmode", (getter) SessionPool_GetOCIAttr,
            (setter) SessionPool_SetOCIAttr, 0, &gc_GetModeAttribute },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type "SessionPool"
//-----------------------------------------------------------------------------
static PyTypeObject g_SessionPoolType = {
    PyObject_HEAD_INIT(NULL)
    0,					// ob_size
    "OracleSessionPool",		// tp_name
    sizeof(udt_SessionPool),		// tp_basicsize
    0,					// tp_itemsize
    (destructor) SessionPool_Free,	// tp_dealloc
    0,					// tp_print
    0,					// tp_getattr
    0,					// tp_setattr
    0,					// tp_compare
    0,					// tp_repr
    0,					// tp_as_number
    0,					// tp_as_sequence
    0,					// tp_as_mapping
    0,					// tp_hash
    0,					// tp_call
    0,					// tp_str
    0,					// tp_getattro
    0,					// tp_setattro
    0,					// tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
					// tp_flags
    0,					// tp_doc
    0,					// tp_traverse
    0,					// tp_clear
    0,					// tp_richcompare
    0,					// tp_weaklistoffset
    0,					// tp_iter
    0,					// tp_iternext
    g_SessionPoolMethods,		// tp_methods
    g_SessionPoolMembers,		// tp_members
    g_SessionPoolCalcMembers,		// tp_getset
    0,					// tp_base
    0,					// tp_dict
    0,					// tp_descr_get
    0,					// tp_descr_set
    0,					// tp_dictoffset
    (initproc) SessionPool_Init,	// tp_init
    0,					// tp_alloc
    (newfunc) SessionPool_New,		// tp_new
    0,					// tp_free
    0,					// tp_is_gc
    0					// tp_bases
};


#include "Connection.c"


//-----------------------------------------------------------------------------
// SessionPool_New()
//   Create a new session pool object.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_New(
    PyTypeObject *type,			// type object
    PyObject *args,			// arguments
    PyObject *keywordArgs)		// keyword arguments
{
    udt_SessionPool *newObject;

    // create the object
    newObject = (udt_SessionPool*) type->tp_alloc(type, 0);
    if (!newObject)
        return NULL;
    newObject->environment = NULL;

    return (PyObject*) newObject;
}


//-----------------------------------------------------------------------------
// SessionPool_Init()
//   Initialize the session pool object.
//-----------------------------------------------------------------------------
static int SessionPool_Init(
    udt_SessionPool *self,		// session pool object
    PyObject *args,			// arguments
    PyObject *keywordArgs)		// keyword arguments
{
    unsigned usernameLength, passwordLength, dsnLength, poolNameLength;
    const char *username, *password, *dsn, *poolName;
    unsigned minSessions, maxSessions, sessionIncrement;
    PyTypeObject *connectionType;
    PyObject *threadedObj;
    int threaded;
    sword status;
    ub1 getMode;

    // define keyword arguments
    static char *keywordList[] = { "user", "password", "dsn", "min", "max",
            "increment", "connectiontype", "threaded", "getmode", NULL };

    // parse arguments and keywords
    threaded = 0;
    threadedObj = NULL;
    connectionType = &g_ConnectionType;
    getMode = OCI_SPOOL_ATTRVAL_NOWAIT;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "s#s#s#iii|OOb",
            keywordList, &username, &usernameLength, &password,
            &passwordLength, &dsn, &dsnLength, &minSessions,
            &maxSessions, &sessionIncrement, &connectionType, &threadedObj,
            &getMode))
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
    if (threadedObj) {
        threaded = PyObject_IsTrue(threadedObj);
        if (threaded < 0)
            return -1;
    }

    // initialize the object's members
    Py_INCREF(connectionType);
    self->connectionType = connectionType;
    self->minSessions = minSessions;
    self->maxSessions = maxSessions;
    self->sessionIncrement = sessionIncrement;

    // set up the environment
    self->environment = Environment_New(threaded);
    if (!self->environment)
        return -1;

    // create the string for the username
    self->username = PyString_FromStringAndSize(username, usernameLength);
    if (!self->username)
        return -1;

    // create the string for the password
    self->password = PyString_FromStringAndSize(password, passwordLength);
    if (!self->password)
        return -1;

    // create the string for the TNS entry
    self->dsn = PyString_FromStringAndSize(dsn, dsnLength);
    if (!self->dsn)
        return -1;

    // create the session pool handle
    status = OCIHandleAlloc(self->environment->handle, (dvoid**) &self->handle,
            OCI_HTYPE_SPOOL, 0, 0);
    if (Environment_CheckForError(self->environment, status,
            "SessionPool_New(): allocate handle") < 0)
        return -1;

    // create the session pool
    Py_BEGIN_ALLOW_THREADS
    status = OCISessionPoolCreate(self->environment->handle,
            self->environment->errorHandle, self->handle,
            (OraText**) &poolName, &poolNameLength, (OraText*) dsn, dsnLength,
            minSessions, maxSessions, sessionIncrement, (OraText*) username,
            usernameLength, (OraText*) password, passwordLength,
            OCI_SPC_HOMOGENEOUS | OCI_SPC_STMTCACHE);
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(self->environment, status,
            "SessionPool_New(): create pool") < 0)
        return -1;

    // create the string for the pool name
    self->name = PyString_FromStringAndSize(poolName, poolNameLength);
    if (!self->name)
        return -1;

    // set the mode on the pool
    status = OCIAttrSet(self->handle, OCI_HTYPE_SPOOL, (dvoid*) &getMode, 0,
            OCI_ATTR_SPOOL_GETMODE, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "SessionPool_New(): set wait mode") < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// SessionPool_Free()
//   Deallocate the session pool.
//-----------------------------------------------------------------------------
static void SessionPool_Free(
    udt_SessionPool *self)		// session pool
{
    if (self->handle) {
        OCISessionPoolDestroy(self->handle, self->environment->errorHandle,
                OCI_SPD_FORCE);
        OCIHandleFree(self->handle, OCI_HTYPE_SPOOL);
    }
    Py_XDECREF(self->name);
    Py_XDECREF(self->environment);
    Py_XDECREF(self->username);
    Py_XDECREF(self->password);
    Py_XDECREF(self->dsn);
    self->ob_type->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// SessionPool_IsConnected()
//   Determines if the session pool object is connected to the database. If
// not, a Python exception is raised.
//-----------------------------------------------------------------------------
static int SessionPool_IsConnected(
    udt_SessionPool *self)		// session pool
{
    if (!self->handle) {
        PyErr_SetString(g_InterfaceErrorException, "not connected");
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// SessionPool_Acquire()
//   Create a new connection within the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_Acquire(
    udt_SessionPool *self,		// session pool
    PyObject *args)			// arguments
{
    PyObject *createArgs, *createKeywordArgs, *result;

    // make sure session pool is connected
    if (SessionPool_IsConnected(self) < 0)
        return NULL;

    // create arguments
    createArgs = PyTuple_New(0);
    if (!createArgs)
        return NULL;
    createKeywordArgs = PyDict_New();
    if (!createKeywordArgs) {
        Py_DECREF(createArgs);
        return NULL;
    }
    if (PyDict_SetItemString(createKeywordArgs, "pool",
            (PyObject*) self) < 0) {
        Py_DECREF(createArgs);
        Py_DECREF(createKeywordArgs);
        return NULL;
    }

    // create the connection object
    result = PyObject_Call( (PyObject*) self->connectionType, createArgs,
            createKeywordArgs);
    Py_DECREF(createArgs);
    Py_DECREF(createKeywordArgs);

    return result;
}


//-----------------------------------------------------------------------------
// SessionPool_InternalRelease()
//   Internal method used to release a connection back to the pool in order to
// allow for the possibility of dropping the connection.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_InternalRelease(
    udt_SessionPool *self,		// session pool
    PyObject *args,			// arguments
    ub4 mode)				// OCI mode to use
{
    udt_Connection *connection;
    sword status;

    // connection is expected
    if (!PyArg_ParseTuple(args, "O!", &g_ConnectionType, &connection))
        return NULL;

    // make sure session pool is connected
    if (SessionPool_IsConnected(self) < 0)
        return NULL;
    if (connection->sessionPool != self) {
        PyErr_SetString(g_ProgrammingErrorException,
                "connection not acquired with this session pool");
        return NULL;
    }

    // release the connection
    Py_BEGIN_ALLOW_THREADS
    status = OCITransRollback(connection->handle,
            connection->environment->errorHandle, OCI_DEFAULT);
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(connection->environment, status,
            "SessionPool_Release(): rollback") < 0)
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    status = OCISessionRelease(connection->handle,
            connection->environment->errorHandle, NULL, 0, mode);
    Py_END_ALLOW_THREADS
    if (Environment_CheckForError(connection->environment, status,
            "SessionPool_Release(): release session") < 0)
        return NULL;

    // ensure that the connection behaves as closed
    Py_DECREF(connection->sessionPool);
    connection->sessionPool = NULL;
    connection->handle = NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
// SessionPool_Drop()
//   Release a connection back to the session pool, dropping it so that a new
// connection will be created if needed.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_Drop(
    udt_SessionPool *self,		// session pool
    PyObject *args)			// arguments
{
    return SessionPool_InternalRelease(self, args, OCI_SESSRLS_DROPSESS);
}


//-----------------------------------------------------------------------------
// SessionPool_Release()
//   Release a connection back to the session pool.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_Release(
    udt_SessionPool *self,		// session pool
    PyObject *args)			// arguments
{
    return SessionPool_InternalRelease(self, args, OCI_DEFAULT);
}


//-----------------------------------------------------------------------------
// SessionPool_GetOCIAttr()
//   Return the value for the OCI attribute.
//-----------------------------------------------------------------------------
static PyObject *SessionPool_GetOCIAttr(
    udt_SessionPool *self,		// session pool
    ub4 *attribute)			// OCI attribute type
{
    sword status;
    ub4 value;

    // make sure session pool is connected
    if (SessionPool_IsConnected(self) < 0)
        return NULL;

    // get the value from the OCI
    status = OCIAttrGet(self->handle, OCI_HTYPE_SPOOL, &value, 0, *attribute,
            self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "SessionPool_GetOCIAttr()") < 0)
        return NULL;
    if (*attribute == OCI_ATTR_SPOOL_GETMODE)
        return PyInt_FromLong((ub1) value);
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// SessionPool_SetOCIAttr()
//   Set the value of the OCI attribute.
//-----------------------------------------------------------------------------
static int SessionPool_SetOCIAttr(
    udt_SessionPool *self,		// session pool
    PyObject *value,			// value to set
    ub4 *attribute)			// OCI attribute type
{
    ub4 ociValue;
    sword status;

    // make sure session pool is connected
    if (SessionPool_IsConnected(self) < 0)
        return -1;

    // set the value in the OCI
    if (!PyInt_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "value must be an integer");
        return -1;
    }
    ociValue = PyInt_AS_LONG(value);
    status = OCIAttrSet(self->handle, OCI_HTYPE_SPOOL, &ociValue, 0,
            *attribute, self->environment->errorHandle);
    if (Environment_CheckForError(self->environment, status,
            "SessionPool_SetOCIAttr()") < 0)
        return -1;
    return 0;
}

