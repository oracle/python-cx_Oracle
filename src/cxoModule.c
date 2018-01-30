//-----------------------------------------------------------------------------
// Copyright 2016-2018, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//
// Licensed under BSD license (see LICENSE.txt).
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoModule.c
//   Implementation of cx_Oracle module.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

// define macro for adding integer constants
#define ADD_INT_CONSTANT(name, value) \
    if (PyModule_AddIntConstant(module, name, value) < 0) \
        return NULL;

// define macro for adding type objects
#define ADD_TYPE_OBJECT(name, type) \
    Py_INCREF(type); \
    if (PyModule_AddObject(module, name, (PyObject*) type) < 0) \
        return NULL;

// define macro for and making types ready
#define MAKE_TYPE_READY(type) \
    if (PyType_Ready(type) < 0) \
        return NULL;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
PyObject *cxoWarningException = NULL;
PyObject *cxoErrorException = NULL;
PyObject *cxoInterfaceErrorException = NULL;
PyObject *cxoDatabaseErrorException = NULL;
PyObject *cxoDataErrorException = NULL;
PyObject *cxoOperationalErrorException = NULL;
PyObject *cxoIntegrityErrorException = NULL;
PyObject *cxoInternalErrorException = NULL;
PyObject *cxoProgrammingErrorException = NULL;
PyObject *cxoNotSupportedErrorException = NULL;
dpiContext *cxoDpiContext = NULL;
dpiVersionInfo cxoClientVersionInfo;

//-----------------------------------------------------------------------------
// SetException()
//   Create an exception and set it in the provided dictionary.
//-----------------------------------------------------------------------------
static int SetException(PyObject *module, PyObject **exception, char *name,
        PyObject *baseException)
{
    char buffer[100];

    sprintf(buffer, "cx_Oracle.%s", name);
    *exception = PyErr_NewException(buffer, baseException, NULL);
    if (!*exception)
        return -1;
    return PyModule_AddObject(module, name, *exception);
}


//-----------------------------------------------------------------------------
// MakeDSN()
//   Make a data source name given the host port and SID.
//-----------------------------------------------------------------------------
static PyObject* MakeDSN(PyObject* self, PyObject* args, PyObject* keywordArgs)
{
    static unsigned int numConnectDataArgs = 5;
    static char *keywordList[] = { "host", "port", "sid", "service_name",
            "region", "sharding_key", "super_sharding_key", NULL };
    PyObject *result, *connectData, *hostObj, *portObj;
    char connectDataFormat[72], *sourcePtr, *targetPtr;
    PyObject *connectDataArgs[5], *formatArgsArray;
    unsigned int i;

    // parse arguments
    for (i = 0; i < numConnectDataArgs; i++)
        connectDataArgs[i] = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO|OOOOO",
            keywordList, &hostObj, &portObj, &connectDataArgs[0],
            &connectDataArgs[1], &connectDataArgs[2], &connectDataArgs[3],
            &connectDataArgs[4]))
        return NULL;

    // create list for connect data format arguments
    formatArgsArray = PyList_New(0);
    if (!formatArgsArray)
        return NULL;

    // process each of the connect data arguments
    // build up a format string and a list of format arguments
    targetPtr = connectDataFormat;
    *targetPtr = '\0';
    for (i = 0; i < numConnectDataArgs; i++) {
        if (connectDataArgs[i]) {
            if (PyList_Append(formatArgsArray, connectDataArgs[i]) < 0) {
                Py_DECREF(formatArgsArray);
                return NULL;
            }
            sourcePtr = keywordList[i + 2];
            *targetPtr++ = '(';
            while (*sourcePtr)
                *targetPtr++ = toupper(*sourcePtr++);
            *targetPtr++ = '=';
            *targetPtr++ = '%';
            *targetPtr++ = 's';
            *targetPtr++ = ')';
            *targetPtr = '\0';
        }
    }

    // determine connect data
    connectData = cxoUtils_formatString(connectDataFormat,
            PyList_AsTuple(formatArgsArray));
    Py_DECREF(formatArgsArray);
    if (!connectData)
        return NULL;

    // perform overall format
    result = cxoUtils_formatString("(DESCRIPTION=(ADDRESS="
            "(PROTOCOL=TCP)(HOST=%s)(PORT=%s))(CONNECT_DATA=%s))",
            PyTuple_Pack(3, hostObj, portObj, connectData));
    Py_DECREF(connectData);
    return result;
}


