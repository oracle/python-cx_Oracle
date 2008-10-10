//-----------------------------------------------------------------------------
// Error.c
//   Error handling.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    sb4 code;
    PyObject *message;
    const char *context;
} udt_Error;


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
static void Error_Free(udt_Error*);
static PyObject *Error_Str(udt_Error*);


//-----------------------------------------------------------------------------
// declaration of members
//-----------------------------------------------------------------------------
static PyMemberDef g_ErrorMembers[] = {
    { "code", T_INT, offsetof(udt_Error, code), READONLY },
    { "message", T_OBJECT, offsetof(udt_Error, message), READONLY },
    { "context", T_STRING, offsetof(udt_Error, context), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
static PyTypeObject g_ErrorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle._Error",                 // tp_name
    sizeof(udt_Error),                  // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Error_Free,            // tp_dealloc
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
    (reprfunc) Error_Str,               // tp_str
    0,                                  // tp_getattro
    0,                                  // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    g_ErrorMembers,                     // tp_members
    0                                   // tp_getset
};


//-----------------------------------------------------------------------------
// Error_New()
//   Create a new error object.
//-----------------------------------------------------------------------------
static udt_Error *Error_New(
    udt_Environment *environment,       // environment object
    const char *context,                // context in which error occurred
    int retrieveError)                  // retrieve error from OCI?
{
    char errorText[CXORA_ERROR_TEXT_LENGTH];
    udt_Error *error;
    ub4 handleType;
    dvoid *handle;
    sword status;

    error = PyObject_NEW(udt_Error, &g_ErrorType);
    if (!error)
        return NULL;
    error->context = context;
    if (retrieveError) {
        if (environment->errorHandle) {
            handle = environment->errorHandle;
            handleType = OCI_HTYPE_ERROR;
        } else {
            handle = environment->handle;
            handleType = OCI_HTYPE_ENV;
        }
        status = OCIErrorGet(handle, 1, 0, &error->code,
                (unsigned char*) errorText, sizeof(errorText), handleType);
        if (status != OCI_SUCCESS) {
            Py_DECREF(error);
            PyErr_SetString(g_InternalErrorException, "No Oracle error?");
            return NULL;
        }
#ifdef WITH_UNICODE
        Py_ssize_t len;
	    for (len = 0; len < sizeof(errorText); len += 2) {
	        if (errorText[len] == 0 && errorText[len + 1] == 0)
		        break;
	    }
        error->message = CXORA_TO_STRING_OBJ(errorText, len / 2);
#else
        error->message = PyString_FromString(errorText);
#endif
        if (!error->message) {
            Py_DECREF(error);
            return NULL;
        }
    }

    return error;
}


//-----------------------------------------------------------------------------
// Error_Free()
//   Deallocate the environment, disconnecting from the database if necessary.
//-----------------------------------------------------------------------------
static void Error_Free(
    udt_Error *self)                    // error object
{
    Py_XDECREF(self->message);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Error_Str()
//   Return a string representation of the error variable.
//-----------------------------------------------------------------------------
static PyObject *Error_Str(
    udt_Error *self)                    // variable to return the string for
{
    if (self->message) {
        Py_INCREF(self->message);
        return self->message;
    }
#ifdef WITH_UNICODE
    return PyUnicode_DecodeASCII("", 0, NULL);
#else
    return PyString_FromString("");
#endif
}

