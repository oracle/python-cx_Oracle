//-----------------------------------------------------------------------------
// cx_Oracle.c
//   Shared library for use by Python.
//-----------------------------------------------------------------------------

#include <Python.h>
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

// define whether or not we are building with the native datetime module
#if (PY_VERSION_HEX >= 0x02040000)
#include <datetime.h>
#define NATIVE_DATETIME
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

// define Py_TYPE for versions before Python 2.6
#ifndef Py_TYPE
#define Py_TYPE(ob)             (((PyObject*)(ob))->ob_type)
#endif

// define PyVarObject_HEAD_INIT for versions before Python 2.6
#ifndef PyVarObject_HEAD_INIT
#define PyVarObject_HEAD_INIT(type, size) \
        PyObject_HEAD_INIT(type) size,
#endif

// define simple construct for determining endianness of the platform
// Oracle uses native encoding with OCI_UTF16 but bails when a BOM is written
#define IS_LITTLE_ENDIAN (int)*(unsigned char*) &one

// define macro for adding OCI constants
#define ADD_OCI_CONSTANT(x) \
    if (PyModule_AddIntConstant(module, #x, OCI_ ##x) < 0) \
        return;

// define macro for adding type objects
#define ADD_TYPE_OBJECT(name, type) \
    Py_INCREF(type); \
    if (PyModule_AddObject(module, name, (PyObject*) type) < 0) \
        return;

// define macros for making types ready
#define MAKE_TYPE_READY(type) \
    if (PyType_Ready(type) < 0) \
        return;
#define MAKE_VARIABLE_TYPE_READY(type) \
    (type)->tp_base = &g_BaseVarType;  \
    MAKE_TYPE_READY(type)

// define macros to get the build version as a string and the driver name
#define xstr(s)                 str(s)
#define str(s)                  #s
#define BUILD_VERSION_STRING    xstr(BUILD_VERSION)
#define DRIVER_NAME             "cx_Oracle-"BUILD_VERSION_STRING


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


#include "StringUtils.c"
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
    char *host, *sid;
    int port;

    // parse arguments
    if (!PyArg_ParseTuple(args, "sis", &host, &port, &sid))
        return NULL;

    // return the formatted string
    return PyString_FromFormat("(DESCRIPTION=(ADDRESS_LIST=(ADDRESS="
            "(PROTOCOL=TCP)(HOST=%s)(PORT=%d)))(CONNECT_DATA=(SID=%s)))",
            host, port, sid);
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


#ifndef NATIVE_DATETIME
//-----------------------------------------------------------------------------
// Date()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* Date(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
    return ExternalDateTimeVar_New(&g_ExternalDateTimeVarType, args, NULL);
}
#endif


//-----------------------------------------------------------------------------
// DateFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* DateFromTicks(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
#ifdef NATIVE_DATETIME
    return PyDate_FromTimestamp(args);
#else
    struct tm *time;
    time_t ticks;
    int ticksArg;

    if (!PyArg_ParseTuple(args, "i", &ticksArg))
        return NULL;

    ticks = ticksArg;
    time = localtime(&ticks);
    if (time == NULL)
        return PyErr_SetFromErrno(g_DataErrorException);
    return ExternalDateTimeVar_NewFromC(&g_ExternalDateTimeVarType,
            time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, 0, 0, 0, 0);
#endif
}


//-----------------------------------------------------------------------------
// TimestampFromTicks()
//   Returns a date value suitable for binding.
//-----------------------------------------------------------------------------
static PyObject* TimestampFromTicks(
    PyObject* self,                     // passthrough argument
    PyObject* args)                     // arguments to function
{
#ifdef NATIVE_DATETIME
    return PyDateTime_FromTimestamp(args);
#else
    struct tm *time;
    time_t ticks;
    int ticksArg;

    if (!PyArg_ParseTuple(args, "i", &ticksArg))
        return NULL;

    ticks = ticksArg;
    time = localtime(&ticks);
    if (time == NULL)
        return PyErr_SetFromErrno(g_DataErrorException);
    return ExternalDateTimeVar_NewFromC(&g_ExternalDateTimeVarType,
            time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
            time->tm_hour, time->tm_min, time->tm_sec, 0);
#endif
}


