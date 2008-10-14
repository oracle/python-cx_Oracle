//-----------------------------------------------------------------------------
// Environment.c
//   Environment handling.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    OCIEnv *handle;
    OCIError *errorHandle;
    int maxBytesPerCharacter;
    int fixedWidth;
    ub4 maxStringBytes;
} udt_Environment;

//-----------------------------------------------------------------------------
// maximum number of characters applicable to strings
//-----------------------------------------------------------------------------
#define MAX_STRING_CHARS                4000

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
static void Environment_Free(udt_Environment*);
static int Environment_CheckForError(udt_Environment*, sword, const char*);

//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
static PyTypeObject g_EnvironmentType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "OracleEnvironment",                // tp_name
    sizeof(udt_Environment),            // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Environment_Free,      // tp_dealloc
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
    0                                   // tp_doc
};


#include "Error.c"


//-----------------------------------------------------------------------------
// Environment_New()
//   Create a new environment object.
//-----------------------------------------------------------------------------
static udt_Environment *Environment_New(
    int threaded,                       // use threaded mode?
    int events)                         // use events mode?
{
    udt_Environment *environment;
    sword status;
    ub4 mode;

    // create a new object for the Oracle environment
    environment = PyObject_NEW(udt_Environment, &g_EnvironmentType);
    if (!environment)
        return NULL;
    environment->handle = NULL;
    environment->errorHandle = NULL;
#ifdef WITH_UNICODE
    environment->maxBytesPerCharacter = 2;
#else
    environment->maxBytesPerCharacter = 1;
#endif
    environment->fixedWidth = 1;
    environment->maxStringBytes = MAX_STRING_CHARS;

    // turn threading mode on, if desired
    mode = OCI_OBJECT;
    if (threaded)
        mode |= OCI_THREADED;
#ifdef ORACLE_11G
    if (events)
        mode |= OCI_EVENTS;
#endif

    // create the environment handle
    status = OCIEnvNlsCreate(&environment->handle, mode, NULL, NULL, NULL,
            NULL, 0, NULL, CXORA_CHARSETID, CXORA_CHARSETID);
    if (!environment->handle) {
        Py_DECREF(environment);
        PyErr_SetString(g_InterfaceErrorException,
                "Unable to acquire Oracle environment handle");
        return NULL;
    }
    if (Environment_CheckForError(environment, status,
            "Environment_New(): create env handle") < 0) {
        environment->handle = NULL;
        Py_DECREF(environment);
        return NULL;
    }

    // create the error handle
    status = OCIHandleAlloc(environment->handle,
            (dvoid**) &environment->errorHandle, OCI_HTYPE_ERROR, 0, 0);
    if (Environment_CheckForError(environment, status,
            "Environment_New(): create error handle") < 0) {
        Py_DECREF(environment);
        return NULL;
    }

    // acquire max bytes per character
#ifndef WITH_UNICODE
    status = OCINlsNumericInfoGet(environment->handle,
            environment->errorHandle, &environment->maxBytesPerCharacter,
            OCI_NLS_CHARSET_MAXBYTESZ);
    if (Environment_CheckForError(environment, status,
            "Environment_New(): get max bytes per character") < 0) {
        Py_DECREF(environment);
        return NULL;
    }
    environment->maxStringBytes =
            MAX_STRING_CHARS * environment->maxBytesPerCharacter;

    // acquire whether character set is fixed width
    status = OCINlsNumericInfoGet(environment->handle,
            environment->errorHandle, &environment->fixedWidth,
            OCI_NLS_CHARSET_FIXEDWIDTH);
    if (Environment_CheckForError(environment, status,
            "Environment_New(): determine if charset is fixed width") < 0) {
        Py_DECREF(environment);
        return NULL;
    }
#endif

    return environment;
}


//-----------------------------------------------------------------------------
// Environment_Free()
//   Deallocate the environment. Note that destroying the environment handle
// will automatically destroy any child handles that were created.
//-----------------------------------------------------------------------------
static void Environment_Free(
    udt_Environment *environment)       // environment object
{
    if (environment->handle)
        OCIHandleFree(environment->handle, OCI_HTYPE_ENV);
    PyObject_DEL(environment);
}


//-----------------------------------------------------------------------------
// Environment_RaiseError()
//   Reads the error that was caused by the last Oracle statement and raise an
// exception for Python. At this point it is assumed that the Oracle
// environment is fully initialized.
//-----------------------------------------------------------------------------
static void Environment_RaiseError(
    udt_Environment *environment,       // environment to raise error for
    const char *context)                // context in which error occurred
{
    PyObject *exceptionType;
    udt_Error *error;

    error = Error_New(environment, context, 1);
    if (error) {
        switch (error->code) {
            case 1:
            case 1400:
            case 2290:
            case 2291:
            case 2292:
                exceptionType = g_IntegrityErrorException;
                break;
            case 1012:
            case 1033:
            case 1034:
            case 1089:
            case 3113:
            case 3114:
            case 12203:
            case 12500:
            case 12571:
                exceptionType = g_OperationalErrorException;
                break;
            default:
                exceptionType = g_DatabaseErrorException;
                break;
        }
        PyErr_SetObject(exceptionType, (PyObject*) error);
        Py_DECREF(error);
    }
}


//-----------------------------------------------------------------------------
// Environment_CheckForError()
//   Check for an error in the last call and if an error has occurred, raise a
// Python exception.
//-----------------------------------------------------------------------------
static int Environment_CheckForError(
    udt_Environment *environment,       // environment to raise error in
    sword status,                       // status of last call
    const char *context)                // context
{
    if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO) {
        if (status == OCI_INVALID_HANDLE)
            PyErr_SetString(g_DatabaseErrorException, "Invalid handle!");
        else Environment_RaiseError(environment, context);
        return -1;
    }
    return 0;
}

