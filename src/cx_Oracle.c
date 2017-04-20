//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//
// Licensed under BSD license (see LICENSE.txt).
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cx_Oracle.c
//   Shared library for use by Python.
//-----------------------------------------------------------------------------

#include <Python.h>
#include <datetime.h>
#include <structmember.h>
#include <time.h>
#include <dpi.h>

// define integer macros/methods for Python 3.x
#ifndef PyInt_Check
#define PyInt_Check                     PyLong_Check
#define PyInt_FromLong                  PyLong_FromLong
#define PyInt_FromUnsignedLong          PyLong_FromUnsignedLong
#define PyInt_AsLong                    PyLong_AsLong
#define PyInt_AsUnsignedLong            PyLong_AsUnsignedLong
#define PyInt_Type                      PyLong_Type
#define PyNumber_Int                    PyNumber_Long
#endif

// use the bytes methods in cx_Oracle and define them as the equivalent string
// type methods as is done in Python 2.6
#ifndef PyBytes_Check
    #define PyBytes_Type                PyString_Type
    #define PyBytes_AS_STRING           PyString_AS_STRING
    #define PyBytes_GET_SIZE            PyString_GET_SIZE
    #define PyBytes_Check               PyString_Check
    #define PyBytes_Format              PyString_Format
    #define PyBytes_FromString          PyString_FromString
    #define PyBytes_FromStringAndSize   PyString_FromStringAndSize
#endif

// define types and methods for strings and binary data
#if PY_MAJOR_VERSION >= 3
    #define cxBinary_Type               PyBytes_Type
    #define cxBinary_Check              PyBytes_Check
    #define cxString_Type               &PyUnicode_Type
    #define cxString_Format             PyUnicode_Format
    #define cxString_Check              PyUnicode_Check
    #define cxString_GetSize            PyUnicode_GET_LENGTH
#else
    #define cxBinary_Type               PyBuffer_Type
    #define cxBinary_Check              PyBuffer_Check
    #define cxString_Type               &PyBytes_Type
    #define cxString_Format             PyBytes_Format
    #define cxString_Check              PyBytes_Check
    #define cxString_GetSize            PyBytes_GET_SIZE
#endif

// define string type and methods
#if PY_MAJOR_VERSION >= 3
    #define cxString_FromAscii(str) \
        PyUnicode_DecodeASCII(str, strlen(str), NULL)
    #define cxString_FromEncodedString(buffer, numBytes, encoding) \
        PyUnicode_Decode(buffer, numBytes, encoding, NULL)
#else
    #define cxString_FromAscii(str) \
        PyBytes_FromString(str)
    #define cxString_FromEncodedString(buffer, numBytes, encoding) \
        PyBytes_FromStringAndSize(buffer, numBytes)
#endif

// define base exception
#if PY_MAJOR_VERSION >= 3
#define CXORA_BASE_EXCEPTION    NULL
#define CXORA_TYPE_ERROR        "expecting string or bytes object"
#else
#define CXORA_BASE_EXCEPTION    PyExc_StandardError
#define CXORA_TYPE_ERROR        "expecting string, unicode or buffer object"
#endif

// define macro for adding integer constants
#define ADD_INT_CONSTANT(name, value) \
    if (PyModule_AddIntConstant(module, name, value) < 0) \
        return NULL;

// define macro for adding type objects
#define ADD_TYPE_OBJECT(name, type) \
    Py_INCREF(type); \
    if (PyModule_AddObject(module, name, (PyObject*) type) < 0) \
        return NULL;