//-----------------------------------------------------------------------------
//   Declaration of methods supported by this module
//-----------------------------------------------------------------------------
static PyMethodDef g_ModuleMethods[] = {
    { "makedsn", MakeDSN, METH_VARARGS },
#ifndef NATIVE_DATETIME
    { "Date", Date, METH_VARARGS },
#endif
    { "Time", Time, METH_VARARGS },
    { "DateFromTicks", DateFromTicks, METH_VARARGS },
    { "TimeFromTicks", TimeFromTicks, METH_VARARGS },
    { "TimestampFromTicks", TimestampFromTicks, METH_VARARGS },
#ifdef ORACLE_10GR2
    { "clientversion", ClientVersion, METH_NOARGS },
#endif
    { NULL }
};

//-----------------------------------------------------------------------------
// initcx_Oracle()
//   Initialization routine for the shared libary.
//-----------------------------------------------------------------------------
void initcx_Oracle(void)
{
    PyThreadState *threadState;
    PyObject *module;

    // initialize the interpreter state for callbacks
#ifdef WITH_THREAD
    PyEval_InitThreads();
    threadState = PyThreadState_Swap(NULL);
    if (!threadState)
        return;
    g_InterpreterState = threadState->interp;
    PyThreadState_Swap(threadState);
#endif

    // manage the use of the datetime module; in Python 2.4 and up, the native
    // datetime type is used since a C API is exposed; in all other cases, the
    // internal datetime type is used and if the datetime module exists,
    // binding to that type is permitted
#ifdef NATIVE_DATETIME
    PyDateTime_IMPORT;
    if (PyErr_Occurred())
        return;
#else
    module = PyImport_ImportModule("datetime");
    if (module)
        g_DateTimeType = (PyTypeObject*) PyObject_GetAttrString(module,
                "datetime");
    PyErr_Clear();
#endif

    // attempt to import the decimal module
    module = PyImport_ImportModule("decimal");
    if (module)
        g_DecimalType = (PyTypeObject*) PyObject_GetAttrString(module,
                "Decimal");
    PyErr_Clear();

    // prepare the types for use by the module
    MAKE_TYPE_READY(&g_ConnectionType);
    MAKE_TYPE_READY(&g_CursorType);
    MAKE_TYPE_READY(&g_ErrorType);
#ifndef NATIVE_DATETIME
    MAKE_TYPE_READY(&g_ExternalDateTimeVarType); // not an udt_Variable type
#endif
    MAKE_TYPE_READY(&g_SessionPoolType);
    MAKE_TYPE_READY(&g_EnvironmentType);
    MAKE_TYPE_READY(&g_ObjectTypeType);
    MAKE_TYPE_READY(&g_ObjectAttributeType);
    MAKE_TYPE_READY(&g_ExternalLobVarType);
    MAKE_TYPE_READY(&g_ExternalObjectVarType);
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
    MAKE_VARIABLE_TYPE_READY(&g_NCLOBVarType);
    MAKE_VARIABLE_TYPE_READY(&g_BLOBVarType);
    MAKE_VARIABLE_TYPE_READY(&g_BFILEVarType);
    MAKE_VARIABLE_TYPE_READY(&g_CursorVarType);
    MAKE_VARIABLE_TYPE_READY(&g_ObjectVarType);
    MAKE_VARIABLE_TYPE_READY(&g_UnicodeVarType);
    MAKE_VARIABLE_TYPE_READY(&g_FixedUnicodeVarType);
#ifdef SQLT_BFLOAT
    MAKE_VARIABLE_TYPE_READY(&g_NativeFloatVarType);
#endif

    // initialize module and retrieve the dictionary
    module = Py_InitModule("cx_Oracle", g_ModuleMethods);
    if (!module)
        return;

    // create exception object and add it to the dictionary
    if (SetException(module, &g_WarningException,
            "Warning", PyExc_StandardError) < 0)
        return;
    if (SetException(module, &g_ErrorException,
            "Error", PyExc_StandardError) < 0)
        return;
    if (SetException(module, &g_InterfaceErrorException,
            "InterfaceError", g_ErrorException) < 0)
        return;
    if (SetException(module, &g_DatabaseErrorException,
            "DatabaseError", g_ErrorException) < 0)
        return;
    if (SetException(module, &g_DataErrorException,
            "DataError", g_DatabaseErrorException) < 0)
        return;
    if (SetException(module, &g_OperationalErrorException,
            "OperationalError", g_DatabaseErrorException) < 0)
        return;
    if (SetException(module, &g_IntegrityErrorException,
            "IntegrityError", g_DatabaseErrorException) < 0)
        return;
    if (SetException(module, &g_InternalErrorException,
            "InternalError", g_DatabaseErrorException) < 0)
        return;
    if (SetException(module, &g_ProgrammingErrorException,
            "ProgrammingError", g_DatabaseErrorException) < 0)
        return;
    if (SetException(module, &g_NotSupportedErrorException,
            "NotSupportedError", g_DatabaseErrorException) < 0)
        return;

    // set up the types that are available
    ADD_TYPE_OBJECT("Binary", &PyBuffer_Type)
    ADD_TYPE_OBJECT("Connection", &g_ConnectionType)
    ADD_TYPE_OBJECT("Cursor", &g_CursorType)
#ifdef NATIVE_DATETIME
    ADD_TYPE_OBJECT("Timestamp", PyDateTimeAPI->DateTimeType)
    ADD_TYPE_OBJECT("Date", PyDateTimeAPI->DateType)
#else
    ADD_TYPE_OBJECT("Timestamp", &g_ExternalDateTimeVarType)
#endif
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
    ADD_TYPE_OBJECT("FIXED_UNICODE", &g_FixedUnicodeVarType)
    ADD_TYPE_OBJECT("LOB", &g_ExternalLobVarType)
    ADD_TYPE_OBJECT("LONG_BINARY", &g_LongBinaryVarType)
    ADD_TYPE_OBJECT("LONG_STRING", &g_LongStringVarType)
    ADD_TYPE_OBJECT("NCLOB", &g_NCLOBVarType)
    ADD_TYPE_OBJECT("NUMBER", &g_NumberVarType)
    ADD_TYPE_OBJECT("ROWID", &g_RowidVarType)
    ADD_TYPE_OBJECT("STRING", &g_StringVarType)
    ADD_TYPE_OBJECT("TIMESTAMP", &g_TimestampVarType)
    ADD_TYPE_OBJECT("UNICODE", &g_UnicodeVarType)
#ifdef SQLT_BFLOAT
    ADD_TYPE_OBJECT("NATIVE_FLOAT", &g_NativeFloatVarType)
#endif

    // create constants required by Python DB API 2.0
    if (PyModule_AddStringConstant(module, "apilevel", "2.0") < 0)
        return;
    if (PyModule_AddIntConstant(module, "threadsafety", 2) < 0)
        return;
    if (PyModule_AddStringConstant(module, "paramstyle", "named") < 0)
        return;

    // add version and build time for easier support
    if (PyModule_AddStringConstant(module, "version",
            BUILD_VERSION_STRING) < 0)
        return;
    if (PyModule_AddStringConstant(module, "buildtime",
            __DATE__ " " __TIME__) < 0)
        return;

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
#endif
#ifdef ORACLE_11G
    ADD_OCI_CONSTANT(ATTR_PURITY_DEFAULT)
    ADD_OCI_CONSTANT(ATTR_PURITY_NEW)
    ADD_OCI_CONSTANT(ATTR_PURITY_SELF)
#endif
}