//-----------------------------------------------------------------------------
// ClientVersion()
//   Return the version of the Oracle client being used as a 5-tuple.
//-----------------------------------------------------------------------------
static PyObject* ClientVersion(PyObject* self, PyObject* args)
{
    if (cxoUtils_initializeDPI() < 0)
        return NULL;
    return Py_BuildValue("(iiiii)", cxoClientVersionInfo.versionNum,
            cxoClientVersionInfo.releaseNum, cxoClientVersionInfo.updateNum,
            cxoClientVersionInfo.portReleaseNum,
            cxoClientVersionInfo.portUpdateNum);
}


//-----------------------------------------------------------------------------
// Time()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* Time(PyObject* self, PyObject* args)
{
    PyErr_SetString(cxoNotSupportedErrorException,
            "Oracle does not support time only variables");
    return NULL;
}


//-----------------------------------------------------------------------------
// TimeFromTicks()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimeFromTicks(PyObject* self, PyObject* args)
{
    PyErr_SetString(cxoNotSupportedErrorException,
            "Oracle does not support time only variables");
    return NULL;
}


//-----------------------------------------------------------------------------
// DateFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* DateFromTicks(PyObject* self, PyObject* args)
{
    return cxoTransform_dateFromTicks(args);
}


//-----------------------------------------------------------------------------
// TimestampFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimestampFromTicks(PyObject* self, PyObject* args)
{
    return cxoTransform_timestampFromTicks(args);
}


//-----------------------------------------------------------------------------
//   Declaration of methods supported by this module
//-----------------------------------------------------------------------------
static PyMethodDef cxoModuleMethods[] = {
    { "makedsn", (PyCFunction) MakeDSN, METH_VARARGS | METH_KEYWORDS },
    { "Time", (PyCFunction) Time, METH_VARARGS },
    { "DateFromTicks", (PyCFunction) DateFromTicks, METH_VARARGS },
    { "TimeFromTicks", (PyCFunction) TimeFromTicks, METH_VARARGS },
    { "TimestampFromTicks", (PyCFunction) TimestampFromTicks, METH_VARARGS },
    { "clientversion", (PyCFunction) ClientVersion, METH_NOARGS },
    { NULL }
};


#if PY_MAJOR_VERSION >= 3
//-----------------------------------------------------------------------------
//   Declaration of module definition for Python 3.x.
//-----------------------------------------------------------------------------
static struct PyModuleDef cxoModuleDef = {
    PyModuleDef_HEAD_INIT,
    "cx_Oracle",
    NULL,
    -1,
    cxoModuleMethods,                      // methods
    NULL,                                  // m_reload
    NULL,                                  // traverse
    NULL,                                  // clear
    NULL                                   // free
};
#endif


