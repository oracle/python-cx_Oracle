//-----------------------------------------------------------------------------
// cx_Oracle.c
//   Shared library for use by Python.
//-----------------------------------------------------------------------------

#include <Python.h>
#include <datetime.h>
#include <structmember.h>
#include <time.h>
#include <oci.h>
#include <orid.h>
#include <xa.h>

// define whether or not we are building Oracle 9i
#ifdef OCI_ATTR_MODULE
#define ORACLE_10G
#endif
#ifdef OCI_MAJOR_VERSION
#define ORACLE_10GR2
#endif
#ifdef OCI_ATTR_CONNECTION_CLASS
#define ORACLE_11G
#endif

// PY_LONG_LONG was called LONG_LONG before Python 2.3
#ifndef PY_LONG_LONG
#define PY_LONG_LONG LONG_LONG
#endif

// define Py_ssize_t for versions before Python 2.5
#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

// define T_BOOL for versions before Python 2.5
#ifndef T_BOOL
#define T_BOOL                  T_INT
#endif

// define Py_TYPE for versions before Python 2.6
#ifndef Py_TYPE
#define Py_TYPE(ob)             (((PyObject*)(ob))->ob_type)
#endif

// define PyVarObject_HEAD_INIT for versions before Python 2.6
#ifndef PyVarObject_HEAD_INIT
#define PyVarObject_HEAD_INIT(type, size) \
        PyObject_HEAD_INIT(type) size,
#endif

// define PyInt_* macros for Python 3.x
#ifndef PyInt_Check
#define PyInt_Check             PyLong_Check
#define PyInt_FromLong          PyLong_FromLong
#define PyInt_AsLong            PyLong_AsLong
#define PyInt_Type              PyLong_Type
#endif

// define base exception
#if PY_MAJOR_VERSION >= 3
#define CXORA_BASE_EXCEPTION    NULL
#else
#define CXORA_BASE_EXCEPTION    PyExc_StandardError
#endif

// define simple construct for determining endianness of the platform
// Oracle uses native encoding with OCI_UTF16 but bails when a BOM is written
#define IS_LITTLE_ENDIAN (int)*(unsigned char*) &one

// define macro for adding OCI constants
#define ADD_OCI_CONSTANT(x) \
    if (PyModule_AddIntConstant(module, #x, OCI_ ##x) < 0) \
        return NULL;

// define macro for adding type objects
#define ADD_TYPE_OBJECT(name, type) \
    Py_INCREF(type); \
    if (PyModule_AddObject(module, name, (PyObject*) type) < 0) \
        return NULL;

// define macros for making types ready
#define MAKE_TYPE_READY(type) \
    if (PyType_Ready(type) < 0) \
        return NULL;
#define MAKE_VARIABLE_TYPE_READY(type) \
    (type)->tp_base = &g_BaseVarType;  \
    MAKE_TYPE_READY(type)

// define macros to get the build version as a string and the driver name
#define xstr(s)                 str(s)
#define str(s)                  #s
#define BUILD_VERSION_STRING    xstr(BUILD_VERSION)
#define DRIVER_NAME             "cx_Oracle-"BUILD_VERSION_STRING

#include "StringUtils.c"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#ifdef WITH_THREAD
static PyInterpreterState *g_InterpreterState;
#endif
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
static PyTypeObject *g_DateTimeType = NULL;
static PyTypeObject *g_DecimalType = NULL;
static PyObject *g_ShortNumberToStringFormatObj = NULL;
static udt_StringBuffer g_ShortNumberToStringFormatBuffer;
static PyObject *g_NumberToStringFormatObj = NULL;
static udt_StringBuffer g_NumberToStringFormatBuffer;


//-----------------------------------------------------------------------------
// SetException()
//   Create an exception and set it in the provided dictionary.
//-----------------------------------------------------------------------------
static int SetException(
    PyObject *module,                   // module object
    PyObject **exception,               // exception to create
    char *name,                         // name of the exception
    PyObject *baseException)            // exception to base exception on
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
static int GetModuleAndName(
    PyTypeObject *type,                 // type to get module/name for
    PyObject **module,                  // name of module
    PyObject **name)                    // name of type
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


#include "Environment.c"
#include "SessionPool.c"


//-----------------------------------------------------------------------------
// MakeDSN()
//   Make a data source name given the host port and SID.
//-----------------------------------------------------------------------------
static PyObject* MakeDSN(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    PyObject *format, *result;

    format = cxString_FromAscii("(DESCRIPTION=(ADDRESS_LIST=(ADDRESS="
            "(PROTOCOL=TCP)(HOST=%s)(PORT=%s)))(CONNECT_DATA=(SID=%s)))");
    if (!format)
        return NULL;
    result = cxString_Format(format, args);
    Py_DECREF(format);
    return result;
}


#ifdef ORACLE_10GR2
//-----------------------------------------------------------------------------
// ClientVersion()
//   Return the version of the Oracle client being used as a 5-tuple.
//-----------------------------------------------------------------------------
static PyObject* ClientVersion(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    sword majorVersion, minorVersion, updateNum, patchNum, portUpdateNum;

    OCIClientVersion(&majorVersion, &minorVersion, &updateNum,
            &patchNum, &portUpdateNum);
    return Py_BuildValue("(iiiii)", majorVersion, minorVersion, updateNum,
            patchNum, portUpdateNum);
}
#endif


//-----------------------------------------------------------------------------
// Time()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* Time(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    PyErr_SetString(g_NotSupportedErrorException,
            "Oracle does not support time only variables");
    return NULL;
}


