//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Error.c
//   Error handling.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure for the Python type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    long code;
    unsigned offset;
    PyObject *message;
    PyObject *context;
    char isRecoverable;
} udt_Error;


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
static void Error_Free(udt_Error*);
static PyObject *Error_Str(udt_Error*);
static PyObject *Error_New(PyTypeObject*, PyObject*, PyObject*);
static PyObject *Error_Reduce(udt_Error*);


//-----------------------------------------------------------------------------
// declaration of methods
//-----------------------------------------------------------------------------
static PyMethodDef g_ErrorMethods[] = {
    { "__reduce__", (PyCFunction) Error_Reduce, METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// declaration of members
//-----------------------------------------------------------------------------
static PyMemberDef g_ErrorMembers[] = {
    { "code", T_LONG, offsetof(udt_Error, code), READONLY },
    { "offset", T_UINT, offsetof(udt_Error, offset), READONLY },
    { "message", T_OBJECT, offsetof(udt_Error, message), READONLY },
    { "context", T_OBJECT, offsetof(udt_Error, context), READONLY },
    { "isrecoverable", T_BOOL, offsetof(udt_Error, isRecoverable), READONLY },
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
    g_ErrorMethods,                     // tp_methods
    g_ErrorMembers,                     // tp_members
    0,                                  // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    Error_New,                          // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// Error_Free()
//   Deallocate the error.
//-----------------------------------------------------------------------------
static void Error_Free(udt_Error *self)
{
    Py_CLEAR(self->message);
    Py_CLEAR(self->context);
    PyObject_Del(self);
}


//-----------------------------------------------------------------------------
// Error_New()
//   Create a new error object. This is intended to only be used by the
// unpickling routine, and not by direct creation!
//-----------------------------------------------------------------------------
static PyObject *Error_New(PyTypeObject *type, PyObject *args,
        PyObject *keywordArgs)
{
    PyObject *message, *context;
    int isRecoverable, code;
    udt_Error *self;
    unsigned offset;

    isRecoverable = 0;
    if (!PyArg_ParseTuple(args, "OiIO|i", &message, &code, &offset, &context,
            &isRecoverable))
        return NULL;
    self = (udt_Error*) type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    self->code = code;
    self->offset = offset;
    self->isRecoverable = (char) isRecoverable;
    Py_INCREF(message);
    self->message = message;
    Py_INCREF(context);
    self->context = context;

    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Error_Str()
//   Return a string representation of the error variable.
//-----------------------------------------------------------------------------
static PyObject *Error_Str(udt_Error *self)
{
    Py_INCREF(self->message);
    return self->message;
}


//-----------------------------------------------------------------------------
// Error_InternalNew()
//   Internal method for creating an error object from the DPI error
// information.
//-----------------------------------------------------------------------------
static udt_Error *Error_InternalNew(dpiErrorInfo *errorInfo)
{
    PyObject *format, *args, *fnName, *action;
    udt_Error *self;

    // create error object and initialize it
    self = (udt_Error*) g_ErrorType.tp_alloc(&g_ErrorType, 0);
    if (!self)
        return NULL;
    self->code = errorInfo->code;
    self->offset = errorInfo->offset;
    self->isRecoverable = (char) errorInfo->isRecoverable;

    // create message
    self->message = cxString_FromEncodedString(errorInfo->message,
            errorInfo->messageLength, errorInfo->encoding);
    if (!self->message) {
        Py_DECREF(self);
        return NULL;
    }

    // create context composed of function name and action
    fnName = cxString_FromAscii(errorInfo->fnName);
    if (!fnName) {
        Py_DECREF(self);
        return NULL;
    }
    action = cxString_FromAscii(errorInfo->action);
    if (!action) {
        Py_DECREF(fnName);
        Py_DECREF(self);
        return NULL;
    }
    args = PyTuple_Pack(2, fnName, action);
    Py_DECREF(fnName);
    Py_DECREF(action);
    if (!args) {
        Py_DECREF(self);
        return NULL;
    }
    format = cxString_FromAscii("%s: %s");
    if (!format) {
        Py_DECREF(self);
        Py_DECREF(args);
        return NULL;
    }
    self->context = cxString_Format(format, args);
    Py_DECREF(format);
    Py_DECREF(args);
    if (!self->context) {
        Py_DECREF(self);
        return NULL;
    }

    return self;
}


//-----------------------------------------------------------------------------
// Error_RaiseFromInfo()
//   Internal method for raising an exception given an error information
// structure from DPI. Return -1 as a convenience to the caller.
//-----------------------------------------------------------------------------
static int Error_RaiseFromInfo(dpiErrorInfo *errorInfo)
{
    PyObject *exceptionType;
    udt_Error *self;

    self = Error_InternalNew(errorInfo);
    if (!self)
        return -1;
    switch (errorInfo->code) {
        case 1:
        case 1400:
        case 2290:
        case 2291:
        case 2292:
            exceptionType = g_IntegrityErrorException;
            break;
        case 22:
        case 378:
        case 602:
        case 603:
        case 604:
        case 609:
        case 1012:
        case 1013:
        case 1033:
        case 1034:
        case 1041:
        case 1043:
        case 1089:
        case 1090:
        case 1092:
        case 3113:
        case 3114:
        case 3122:
        case 3135:
        case 12153:
        case 12203:
        case 12500:
        case 12571:
        case 27146:
        case 28511:
            exceptionType = g_OperationalErrorException;
            break;
        default:
            exceptionType = g_DatabaseErrorException;
            break;
    }
    PyErr_SetObject(exceptionType, (PyObject*) self);
    Py_DECREF(self);
    return -1;
}


//-----------------------------------------------------------------------------
// Error_RaiseAndReturnInt()
//   Internal method for raising an exception from an error generated from DPI.
// Return -1 as a convenience to the caller.
//-----------------------------------------------------------------------------
static int Error_RaiseAndReturnInt(void)
{
    dpiErrorInfo errorInfo;

    dpiContext_getError(g_DpiContext, &errorInfo);
    return Error_RaiseFromInfo(&errorInfo);
}


//-----------------------------------------------------------------------------
// Error_RaiseAndReturnNull()
//   Internal method for raising an exception from an error generated from DPI.
// Return NULL as a convenience to the caller.
//-----------------------------------------------------------------------------
static PyObject *Error_RaiseAndReturnNull(void)
{
    Error_RaiseAndReturnInt();
    return NULL;
}


//-----------------------------------------------------------------------------
// Error_Reduce()
//   Method provided for pickling/unpickling of Error objects.
//-----------------------------------------------------------------------------
static PyObject *Error_Reduce(udt_Error *self)
{
    return Py_BuildValue("(O(OiIO))", Py_TYPE(self), self->message,
            self->code, self->offset, self->context);
}