//-----------------------------------------------------------------------------
// Module_Initialize()
//   Initialization routine for the module.
//-----------------------------------------------------------------------------
static PyObject *Module_Initialize(void)
{
    PyObject *module;

#ifdef WITH_THREAD
    PyEval_InitThreads();
#endif

    // initialize transforms
    if (cxoTransform_init() < 0)
        return NULL;

    // prepare the types for use by the module
    MAKE_TYPE_READY(&cxoPyTypeBfileVar);
    MAKE_TYPE_READY(&cxoPyTypeBinaryVar);
    MAKE_TYPE_READY(&cxoPyTypeBlobVar);
    MAKE_TYPE_READY(&cxoPyTypeBooleanVar);
    MAKE_TYPE_READY(&cxoPyTypeClobVar);
    MAKE_TYPE_READY(&cxoPyTypeConnection);
    MAKE_TYPE_READY(&cxoPyTypeCursor);
    MAKE_TYPE_READY(&cxoPyTypeCursorVar);
    MAKE_TYPE_READY(&cxoPyTypeDateTimeVar);
    MAKE_TYPE_READY(&cxoPyTypeDeqOptions);
    MAKE_TYPE_READY(&cxoPyTypeEnqOptions);
    MAKE_TYPE_READY(&cxoPyTypeError);
    MAKE_TYPE_READY(&cxoPyTypeFixedCharVar);
    MAKE_TYPE_READY(&cxoPyTypeFixedNcharVar);
    MAKE_TYPE_READY(&cxoPyTypeIntervalVar);
    MAKE_TYPE_READY(&cxoPyTypeLob);
    MAKE_TYPE_READY(&cxoPyTypeLongBinaryVar);
    MAKE_TYPE_READY(&cxoPyTypeLongStringVar);
    MAKE_TYPE_READY(&cxoPyTypeMsgProps);
    MAKE_TYPE_READY(&cxoPyTypeMessage);
    MAKE_TYPE_READY(&cxoPyTypeMessageQuery);
    MAKE_TYPE_READY(&cxoPyTypeMessageRow);
    MAKE_TYPE_READY(&cxoPyTypeMessageTable);
    MAKE_TYPE_READY(&cxoPyTypeNativeFloatVar);
    MAKE_TYPE_READY(&cxoPyTypeNativeIntVar);
    MAKE_TYPE_READY(&cxoPyTypeNcharVar);
    MAKE_TYPE_READY(&cxoPyTypeNclobVar);
    MAKE_TYPE_READY(&cxoPyTypeNumberVar);
    MAKE_TYPE_READY(&cxoPyTypeObjectAttr);
    MAKE_TYPE_READY(&cxoPyTypeObject);
    MAKE_TYPE_READY(&cxoPyTypeObjectType);
    MAKE_TYPE_READY(&cxoPyTypeObjectVar);
    MAKE_TYPE_READY(&cxoPyTypeRowidVar);
    MAKE_TYPE_READY(&cxoPyTypeSessionPool);
    MAKE_TYPE_READY(&cxoPyTypeStringVar);
    MAKE_TYPE_READY(&cxoPyTypeSubscr);
    MAKE_TYPE_READY(&cxoPyTypeTimestampVar);

    // initialize module and retrieve the dictionary
#if PY_MAJOR_VERSION >= 3
    module = PyModule_Create(&cxoModuleDef);
#else
    module = Py_InitModule("cx_Oracle", cxoModuleMethods);
#endif
    if (!module)
        return NULL;

    // create exception object and add it to the dictionary
    if (SetException(module, &cxoWarningException,
            "Warning", CXO_BASE_EXCEPTION) < 0)
        return NULL;
    if (SetException(module, &cxoErrorException,
            "Error", CXO_BASE_EXCEPTION) < 0)
        return NULL;
    if (SetException(module, &cxoInterfaceErrorException,
            "InterfaceError", cxoErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoDatabaseErrorException,
            "DatabaseError", cxoErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoDataErrorException,
            "DataError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoOperationalErrorException,
            "OperationalError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoIntegrityErrorException,
            "IntegrityError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoInternalErrorException,
            "InternalError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoProgrammingErrorException,
            "ProgrammingError", cxoDatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &cxoNotSupportedErrorException,
            "NotSupportedError", cxoDatabaseErrorException) < 0)
        return NULL;

    // set up the types that are available
#if PY_MAJOR_VERSION >= 3
    ADD_TYPE_OBJECT("Binary", &PyBytes_Type)
#else
    ADD_TYPE_OBJECT("Binary", &PyBuffer_Type)
#endif
    ADD_TYPE_OBJECT("Connection", &cxoPyTypeConnection)
    ADD_TYPE_OBJECT("Cursor", &cxoPyTypeCursor)
    ADD_TYPE_OBJECT("Timestamp", cxoPyTypeDateTime)
    ADD_TYPE_OBJECT("Date", cxoPyTypeDate)
    ADD_TYPE_OBJECT("SessionPool", &cxoPyTypeSessionPool)
    ADD_TYPE_OBJECT("_Error", &cxoPyTypeError)
    ADD_TYPE_OBJECT("Object", &cxoPyTypeObject)
    ADD_TYPE_OBJECT("ObjectType", &cxoPyTypeObjectType)
    ADD_TYPE_OBJECT("EnqOptions", &cxoPyTypeEnqOptions)
    ADD_TYPE_OBJECT("DeqOptions", &cxoPyTypeDeqOptions)
    ADD_TYPE_OBJECT("MessageProperties", &cxoPyTypeMsgProps)

    // the name "connect" is required by the DB API
    ADD_TYPE_OBJECT("connect", &cxoPyTypeConnection)

    // create the basic data types for setting input sizes
    ADD_TYPE_OBJECT("BINARY", &cxoPyTypeBinaryVar)
    ADD_TYPE_OBJECT("BFILE", &cxoPyTypeBfileVar)
    ADD_TYPE_OBJECT("BLOB", &cxoPyTypeBlobVar)
    ADD_TYPE_OBJECT("CLOB", &cxoPyTypeClobVar)
    ADD_TYPE_OBJECT("CURSOR", &cxoPyTypeCursorVar)
    ADD_TYPE_OBJECT("OBJECT", &cxoPyTypeObjectVar)
    ADD_TYPE_OBJECT("DATETIME", &cxoPyTypeDateTimeVar)
    ADD_TYPE_OBJECT("FIXED_CHAR", &cxoPyTypeFixedCharVar)
    ADD_TYPE_OBJECT("FIXED_NCHAR", &cxoPyTypeFixedNcharVar)
    ADD_TYPE_OBJECT("NCHAR", &cxoPyTypeNcharVar)
    ADD_TYPE_OBJECT("INTERVAL", &cxoPyTypeIntervalVar)
    ADD_TYPE_OBJECT("LOB", &cxoPyTypeLob)
    ADD_TYPE_OBJECT("LONG_BINARY", &cxoPyTypeLongBinaryVar)
    ADD_TYPE_OBJECT("LONG_STRING", &cxoPyTypeLongStringVar)
    ADD_TYPE_OBJECT("NCLOB", &cxoPyTypeNclobVar)
    ADD_TYPE_OBJECT("NUMBER", &cxoPyTypeNumberVar)
    ADD_TYPE_OBJECT("ROWID", &cxoPyTypeRowidVar)
    ADD_TYPE_OBJECT("STRING", &cxoPyTypeStringVar)
    ADD_TYPE_OBJECT("TIMESTAMP", &cxoPyTypeTimestampVar)
    ADD_TYPE_OBJECT("NATIVE_INT", &cxoPyTypeNativeIntVar)
    ADD_TYPE_OBJECT("NATIVE_FLOAT", &cxoPyTypeNativeFloatVar)
    ADD_TYPE_OBJECT("BOOLEAN", &cxoPyTypeBooleanVar)

    // create constants required by Python DB API 2.0
    if (PyModule_AddStringConstant(module, "apilevel", "2.0") < 0)
        return NULL;
    if (PyModule_AddIntConstant(module, "threadsafety", 2) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "paramstyle", "named") < 0)
        return NULL;

    // add version and build time for easier support
    if (PyModule_AddStringConstant(module, "version",
            CXO_BUILD_VERSION_STRING) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "__version__",
            CXO_BUILD_VERSION_STRING) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "buildtime",
            __DATE__ " " __TIME__) < 0)
        return NULL;

    // add constants for authorization modes
    ADD_INT_CONSTANT("SYSASM", DPI_MODE_AUTH_SYSASM)
    ADD_INT_CONSTANT("SYSBKP", DPI_MODE_AUTH_SYSBKP)
    ADD_INT_CONSTANT("SYSDBA", DPI_MODE_AUTH_SYSDBA)
    ADD_INT_CONSTANT("SYSDGD", DPI_MODE_AUTH_SYSDGD)
    ADD_INT_CONSTANT("SYSKMT", DPI_MODE_AUTH_SYSKMT)
    ADD_INT_CONSTANT("SYSOPER", DPI_MODE_AUTH_SYSOPER)
    ADD_INT_CONSTANT("SYSRAC", DPI_MODE_AUTH_SYSRAC)
    ADD_INT_CONSTANT("PRELIM_AUTH", DPI_MODE_AUTH_PRELIM)

    // add constants for session pool get modes
    ADD_INT_CONSTANT("SPOOL_ATTRVAL_WAIT", DPI_MODE_POOL_GET_WAIT)
    ADD_INT_CONSTANT("SPOOL_ATTRVAL_NOWAIT", DPI_MODE_POOL_GET_NOWAIT)
    ADD_INT_CONSTANT("SPOOL_ATTRVAL_FORCEGET", DPI_MODE_POOL_GET_FORCEGET)

    // add constants for database shutdown modes
    ADD_INT_CONSTANT("DBSHUTDOWN_ABORT", DPI_MODE_SHUTDOWN_ABORT)
    ADD_INT_CONSTANT("DBSHUTDOWN_FINAL", DPI_MODE_SHUTDOWN_FINAL)
    ADD_INT_CONSTANT("DBSHUTDOWN_IMMEDIATE", DPI_MODE_SHUTDOWN_IMMEDIATE)
    ADD_INT_CONSTANT("DBSHUTDOWN_TRANSACTIONAL",
            DPI_MODE_SHUTDOWN_TRANSACTIONAL)
    ADD_INT_CONSTANT("DBSHUTDOWN_TRANSACTIONAL_LOCAL",
            DPI_MODE_SHUTDOWN_TRANSACTIONAL_LOCAL)

    // add constants for purity
    ADD_INT_CONSTANT("ATTR_PURITY_DEFAULT", DPI_PURITY_DEFAULT)
    ADD_INT_CONSTANT("ATTR_PURITY_NEW", DPI_PURITY_NEW)
    ADD_INT_CONSTANT("ATTR_PURITY_SELF", DPI_PURITY_SELF)

    // add constants for subscription protocols
    ADD_INT_CONSTANT("SUBSCR_PROTO_OCI", DPI_SUBSCR_PROTO_CALLBACK)
    ADD_INT_CONSTANT("SUBSCR_PROTO_MAIL", DPI_SUBSCR_PROTO_MAIL)
    ADD_INT_CONSTANT("SUBSCR_PROTO_SERVER", DPI_SUBSCR_PROTO_PLSQL)
    ADD_INT_CONSTANT("SUBSCR_PROTO_HTTP", DPI_SUBSCR_PROTO_HTTP)

    // add constants for subscription quality of service
    ADD_INT_CONSTANT("SUBSCR_QOS_RELIABLE", DPI_SUBSCR_QOS_RELIABLE)
    ADD_INT_CONSTANT("SUBSCR_QOS_DEREG_NFY", DPI_SUBSCR_QOS_DEREG_NFY)
    ADD_INT_CONSTANT("SUBSCR_QOS_ROWIDS", DPI_SUBSCR_QOS_ROWIDS)
    ADD_INT_CONSTANT("SUBSCR_QOS_QUERY", DPI_SUBSCR_QOS_QUERY)
    ADD_INT_CONSTANT("SUBSCR_QOS_BEST_EFFORT", DPI_SUBSCR_QOS_BEST_EFFORT)

    // add constants for subscription namespaces
    ADD_INT_CONSTANT("SUBSCR_NAMESPACE_DBCHANGE",
            DPI_SUBSCR_NAMESPACE_DBCHANGE)

    // add constants for event types
    ADD_INT_CONSTANT("EVENT_NONE", DPI_EVENT_NONE)
    ADD_INT_CONSTANT("EVENT_STARTUP", DPI_EVENT_STARTUP)
    ADD_INT_CONSTANT("EVENT_SHUTDOWN", DPI_EVENT_SHUTDOWN)
    ADD_INT_CONSTANT("EVENT_SHUTDOWN_ANY", DPI_EVENT_SHUTDOWN_ANY)
    ADD_INT_CONSTANT("EVENT_DEREG", DPI_EVENT_DEREG)
    ADD_INT_CONSTANT("EVENT_OBJCHANGE", DPI_EVENT_OBJCHANGE)
    ADD_INT_CONSTANT("EVENT_QUERYCHANGE", DPI_EVENT_QUERYCHANGE)

    // add constants for opcodes
    ADD_INT_CONSTANT("OPCODE_ALLOPS", DPI_OPCODE_ALL_OPS)
    ADD_INT_CONSTANT("OPCODE_ALLROWS", DPI_OPCODE_ALL_ROWS)
    ADD_INT_CONSTANT("OPCODE_INSERT", DPI_OPCODE_INSERT)
    ADD_INT_CONSTANT("OPCODE_UPDATE", DPI_OPCODE_UPDATE)
    ADD_INT_CONSTANT("OPCODE_DELETE", DPI_OPCODE_DELETE)
    ADD_INT_CONSTANT("OPCODE_ALTER", DPI_OPCODE_ALTER)
    ADD_INT_CONSTANT("OPCODE_DROP", DPI_OPCODE_DROP)

    // add constants for AQ dequeue modes
    ADD_INT_CONSTANT("DEQ_BROWSE", DPI_MODE_DEQ_BROWSE)
    ADD_INT_CONSTANT("DEQ_LOCKED", DPI_MODE_DEQ_LOCKED)
    ADD_INT_CONSTANT("DEQ_REMOVE", DPI_MODE_DEQ_REMOVE)
    ADD_INT_CONSTANT("DEQ_REMOVE_NODATA", DPI_MODE_DEQ_REMOVE_NO_DATA)

    // add constants for AQ dequeue navigation
    ADD_INT_CONSTANT("DEQ_FIRST_MSG", DPI_DEQ_NAV_FIRST_MSG)
    ADD_INT_CONSTANT("DEQ_NEXT_TRANSACTION", DPI_DEQ_NAV_NEXT_TRANSACTION)
    ADD_INT_CONSTANT("DEQ_NEXT_MSG", DPI_DEQ_NAV_NEXT_MSG)

    // add constants for AQ dequeue visibility
    ADD_INT_CONSTANT("DEQ_IMMEDIATE", DPI_VISIBILITY_IMMEDIATE)
    ADD_INT_CONSTANT("DEQ_ON_COMMIT", DPI_VISIBILITY_ON_COMMIT)

    // add constants for AQ dequeue wait
    ADD_INT_CONSTANT("DEQ_NO_WAIT", DPI_DEQ_WAIT_NO_WAIT)
    ADD_INT_CONSTANT("DEQ_WAIT_FOREVER", DPI_DEQ_WAIT_FOREVER)

    // add constants for AQ enqueue visibility
    ADD_INT_CONSTANT("ENQ_IMMEDIATE", DPI_VISIBILITY_IMMEDIATE)
    ADD_INT_CONSTANT("ENQ_ON_COMMIT", DPI_VISIBILITY_ON_COMMIT)

    // add constants for AQ table purge mode (message)
    ADD_INT_CONSTANT("MSG_PERSISTENT", DPI_MODE_MSG_PERSISTENT)
    ADD_INT_CONSTANT("MSG_BUFFERED", DPI_MODE_MSG_BUFFERED)
    ADD_INT_CONSTANT("MSG_PERSISTENT_OR_BUFFERED",
            DPI_MODE_MSG_PERSISTENT_OR_BUFFERED)

    // add constants for AQ message state
    ADD_INT_CONSTANT("MSG_EXPIRED", DPI_MSG_STATE_EXPIRED)
    ADD_INT_CONSTANT("MSG_READY", DPI_MSG_STATE_READY)
    ADD_INT_CONSTANT("MSG_PROCESSED", DPI_MSG_STATE_PROCESSED)
    ADD_INT_CONSTANT("MSG_WAITING", DPI_MSG_STATE_WAITING)

    // add special constants for AQ delay/expiration
    ADD_INT_CONSTANT("MSG_NO_DELAY", 0)
    ADD_INT_CONSTANT("MSG_NO_EXPIRATION", -1)

    return module;
}


//-----------------------------------------------------------------------------
// Start routine for the module.
//-----------------------------------------------------------------------------
#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_cx_Oracle(void)
{
    return Module_Initialize();
}
#else
void initcx_Oracle(void)
{
    Module_Initialize();
}
#endif