// define macros for defining and making variable types ready
#define DECLARE_VARIABLE_TYPE(INTERNAL_NAME, EXTERNAL_NAME) \
    static PyTypeObject INTERNAL_NAME = { \
        PyVarObject_HEAD_INIT(NULL, 0) \
        "cx_Oracle." #EXTERNAL_NAME,        /* tp_name */ \
        sizeof(udt_Variable),               /* tp_basicsize */ \
        0,                                  /* tp_itemsize */ \
        (destructor) Variable_Free,         /* tp_dealloc */ \
        0,                                  /* tp_print */ \
        0,                                  /* tp_getattr */ \
        0,                                  /* tp_setattr */ \
        0,                                  /* tp_compare */ \
        (reprfunc) Variable_Repr,           /* tp_repr */ \
        0,                                  /* tp_as_number */ \
        0,                                  /* tp_as_sequence */ \
        0,                                  /* tp_as_mapping */ \
        0,                                  /* tp_hash */ \
        0,                                  /* tp_call */ \
        0,                                  /* tp_str */ \
        0,                                  /* tp_getattro */ \
        0,                                  /* tp_setattro */ \
        0,                                  /* tp_as_buffer */ \
        Py_TPFLAGS_DEFAULT,                 /* tp_flags */ \
        0,                                  /* tp_doc */ \
        0,                                  /* tp_traverse */ \
        0,                                  /* tp_clear */ \
        0,                                  /* tp_richcompare */ \
        0,                                  /* tp_weaklistoffset */ \
        0,                                  /* tp_iter */ \
        0,                                  /* tp_iternext */ \
        g_VariableMethods,                  /* tp_methods */ \
        g_VariableMembers,                  /* tp_members */ \
        g_VariableCalcMembers               /* tp_getset */ \
    };

#define MAKE_TYPE_READY(type) \
    if (PyType_Ready(type) < 0) \
        return NULL;

// define macros to get the build version as a string and the driver name
#define xstr(s)                 str(s)
#define str(s)                  #s
#define BUILD_VERSION_STRING    xstr(BUILD_VERSION)
#define DRIVER_NAME             "cx_Oracle : "BUILD_VERSION_STRING