//-----------------------------------------------------------------------------
// TimeFromTicks()
//   Returns a time value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimeFromTicks(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    PyErr_SetString(g_NotSupportedErrorException,
            "Oracle does not support time only variables");
    return NULL;
}


//-----------------------------------------------------------------------------
// DateFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* DateFromTicks(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    return PyDate_FromTimestamp(args);
}


//-----------------------------------------------------------------------------
// TimestampFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimestampFromTicks(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    return PyDateTime_FromTimestamp(args);
}


//-----------------------------------------------------------------------------
//   Declaration of methods supported by this module
//-----------------------------------------------------------------------------
static PyMethodDef g_ModuleMethods[] = {
    { "makedsn", MakeDSN, METH_VARARGS },
    { "Time", Time, METH_VARARGS },
    { "DateFromTicks", DateFromTicks, METH_VARARGS },
    { "TimeFromTicks", TimeFromTicks, METH_VARARGS },
    { "TimestampFromTicks", TimestampFromTicks, METH_VARARGS },
#ifdef ORACLE_10GR2
    { "clientversion", ClientVersion, METH_NOARGS },
#endif
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
    PyThreadState *threadState;
    PyObject *module;

    // initialize the interpreter state for callbacks
#ifdef WITH_THREAD
    PyEval_InitThreads();
    threadState = PyThreadState_Swap(NULL);
    if (!threadState)
        return NULL;
    g_InterpreterState = threadState->interp;
    PyThreadState_Swap(threadState);
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

    // set up the string and buffer for converting numbers to strings
    g_ShortNumberToStringFormatObj = cxString_FromAscii("TM9");
    if (!g_ShortNumberToStringFormatObj)
        return NULL;
    if (StringBuffer_Fill(&g_ShortNumberToStringFormatBuffer,
            g_ShortNumberToStringFormatObj) < 0)
        return NULL;
    g_NumberToStringFormatObj = cxString_FromAscii(
            "999999999999999999999999999999999999999999999999999999999999999");
    if (!g_NumberToStringFormatObj)
        return NULL;
    if (StringBuffer_Fill(&g_NumberToStringFormatBuffer,
            g_NumberToStringFormatObj) < 0)
        return NULL;

    // prepare the types for use by the module
    MAKE_TYPE_READY(&g_ConnectionType);
    MAKE_TYPE_READY(&g_CursorType);
    MAKE_TYPE_READY(&g_ErrorType);
    MAKE_TYPE_READY(&g_SessionPoolType);
    MAKE_TYPE_READY(&g_EnvironmentType);
    MAKE_TYPE_READY(&g_ObjectTypeType);
    MAKE_TYPE_READY(&g_ObjectAttributeType);
    MAKE_TYPE_READY(&g_ExternalLobVarType);
    MAKE_TYPE_READY(&g_ExternalObjectVarType);
#ifdef ORACLE_10GR2
    MAKE_TYPE_READY(&g_SubscriptionType);
    MAKE_TYPE_READY(&g_MessageType);
    MAKE_TYPE_READY(&g_MessageTableType);
    MAKE_TYPE_READY(&g_MessageRowType);
#endif
    MAKE_VARIABLE_TYPE_READY(&g_StringVarType);
    MAKE_VARIABLE_TYPE_READY(&g_FixedCharVarType);
    MAKE_VARIABLE_TYPE_READY(&g_RowidVarType);
    MAKE_VARIABLE_TYPE_READY(&g_BinaryVarType);
    MAKE_VARIABLE_TYPE_READY(&g_LongStringVarType);
    MAKE_VARIABLE_TYPE_READY(&g_LongBinaryVarType);
    MAKE_VARIABLE_TYPE_READY(&g_NumberVarType);
    MAKE_VARIABLE_TYPE_READY(&g_DateTimeVarType);
    MAKE_VARIABLE_TYPE_READY(&g_TimestampVarType);
    MAKE_VARIABLE_TYPE_READY(&g_CLOBVarType);
    MAKE_VARIABLE_TYPE_READY(&g_BLOBVarType);
    MAKE_VARIABLE_TYPE_READY(&g_BFILEVarType);
    MAKE_VARIABLE_TYPE_READY(&g_CursorVarType);
    MAKE_VARIABLE_TYPE_READY(&g_ObjectVarType);
#ifndef WITH_UNICODE
    MAKE_VARIABLE_TYPE_READY(&g_UnicodeVarType);
    MAKE_VARIABLE_TYPE_READY(&g_FixedUnicodeVarType);
#endif
    MAKE_VARIABLE_TYPE_READY(&g_NCLOBVarType);
#ifdef SQLT_BFLOAT
    MAKE_VARIABLE_TYPE_READY(&g_NativeFloatVarType);
#endif
    MAKE_VARIABLE_TYPE_READY(&g_IntervalVarType);

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

    // set up the types that are available
    ADD_TYPE_OBJECT("Binary", &cxBinary_Type)
    ADD_TYPE_OBJECT("Connection", &g_ConnectionType)
    ADD_TYPE_OBJECT("Cursor", &g_CursorType)
    ADD_TYPE_OBJECT("Timestamp", PyDateTimeAPI->DateTimeType)
    ADD_TYPE_OBJECT("Date", PyDateTimeAPI->DateType)
    ADD_TYPE_OBJECT("SessionPool", &g_SessionPoolType)
    ADD_TYPE_OBJECT("_Error", &g_ErrorType)

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
#ifndef WITH_UNICODE
    ADD_TYPE_OBJECT("FIXED_UNICODE", &g_FixedUnicodeVarType)
    ADD_TYPE_OBJECT("UNICODE", &g_UnicodeVarType)
#endif
    ADD_TYPE_OBJECT("INTERVAL", &g_IntervalVarType)
    ADD_TYPE_OBJECT("LOB", &g_ExternalLobVarType)
    ADD_TYPE_OBJECT("LONG_BINARY", &g_LongBinaryVarType)
    ADD_TYPE_OBJECT("LONG_STRING", &g_LongStringVarType)
    ADD_TYPE_OBJECT("NCLOB", &g_NCLOBVarType)
    ADD_TYPE_OBJECT("NUMBER", &g_NumberVarType)
    ADD_TYPE_OBJECT("ROWID", &g_RowidVarType)
    ADD_TYPE_OBJECT("STRING", &g_StringVarType)
    ADD_TYPE_OBJECT("TIMESTAMP", &g_TimestampVarType)
#ifdef SQLT_BFLOAT
    ADD_TYPE_OBJECT("NATIVE_FLOAT", &g_NativeFloatVarType)
#endif

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
    if (PyModule_AddStringConstant(module, "buildtime",
            __DATE__ " " __TIME__) < 0)
        return NULL;

    // add constants for registering callbacks
    ADD_OCI_CONSTANT(SYSDBA)
    ADD_OCI_CONSTANT(SYSOPER)
    ADD_OCI_CONSTANT(FNCODE_BINDBYNAME)
    ADD_OCI_CONSTANT(FNCODE_BINDBYPOS)
    ADD_OCI_CONSTANT(FNCODE_DEFINEBYPOS)
    ADD_OCI_CONSTANT(FNCODE_STMTEXECUTE)
    ADD_OCI_CONSTANT(FNCODE_STMTFETCH)
    ADD_OCI_CONSTANT(FNCODE_STMTPREPARE)
    ADD_OCI_CONSTANT(UCBTYPE_ENTRY)
    ADD_OCI_CONSTANT(UCBTYPE_EXIT)
    ADD_OCI_CONSTANT(UCBTYPE_REPLACE)
    ADD_OCI_CONSTANT(SPOOL_ATTRVAL_WAIT)
    ADD_OCI_CONSTANT(SPOOL_ATTRVAL_NOWAIT)
    ADD_OCI_CONSTANT(SPOOL_ATTRVAL_FORCEGET)
#ifdef ORACLE_10GR2
    ADD_OCI_CONSTANT(PRELIM_AUTH)
    ADD_OCI_CONSTANT(DBSHUTDOWN_ABORT)
    ADD_OCI_CONSTANT(DBSHUTDOWN_FINAL)
    ADD_OCI_CONSTANT(DBSHUTDOWN_IMMEDIATE)
    ADD_OCI_CONSTANT(DBSHUTDOWN_TRANSACTIONAL)
    ADD_OCI_CONSTANT(DBSHUTDOWN_TRANSACTIONAL_LOCAL)
    ADD_OCI_CONSTANT(EVENT_NONE)
    ADD_OCI_CONSTANT(EVENT_STARTUP)
    ADD_OCI_CONSTANT(EVENT_SHUTDOWN)
    ADD_OCI_CONSTANT(EVENT_SHUTDOWN_ANY)
    ADD_OCI_CONSTANT(EVENT_DEREG)
    ADD_OCI_CONSTANT(EVENT_OBJCHANGE)
    ADD_OCI_CONSTANT(OPCODE_ALLOPS)
    ADD_OCI_CONSTANT(OPCODE_ALLROWS)
    ADD_OCI_CONSTANT(OPCODE_INSERT)
    ADD_OCI_CONSTANT(OPCODE_UPDATE)
    ADD_OCI_CONSTANT(OPCODE_DELETE)
    ADD_OCI_CONSTANT(OPCODE_ALTER)
    ADD_OCI_CONSTANT(OPCODE_DROP)
    ADD_OCI_CONSTANT(SUBSCR_NAMESPACE_ANONYMOUS)
    ADD_OCI_CONSTANT(SUBSCR_NAMESPACE_AQ)
    ADD_OCI_CONSTANT(SUBSCR_NAMESPACE_DBCHANGE)
    ADD_OCI_CONSTANT(SUBSCR_PROTO_OCI)
    ADD_OCI_CONSTANT(SUBSCR_PROTO_MAIL)
    ADD_OCI_CONSTANT(SUBSCR_PROTO_SERVER)
    ADD_OCI_CONSTANT(SUBSCR_PROTO_HTTP)
#endif
#ifdef ORACLE_11G
    ADD_OCI_CONSTANT(ATTR_PURITY_DEFAULT)
    ADD_OCI_CONSTANT(ATTR_PURITY_NEW)
    ADD_OCI_CONSTANT(ATTR_PURITY_SELF)
#endif

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

