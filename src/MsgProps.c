//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// MsgProps.c
//   Implements the message properties object used in Advanced Queuing.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// structures used for handling AQ options and message properties
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    dpiMsgProps *handle;
    const char *encoding;
} udt_MsgProps;


//-----------------------------------------------------------------------------
// Declaration of methods used for message properties
//-----------------------------------------------------------------------------
static udt_MsgProps *MsgProps_New(udt_Connection*);
static void MsgProps_Free(udt_MsgProps*);
static PyObject *MsgProps_GetNumAttempts(udt_MsgProps*, void*);
static PyObject *MsgProps_GetCorrelation(udt_MsgProps*, void*);
static PyObject *MsgProps_GetDelay(udt_MsgProps*, void*);
static PyObject *MsgProps_GetDeliveryMode(udt_MsgProps*, void*);
static PyObject *MsgProps_GetEnqTime(udt_MsgProps*, void*);
static PyObject *MsgProps_GetExceptionQ(udt_MsgProps*, void*);
static PyObject *MsgProps_GetExpiration(udt_MsgProps*, void*);
static PyObject *MsgProps_GetOriginalMsgId(udt_MsgProps*, void*);
static PyObject *MsgProps_GetPriority(udt_MsgProps*, void*);
static PyObject *MsgProps_GetState(udt_MsgProps*, void*);
static int MsgProps_SetCorrelation(udt_MsgProps*, PyObject*, void*);
static int MsgProps_SetDelay(udt_MsgProps*, PyObject*, void*);
static int MsgProps_SetExceptionQ(udt_MsgProps*, PyObject*, void*);
static int MsgProps_SetExpiration(udt_MsgProps*, PyObject*, void*);
static int MsgProps_SetOriginalMsgId(udt_MsgProps*, PyObject*, void*);
static int MsgProps_SetPriority(udt_MsgProps*, PyObject*, void*);


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type "MessageProperties"
//-----------------------------------------------------------------------------
static PyGetSetDef g_MessagePropertiesCalcMembers[] = {
    { "attempts", (getter) MsgProps_GetNumAttempts, 0, 0, 0 },
    { "correlation", (getter) MsgProps_GetCorrelation,
            (setter) MsgProps_SetCorrelation, 0, 0 },
    { "delay", (getter) MsgProps_GetDelay, (setter) MsgProps_SetDelay, 0, 0 },
    { "deliverymode", (getter) MsgProps_GetDeliveryMode, 0, 0, 0 },
    { "enqtime", (getter) MsgProps_GetEnqTime, 0, 0, 0 },
    { "exceptionq", (getter) MsgProps_GetExceptionQ,
            (setter) MsgProps_SetExceptionQ, 0, 0 },
    { "expiration", (getter) MsgProps_GetExpiration,
            (setter) MsgProps_SetExpiration, 0, 0 },
    { "msgid", (getter) MsgProps_GetOriginalMsgId,
            (setter) MsgProps_SetOriginalMsgId, 0, 0 },
    { "priority", (getter) MsgProps_GetPriority,
            (setter) MsgProps_SetPriority, 0, 0 },
    { "state", (getter) MsgProps_GetState, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
static PyTypeObject g_MessagePropertiesType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.MessageProperties",      // tp_name
    sizeof(udt_MsgProps),               // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) MsgProps_Free,         // tp_dealloc
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
    g_MessagePropertiesCalcMembers,     // tp_getset
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
// MsgProps_New()
//   Create a new message properties object.
//-----------------------------------------------------------------------------
static udt_MsgProps *MsgProps_New(udt_Connection *connection)
{
    udt_MsgProps *self;

    self = (udt_MsgProps*)
            g_MessagePropertiesType.tp_alloc(&g_MessagePropertiesType, 0);
    if (!self)
        return NULL;
    if (dpiConn_newMsgProps(connection->handle, &self->handle) < 0) {
        Py_DECREF(self);
        Error_RaiseAndReturnNull();
        return NULL;
    }
    self->encoding = connection->encodingInfo.encoding;

    return self;
}


//-----------------------------------------------------------------------------
// MsgProps_Free()
//   Free the memory associated with the message properties object.
//-----------------------------------------------------------------------------
static void MsgProps_Free(udt_MsgProps *self)
{
    if (self->handle) {
        dpiMsgProps_release(self->handle);
        self->handle = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// MsgProps_GetAttrInt32()
//   Get the value of the attribute as a 32-bit integer.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetAttrInt32(udt_MsgProps *self,
        int (*func)(dpiMsgProps *props, int32_t *value))
{
    int32_t value;

    if ((*func)(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// MsgProps_SetAttrInt32()
//   Set the value of the attribute as a 32-bit integer.
//-----------------------------------------------------------------------------
static int MsgProps_SetAttrInt32(udt_MsgProps *self, PyObject *valueObj,
        int (*func)(dpiMsgProps *props, int32_t value))
{
    int32_t value;

    value = PyInt_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if ((*func)(self->handle, value) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// MsgProps_GetNumAttempts()
//   Get the value of the attempts property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetNumAttempts(udt_MsgProps *self, void *unused)
{
    return MsgProps_GetAttrInt32(self, dpiMsgProps_getNumAttempts);
}


//-----------------------------------------------------------------------------
// MsgProps_GetCorrelation()
//   Get the value of the correlation property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetCorrelation(udt_MsgProps *self, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiMsgProps_getCorrelation(self->handle, &value, &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return cxString_FromEncodedString(value, valueLength, self->encoding);
}


//-----------------------------------------------------------------------------
// MsgProps_GetDelay()
//   Get the value of the delay property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetDelay(udt_MsgProps *self, void *unused)
{
    return MsgProps_GetAttrInt32(self, dpiMsgProps_getDelay);
}


//-----------------------------------------------------------------------------
// MsgProps_GetDeliveryMode()
//   Get the value of the delivery mode property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetDeliveryMode(udt_MsgProps *self, void *unused)
{
    dpiMessageDeliveryMode value;

    if (dpiMsgProps_getDeliveryMode(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// MsgProps_GetEnqTime()
//   Get the value of the enqueue time property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetEnqTime(udt_MsgProps *self, void *unused)
{
    dpiTimestamp value;

    if (dpiMsgProps_getEnqTime(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyDateTime_FromDateAndTime(value.year, value.month, value.day,
            value.hour, value.minute, value.second, 0);
}


//-----------------------------------------------------------------------------
// MsgProps_GetExceptionQ()
//   Get the value of the exception queue property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetExceptionQ(udt_MsgProps *self, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiMsgProps_getExceptionQ(self->handle, &value, &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return cxString_FromEncodedString(value, valueLength, self->encoding);
}


//-----------------------------------------------------------------------------
// MsgProps_GetExpiration()
//   Get the value of the expiration property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetExpiration(udt_MsgProps *self, void *unused)
{
    return MsgProps_GetAttrInt32(self, dpiMsgProps_getExpiration);
}


//-----------------------------------------------------------------------------
// MsgProps_GetOriginalMsgId()
//   Get the value of the expiration property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetOriginalMsgId(udt_MsgProps *self, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiMsgProps_getOriginalMsgId(self->handle, &value, &valueLength) < 0)
        return Error_RaiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyBytes_FromStringAndSize(value, valueLength);
}


//-----------------------------------------------------------------------------
// MsgProps_GetPriority()
//   Get the value of the priority property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetPriority(udt_MsgProps *self, void *unused)
{
    return MsgProps_GetAttrInt32(self, dpiMsgProps_getPriority);
}


//-----------------------------------------------------------------------------
// MsgProps_GetState()
//   Get the value of the state property.
//-----------------------------------------------------------------------------
static PyObject *MsgProps_GetState(udt_MsgProps *self, void *unused)
{
    dpiMessageState value;

    if (dpiMsgProps_getState(self->handle, &value) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(value);
}


//-----------------------------------------------------------------------------
// MsgProps_SetCorrelation()
//   Set the value of the correlation property.
//-----------------------------------------------------------------------------
static int MsgProps_SetCorrelation(udt_MsgProps *self, PyObject *valueObj,
        void *unused)
{
    udt_Buffer buffer;
    int status;

    if (cxBuffer_FromObject(&buffer, valueObj, self->encoding))
        return -1;
    status = dpiMsgProps_setCorrelation(self->handle, buffer.ptr, buffer.size);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// MsgProps_SetDelay()
//   Set the value of the delay property.
//-----------------------------------------------------------------------------
static int MsgProps_SetDelay(udt_MsgProps *self, PyObject *valueObj,
        void *unused)
{
    return MsgProps_SetAttrInt32(self, valueObj, dpiMsgProps_setDelay);
}


//-----------------------------------------------------------------------------
// MsgProps_SetExceptionQ()
//   Set the value of the exception queue property.
//-----------------------------------------------------------------------------
static int MsgProps_SetExceptionQ(udt_MsgProps *self, PyObject *valueObj,
        void *unused)
{
    udt_Buffer buffer;
    int status;

    if (cxBuffer_FromObject(&buffer, valueObj, self->encoding))
        return -1;
    status = dpiMsgProps_setExceptionQ(self->handle, buffer.ptr, buffer.size);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// MsgProps_SetExpiration()
//   Set the value of the expiration property.
//-----------------------------------------------------------------------------
static int MsgProps_SetExpiration(udt_MsgProps *self, PyObject *valueObj,
        void *unused)
{
    return MsgProps_SetAttrInt32(self, valueObj, dpiMsgProps_setExpiration);
}


//-----------------------------------------------------------------------------
// MsgProps_SetOriginalMsgId()
//   Set the value of the original message id property.
//-----------------------------------------------------------------------------
static int MsgProps_SetOriginalMsgId(udt_MsgProps *self, PyObject *valueObj,
        void *unused)
{
    Py_ssize_t valueLength;
    char *value;

    if (PyBytes_AsStringAndSize(valueObj, &value, &valueLength) < 0)
        return -1;
    if (dpiMsgProps_setOriginalMsgId(self->handle, value,
            (uint32_t) valueLength) < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// MsgProps_SetPriority()
//   Set the value of the expiration property.
//-----------------------------------------------------------------------------
static int MsgProps_SetPriority(udt_MsgProps *self, PyObject *valueObj,
        void *unused)
{
    return MsgProps_SetAttrInt32(self, valueObj, dpiMsgProps_setPriority);
}