#include "Buffer.c"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
static PyObject *g_WarningException = NULL;
static PyObject *g_ErrorException = NULL;
static PyObject *g_InterfaceErrorException = NULL;
static PyObject *g_DatabaseErrorException = NULL;
static PyObject *g_DataErrorException = NULL;
static PyObject *g_OperationalErrorException = NULL;
static PyObject *g_IntegrityErrorException = NULL;
static PyObject *g_InternalErrorException = NULL;
static PyObject *g_ProgrammingErrorException = NULL;
static PyObject *g_NotSupportedErrorException = NULL;
static PyTypeObject *g_DecimalType = NULL;
static dpiContext *g_DpiContext = NULL;


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
// GetModuleAndName()
//   Return the module and name for the type.
//-----------------------------------------------------------------------------
static int GetModuleAndName(PyTypeObject *type, PyObject **module,
        PyObject **name)
{
    *module = PyObject_GetAttrString( (PyObject*) type, "__module__");
    if (!*module)
        return -1;
    *name = PyObject_GetAttrString( (PyObject*) type, "__name__");
    if (!*name) {
        Py_DECREF(*module);
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// GetBooleanValue()
//   Get a boolean value from a Python object.
//-----------------------------------------------------------------------------
static int GetBooleanValue(PyObject *obj, int defaultValue, int *value)
{
    if (!obj)
        *value = defaultValue;
    else {
        *value = PyObject_IsTrue(obj);
        if (*value < 0)
            return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// GetAdjustedEncoding()
//   Return the adjusted encoding to use when encoding and decoding strings
// that are passed to and from the Oracle database. The Oracle client interface
// does not support the inclusion of a BOM in the encoded string but assumes
// native endian order for UTF-16. Python generates a BOM at the beginning of
// the encoded string if plain UTF-16 is specified. For this reason, the
// correct byte order must be determined and used inside Python so that the
// Oracle client receives the data it expects.
//-----------------------------------------------------------------------------
static const char *GetAdjustedEncoding(const char *encoding)
{
    static const union {
        unsigned char bytes[4];
        uint32_t value;
    } hostOrder = { { 0, 1, 2, 3 } };

    if (!encoding || strcmp(encoding, "UTF-16") != 0)
        return encoding;
    return (hostOrder.value == 0x03020100) ? "UTF-16LE" : "UTF-16BE";
}


#include "Error.c"
#include "SessionPool.c"


//-----------------------------------------------------------------------------
// MakeDSN()
//   Make a data source name given the host port and SID.
//-----------------------------------------------------------------------------
static PyObject* MakeDSN(PyObject* self, PyObject* args, PyObject* keywordArgs)
{
    static unsigned int numConnectDataArgs = 5;
    static char *keywordList[] = { "host", "port", "sid", "service_name",
            "region", "sharding_key", "super_sharding_key", NULL };
    PyObject *format, *formatArgs, *result, *connectData, *hostObj, *portObj;
    char connectDataFormat[72], *sourcePtr, *targetPtr;
    PyObject *connectDataArgs[5], *formatArgsArray;
    unsigned int i;

    // parse arguments
    for (i = 0; i < numConnectDataArgs; i++)
        connectDataArgs[i] = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "OO|OOOO0",
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
    formatArgs = PyList_AsTuple(formatArgsArray);
    Py_DECREF(formatArgsArray);
    if (!formatArgs)
        return NULL;

    // determine connect data
    format = cxString_FromAscii(connectDataFormat);
    if (!format) {
        Py_DECREF(formatArgs);
        return NULL;
    }

    connectData = cxString_Format(format, formatArgs);
    Py_DECREF(format);
    Py_DECREF(formatArgs);
    if (!connectData)
        return NULL;

    // perform overall format
    format = cxString_FromAscii("(DESCRIPTION=(ADDRESS="
            "(PROTOCOL=TCP)(HOST=%s)(PORT=%s))(CONNECT_DATA=%s))");
    if (!format) {
        Py_DECREF(connectData);
        return NULL;
    }
    formatArgs = PyTuple_Pack(3, hostObj, portObj, connectData);
    Py_DECREF(connectData);
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
// ClientVersion()
//   Return the version of the Oracle client being used as a 5-tuple.
//-----------------------------------------------------------------------------
static PyObject* ClientVersion(PyObject* self, PyObject* args)
{
    dpiVersionInfo versionInfo;

    if (dpiContext_getClientVersion(g_DpiContext, &versionInfo) < 0)
        return Error_RaiseAndReturnNull();
    return Py_BuildValue("(iiiii)", versionInfo.versionNum,
            versionInfo.releaseNum, versionInfo.updateNum,
            versionInfo.portReleaseNum, versionInfo.portUpdateNum);
}


//-----------------------------------------------------------------------------
// Time()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* Time(PyObject* self, PyObject* args)
{
    PyErr_SetString(g_NotSupportedErrorException,
            "Oracle does not support time only variables");
    return NULL;
}


//-----------------------------------------------------------------------------
// TimeFromTicks()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimeFromTicks(PyObject* self, PyObject* args)
{
    PyErr_SetString(g_NotSupportedErrorException,
            "Oracle does not support time only variables");
    return NULL;
}


//-----------------------------------------------------------------------------
// DateFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* DateFromTicks(PyObject* self, PyObject* args)
{
    return PyDate_FromTimestamp(args);
}


//-----------------------------------------------------------------------------
// TimestampFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimestampFromTicks(PyObject* self, PyObject* args)
{
    return PyDateTime_FromTimestamp(args);
}


//-----------------------------------------------------------------------------
//   Declaration of methods supported by this module
//-----------------------------------------------------------------------------
static PyMethodDef g_ModuleMethods[] = {
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
static struct PyModuleDef g_ModuleDef = {
    PyModuleDef_HEAD_INIT,
    "cx_Oracle",
    NULL,
    -1,
    g_ModuleMethods,                       // methods
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
    dpiErrorInfo errorInfo;
    PyObject *module;

#ifdef WITH_THREAD
    PyEval_InitThreads();
#endif

    // import the datetime module for datetime support
    PyDateTime_IMPORT;
    if (PyErr_Occurred())
        return NULL;

    // import the decimal module for decimal support
    module = PyImport_ImportModule("decimal");
    if (!module)
        return NULL;
    g_DecimalType = (PyTypeObject*) PyObject_GetAttrString(module, "Decimal");
    if (!g_DecimalType)
        return NULL;

    // prepare the types for use by the module
    MAKE_TYPE_READY(&g_ConnectionType);
    MAKE_TYPE_READY(&g_CursorType);
    MAKE_TYPE_READY(&g_ErrorType);
    MAKE_TYPE_READY(&g_SessionPoolType);
    MAKE_TYPE_READY(&g_ObjectTypeType);
    MAKE_TYPE_READY(&g_ObjectAttributeType);
    MAKE_TYPE_READY(&g_LOBType);
    MAKE_TYPE_READY(&g_ObjectType);
    MAKE_TYPE_READY(&g_EnqOptionsType);
    MAKE_TYPE_READY(&g_DeqOptionsType);
    MAKE_TYPE_READY(&g_MessagePropertiesType);
    MAKE_TYPE_READY(&g_SubscriptionType);
    MAKE_TYPE_READY(&g_MessageType);
    MAKE_TYPE_READY(&g_MessageTableType);
    MAKE_TYPE_READY(&g_MessageRowType);
    MAKE_TYPE_READY(&g_MessageQueryType);
    MAKE_TYPE_READY(&g_StringVarType);
    MAKE_TYPE_READY(&g_FixedCharVarType);
    MAKE_TYPE_READY(&g_RowidVarType);
    MAKE_TYPE_READY(&g_BinaryVarType);
    MAKE_TYPE_READY(&g_LongStringVarType);
    MAKE_TYPE_READY(&g_LongBinaryVarType);
    MAKE_TYPE_READY(&g_NumberVarType);
    MAKE_TYPE_READY(&g_DateTimeVarType);
    MAKE_TYPE_READY(&g_TimestampVarType);
    MAKE_TYPE_READY(&g_CLOBVarType);
    MAKE_TYPE_READY(&g_BLOBVarType);
    MAKE_TYPE_READY(&g_BFILEVarType);
    MAKE_TYPE_READY(&g_CursorVarType);
    MAKE_TYPE_READY(&g_ObjectVarType);
    MAKE_TYPE_READY(&g_NCharVarType);
    MAKE_TYPE_READY(&g_FixedNCharVarType);
    MAKE_TYPE_READY(&g_NCLOBVarType);
    MAKE_TYPE_READY(&g_NativeFloatVarType);
    MAKE_TYPE_READY(&g_NativeIntVarType);
    MAKE_TYPE_READY(&g_IntervalVarType);
    MAKE_TYPE_READY(&g_BooleanVarType);

    // initialize module and retrieve the dictionary
#if PY_MAJOR_VERSION >= 3
    module = PyModule_Create(&g_ModuleDef);
#else
    module = Py_InitModule("cx_Oracle", g_ModuleMethods);
#endif
    if (!module)
        return NULL;

    // create exception object and add it to the dictionary
    if (SetException(module, &g_WarningException,
            "Warning", CXORA_BASE_EXCEPTION) < 0)
        return NULL;
    if (SetException(module, &g_ErrorException,
            "Error", CXORA_BASE_EXCEPTION) < 0)
        return NULL;
    if (SetException(module, &g_InterfaceErrorException,
            "InterfaceError", g_ErrorException) < 0)
        return NULL;
    if (SetException(module, &g_DatabaseErrorException,
            "DatabaseError", g_ErrorException) < 0)
        return NULL;
    if (SetException(module, &g_DataErrorException,
            "DataError", g_DatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &g_OperationalErrorException,
            "OperationalError", g_DatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &g_IntegrityErrorException,
            "IntegrityError", g_DatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &g_InternalErrorException,
            "InternalError", g_DatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &g_ProgrammingErrorException,
            "ProgrammingError", g_DatabaseErrorException) < 0)
        return NULL;
    if (SetException(module, &g_NotSupportedErrorException,
            "NotSupportedError", g_DatabaseErrorException) < 0)
        return NULL;

    // initialize DPI library and create DPI context
    if (dpiContext_create(DPI_MAJOR_VERSION, DPI_MINOR_VERSION, &g_DpiContext,
            &errorInfo) < 0) {
        Error_RaiseFromInfo(&errorInfo);
        return NULL;
    }

    // set up the types that are available
    ADD_TYPE_OBJECT("Binary", &cxBinary_Type)
    ADD_TYPE_OBJECT("Connection", &g_ConnectionType)
    ADD_TYPE_OBJECT("Cursor", &g_CursorType)
    ADD_TYPE_OBJECT("Timestamp", PyDateTimeAPI->DateTimeType)
    ADD_TYPE_OBJECT("Date", PyDateTimeAPI->DateType)
    ADD_TYPE_OBJECT("SessionPool", &g_SessionPoolType)
    ADD_TYPE_OBJECT("_Error", &g_ErrorType)
    ADD_TYPE_OBJECT("Object", &g_ObjectType)
    ADD_TYPE_OBJECT("ObjectType", &g_ObjectTypeType)
    ADD_TYPE_OBJECT("EnqOptions", &g_EnqOptionsType)
    ADD_TYPE_OBJECT("DeqOptions", &g_DeqOptionsType)
    ADD_TYPE_OBJECT("MessageProperties", &g_MessagePropertiesType)

    // the name "connect" is required by the DB API
    ADD_TYPE_OBJECT("connect", &g_ConnectionType)

    // create the basic data types for setting input sizes
    ADD_TYPE_OBJECT("BINARY", &g_BinaryVarType)
    ADD_TYPE_OBJECT("BFILE", &g_BFILEVarType)
    ADD_TYPE_OBJECT("BLOB", &g_BLOBVarType)
    ADD_TYPE_OBJECT("CLOB", &g_CLOBVarType)
    ADD_TYPE_OBJECT("CURSOR", &g_CursorVarType)
    ADD_TYPE_OBJECT("OBJECT", &g_ObjectVarType)
    ADD_TYPE_OBJECT("DATETIME", &g_DateTimeVarType)
    ADD_TYPE_OBJECT("FIXED_CHAR", &g_FixedCharVarType)
    ADD_TYPE_OBJECT("FIXED_NCHAR", &g_FixedNCharVarType)
    ADD_TYPE_OBJECT("NCHAR", &g_NCharVarType)
    ADD_TYPE_OBJECT("INTERVAL", &g_IntervalVarType)
    ADD_TYPE_OBJECT("LOB", &g_LOBType)
    ADD_TYPE_OBJECT("LONG_BINARY", &g_LongBinaryVarType)
    ADD_TYPE_OBJECT("LONG_STRING", &g_LongStringVarType)
    ADD_TYPE_OBJECT("NCLOB", &g_NCLOBVarType)
    ADD_TYPE_OBJECT("NUMBER", &g_NumberVarType)
    ADD_TYPE_OBJECT("ROWID", &g_RowidVarType)
    ADD_TYPE_OBJECT("STRING", &g_StringVarType)
    ADD_TYPE_OBJECT("TIMESTAMP", &g_TimestampVarType)
    ADD_TYPE_OBJECT("NATIVE_INT", &g_NativeIntVarType)
    ADD_TYPE_OBJECT("NATIVE_FLOAT", &g_NativeFloatVarType)
    ADD_TYPE_OBJECT("BOOLEAN", &g_BooleanVarType)

    // create constants required by Python DB API 2.0
    if (PyModule_AddStringConstant(module, "apilevel", "2.0") < 0)
        return NULL;
    if (PyModule_AddIntConstant(module, "threadsafety", 2) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "paramstyle", "named") < 0)
        return NULL;

    // add version and build time for easier support
    if (PyModule_AddStringConstant(module, "version",
            BUILD_VERSION_STRING) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "__version__",
            BUILD_VERSION_STRING) < 0)
        return NULL;
    if (PyModule_AddStringConstant(module, "buildtime",
            __DATE__ " " __TIME__) < 0)
        return NULL;

    // add constants for authorization modes
    ADD_INT_CONSTANT("SYSASM", DPI_MODE_AUTH_SYSASM)
    ADD_INT_CONSTANT("SYSDBA", DPI_MODE_AUTH_SYSDBA)
    ADD_INT_CONSTANT("SYSOPER", DPI_MODE_AUTH_SYSOPER)
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


//-----------------------------------------------------------------------------
// include all DPI files
//-----------------------------------------------------------------------------

#include "dpiConn.c"
#include "dpiContext.c"
#include "dpiData.c"
#include "dpiDeqOptions.c"
#include "dpiEnqOptions.c"
#include "dpiEnv.c"
#include "dpiError.c"
#include "dpiGen.c"
#include "dpiGlobal.c"
#include "dpiLob.c"
#include "dpiMsgProps.c"
#include "dpiObject.c"
#include "dpiObjectAttr.c"
#include "dpiObjectType.c"
#include "dpiOci.c"
#include "dpiOracleType.c"
#include "dpiPool.c"
#include "dpiRowid.c"
#include "dpiStmt.c"
#include "dpiSubscr.c"
#include "dpiUtils.c"
#include "dpiVar.c"

