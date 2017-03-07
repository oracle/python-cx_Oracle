//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EnqOptions.c
//   Implements the enqueue options objects used in Advanced Queuing.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure used for implementing enqueue options
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiEnqOptions *handle;
    const char *encoding;
} udt_EnqOptions;


//-----------------------------------------------------------------------------
// Declaration of methods used for enqueue options
//-----------------------------------------------------------------------------
static udt_EnqOptions *EnqOptions_New(udt_Connection*);
static void EnqOptions_Free(udt_EnqOptions*);
static PyObject *EnqOptions_GetTransformation(udt_EnqOptions*, void*);
static PyObject *EnqOptions_GetVisibility(udt_EnqOptions*, void*);
static int EnqOptions_SetDeliveryMode(udt_EnqOptions*, PyObject*, void*);
static int EnqOptions_SetTransformation(udt_EnqOptions*, PyObject*, void*);
static int EnqOptions_SetVisibility(udt_EnqOptions*, PyObject*, void*);


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "EnqOptions"
//-----------------------------------------------------------------------------
static PyGetSetDef g_EnqOptionsCalcMembers[] = {
    { "deliverymode", 0, (setter) EnqOptions_SetDeliveryMode, 0, 0 },
    { "transformation", (getter) EnqOptions_GetTransformation,
            (setter) EnqOptions_SetTransformation, 0, 0 },
    { "visibility", (getter) EnqOptions_GetVisibility,
            (setter) EnqOptions_SetVisibility, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_EnqOptionsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.EnqOptions",             // tp_name
    sizeof(udt_EnqOptions),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) EnqOptions_Free,       // tp_dealloc
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
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    0,                                  // tp_methods
    0,                                  // tp_members
    g_EnqOptionsCalcMembers,            // tp_getset
    0,                                  // tp_base
    0,                                  // tp_dict
    0,                                  // tp_descr_get
    0,                                  // tp_descr_set
    0,                                  // tp_dictoffset
    0,                                  // tp_init
    0,                                  // tp_alloc
    0,                                  // tp_new
    0,                                  // tp_free
    0,                                  // tp_is_gc
    0                                   // tp_bases
};


//-----------------------------------------------------------------------------
// EnqOptions_New()
//   Create a new enqueue options object.
//-----------------------------------------------------------------------------
static udt_EnqOptions *EnqOptions_New(udt_Connection *connection)
{
    udt_EnqOptions *self;

    self = (udt_EnqOptions*) g_EnqOptionsType.tp_alloc(&g_EnqOptionsType, 0);
    if (!self)
        return NULL;
    if (dpiConn_newEnqOptions(connection->handle, &self->handle) < 0) {
        Py_DECREF(self);
        Error_RaiseAndReturnNull();
        return NULL;
    }
    self->encoding = connection->encodingInfo.encoding;

    return self;
}


//-----------------------------------------------------------------------------
// EnqOptions_Free()
//   Free the memory associated with the enqueue options object.
//-----------------------------------------------------------------------------
static void EnqOptions_Free(udt_EnqOptions *self)
{
    if (self->handle) {
        dpiEnqOptions_release(self->handle);
        self->handle = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// EnqOptions_GetTransformation()
//   Get the value of the transformation option.
//-----------------------------------------------------------------------------
static PyObject *EnqOptions_GetTransformation(udt_EnqOptions *self,
        void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiEnqOptions_getTransformation(self->handle, &value,
            &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return cxString_FromEncodedString(value, valueLength, self->encoding);
}


//-----------------------------------------------------------------------------
// EnqOptions_GetVisibility()
//   Get the value of the visibility option.
//-----------------------------------------------------------------------------
static PyObject *EnqOptions_GetVisibility(udt_EnqOptions *self, void *unused)
{
    dpiVisibility value;

    if (dpiEnqOptions_getVisibility(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// EnqOptions_SetDeliveryMode()
//   Set the value of the delivery mode option.
//-----------------------------------------------------------------------------
static int EnqOptions_SetDeliveryMode(udt_EnqOptions *self, PyObject *valueObj,
        void *unused)
{
    dpiMessageDeliveryMode value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiEnqOptions_setDeliveryMode(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// EnqOptions_SetTransformation()
//   Set the value of the transformation option.
//-----------------------------------------------------------------------------
static int EnqOptions_SetTransformation(udt_EnqOptions *self,
        PyObject *valueObj, void *unused)
{
    udt_Buffer buffer;
    int status;

    if (cxBuffer_FromObject(&buffer, valueObj, self->encoding) < 0)
        return -1;
    status = dpiEnqOptions_setTransformation(self->handle, buffer.ptr,
            buffer.size);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// EnqOptions_SetVisibility()
//   Set the value of the visibility option.
//-----------------------------------------------------------------------------
static int EnqOptions_SetVisibility(udt_EnqOptions *self,
        PyObject *valueObj, void *unused)
{
    dpiVisibility value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiEnqOptions_setVisibility(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}

