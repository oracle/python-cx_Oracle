//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DeqOptions.c
//   Implements the dequeue options objects used in Advanced Queuing.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structure used for implementing dequeue options
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiDeqOptions *handle;
    const char *encoding;
} udt_DeqOptions;


//-----------------------------------------------------------------------------
// Declaration of methods used for dequeue options
//-----------------------------------------------------------------------------
static udt_DeqOptions *DeqOptions_New(udt_Connection*);
static void DeqOptions_Free(udt_DeqOptions*);
static PyObject *DeqOptions_GetCondition(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetConsumerName(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetCorrelation(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetMode(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetMsgId(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetNavigation(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetTransformation(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetVisibility(udt_DeqOptions*, void*);
static PyObject *DeqOptions_GetWait(udt_DeqOptions*, void*);
static int DeqOptions_SetCondition(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetConsumerName(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetCorrelation(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetDeliveryMode(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetMode(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetMsgId(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetNavigation(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetTransformation(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetVisibility(udt_DeqOptions*, PyObject*, void*);
static int DeqOptions_SetWait(udt_DeqOptions*, PyObject*, void*);


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "DeqOptions"
//-----------------------------------------------------------------------------
static PyGetSetDef g_DeqOptionsCalcMembers[] = {
    { "condition", (getter) DeqOptions_GetCondition,
            (setter) DeqOptions_SetCondition, 0, 0 },
    { "consumername", (getter) DeqOptions_GetConsumerName,
            (setter) DeqOptions_SetConsumerName, 0, 0 },
    { "correlation", (getter) DeqOptions_GetCorrelation,
            (setter) DeqOptions_SetCorrelation, 0, 0 },
    { "deliverymode", 0, (setter) DeqOptions_SetDeliveryMode, 0, 0 },
    { "mode", (getter) DeqOptions_GetMode, (setter) DeqOptions_SetMode, 0, 0 },
    { "msgid", (getter) DeqOptions_GetMsgId,
            (setter) DeqOptions_SetMsgId, 0, 0 },
    { "navigation", (getter) DeqOptions_GetNavigation,
            (setter) DeqOptions_SetNavigation, 0, 0 },
    { "transformation", (getter) DeqOptions_GetTransformation,
            (setter) DeqOptions_SetTransformation, 0, 0 },
    { "visibility", (getter) DeqOptions_GetVisibility,
            (setter) DeqOptions_SetVisibility, 0, 0 },
    { "wait", (getter) DeqOptions_GetWait, (setter) DeqOptions_SetWait, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_DeqOptionsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.DeqOptions",             // tp_name
    sizeof(udt_DeqOptions),             // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) DeqOptions_Free,       // tp_dealloc
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
    g_DeqOptionsCalcMembers,            // tp_getset
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
// DeqOptions_New()
//   Create a new dequeue options object.
//-----------------------------------------------------------------------------
static udt_DeqOptions *DeqOptions_New(udt_Connection *connection)
{
    udt_DeqOptions *self;

    self = (udt_DeqOptions*) g_DeqOptionsType.tp_alloc(&g_DeqOptionsType, 0);
    if (!self)
        return NULL;
    if (dpiConn_newDeqOptions(connection->handle, &self->handle) < 0) {
        Py_DECREF(self);
        Error_RaiseAndReturnNull();
        return NULL;
    }
    self->encoding = connection->encodingInfo.encoding;

    return self;
}


//-----------------------------------------------------------------------------
// DeqOptions_Free()
//   Free the memory associated with the dequeue options object.
//-----------------------------------------------------------------------------
static void DeqOptions_Free(
    udt_DeqOptions *self)               // object to free
{
    if (self->handle) {
        dpiDeqOptions_release(self->handle);
        self->handle = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetAttrText()
//   Get the value of the attribute as text.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetAttrText(udt_DeqOptions *self,
        int (*func)(dpiDeqOptions*, const char**, uint32_t*))
{
    uint32_t valueLength;
    const char *value;

    if ((*func)(self->handle, &value, &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return cxString_FromEncodedString(value, valueLength, self->encoding);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetAttrText()
//   Set the value of the attribute as text.
//-----------------------------------------------------------------------------
static int DeqOptions_SetAttrText(udt_DeqOptions *self, PyObject *value,
        int (*func)(dpiDeqOptions*, const char*, uint32_t))
{
    udt_Buffer buffer;
    int status;

    if (cxBuffer_FromObject(&buffer, value, self->encoding))
        return -1;
    status = (*func)(self->handle, buffer.ptr, buffer.size);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_GetCondition()
//   Get the value of the condition option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetCondition(udt_DeqOptions *self, void *unused)
{
    return DeqOptions_GetAttrText(self, dpiDeqOptions_getCondition);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetConsumerName()
//   Get the value of the consumer name option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetConsumerName(udt_DeqOptions *self, void *unused)
{
    return DeqOptions_GetAttrText(self, dpiDeqOptions_getConsumerName);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetCorrelation()
//   Get the value of the correlation option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetCorrelation(udt_DeqOptions *self, void *unused)
{
    return DeqOptions_GetAttrText(self, dpiDeqOptions_getCorrelation);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetMode()
//   Get the value of the mode option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetMode(udt_DeqOptions *self, void *unused)
{
    dpiDeqMode value;

    if (dpiDeqOptions_getMode(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetMsgId()
//   Get the value of the message id option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetMsgId(udt_DeqOptions *self, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiDeqOptions_getMsgId(self->handle, &value, &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyBytes_FromStringAndSize(value, valueLength);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetNavigation()
//   Get the value of the navigation option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetNavigation(udt_DeqOptions *self, void *unused)
{
    dpiDeqNavigation value;

    if (dpiDeqOptions_getNavigation(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetTransformation()
//   Get the value of the transformation option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetTransformation(udt_DeqOptions *self,
        void *unused)
{
    return DeqOptions_GetAttrText(self, dpiDeqOptions_getTransformation);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetVisibility()
//   Get the value of the visibility option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetVisibility(udt_DeqOptions *self, void *unused)
{
    dpiVisibility value;

    if (dpiDeqOptions_getVisibility(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// DeqOptions_GetWait()
//   Get the value of the wait option.
//-----------------------------------------------------------------------------
static PyObject *DeqOptions_GetWait(udt_DeqOptions *self, void *unused)
{
    uint32_t value;

    if (dpiDeqOptions_getWait(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetCondition()
//   Set the value of the condition option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetCondition(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    return DeqOptions_SetAttrText(self, valueObj, dpiDeqOptions_setCondition);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetConsumerName()
//   Set the value of the consumer name option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetConsumerName(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    return DeqOptions_SetAttrText(self, valueObj,
            dpiDeqOptions_setConsumerName);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetCorrelation()
//   Set the value of the correlation option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetCorrelation(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    return DeqOptions_SetAttrText(self, valueObj,
            dpiDeqOptions_setCorrelation);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetDeliveryMode()
//   Set the value of the delivery mode option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetDeliveryMode(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    dpiMessageDeliveryMode value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setDeliveryMode(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_SetMode()
//   Set the value of the mode option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetMode(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    dpiDeqMode value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setMode(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_SetMsgId()
//   Set the value of the message id option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetMsgId(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    Py_ssize_t valueLength;
    char *value;

    if (PyBytes_AsStringAndSize(valueObj, &value, &valueLength) < 0)
        return -1;
    if (dpiDeqOptions_setMsgId(self->handle, value,
            (uint32_t) valueLength) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_SetNavigation()
//   Set the value of the navigation option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetNavigation(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    dpiDeqNavigation value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setNavigation(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_SetTransformation()
//   Set the value of the correlation option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetTransformation(udt_DeqOptions *self,
        PyObject *valueObj, void *unused)
{
    return DeqOptions_SetAttrText(self, valueObj,
            dpiDeqOptions_setTransformation);
}


//-----------------------------------------------------------------------------
// DeqOptions_SetVisibility()
//   Set the value of the visibility option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetVisibility(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    dpiVisibility value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setVisibility(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// DeqOptions_SetWait()
//   Set the value of the wait option.
//-----------------------------------------------------------------------------
static int DeqOptions_SetWait(udt_DeqOptions *self, PyObject *valueObj,
        void *unused)
{
    uint32_t value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setWait(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}

