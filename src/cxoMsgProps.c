//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoMsgProps.c
//   Implements the message properties object used in Advanced Queuing.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoMsgProps_new()
//   Create a new message properties object.
//-----------------------------------------------------------------------------
cxoMsgProps *cxoMsgProps_new(cxoConnection *connection, dpiMsgProps *handle)
{
    cxoMsgProps *props;

    props = (cxoMsgProps*) cxoPyTypeMsgProps.tp_alloc(&cxoPyTypeMsgProps, 0);
    if (!props) {
        if (handle)
            dpiMsgProps_release(handle);
        return NULL;
    }
    if (!handle && dpiConn_newMsgProps(connection->handle, &handle) < 0) {
        Py_DECREF(props);
        cxoError_raiseAndReturnNull();
        return NULL;
    }
    props->handle = handle;
    props->encoding = connection->encodingInfo.encoding;

    return props;
}


//-----------------------------------------------------------------------------
// cxoMsgProps_free()
//   Free the memory associated with the message properties object.
//-----------------------------------------------------------------------------
static void cxoMsgProps_free(cxoMsgProps *props)
{
    if (props->handle) {
        dpiMsgProps_release(props->handle);
        props->handle = NULL;
    }
    Py_CLEAR(props->payload);
    Py_TYPE(props)->tp_free((PyObject*) props);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getAttrInt32()
//   Get the value of the attribute as a 32-bit integer.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getAttrInt32(cxoMsgProps *props,
        int (*func)(dpiMsgProps *props, int32_t *value))
{
    int32_t value;

    if ((*func)(props->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_setAttrInt32()
//   Set the value of the attribute as a 32-bit integer.
//-----------------------------------------------------------------------------
static int cxoMsgProps_setAttrInt32(cxoMsgProps *props, PyObject *valueObj,
        int (*func)(dpiMsgProps *props, int32_t value))
{
    int32_t value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if ((*func)(props->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getNumAttempts()
//   Get the value of the attempts property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getNumAttempts(cxoMsgProps *props, void *unused)
{
    return cxoMsgProps_getAttrInt32(props, dpiMsgProps_getNumAttempts);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getCorrelation()
//   Get the value of the correlation property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getCorrelation(cxoMsgProps *props, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiMsgProps_getCorrelation(props->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyUnicode_Decode(value, valueLength, props->encoding, NULL);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getDelay()
//   Get the value of the delay property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getDelay(cxoMsgProps *props, void *unused)
{
    return cxoMsgProps_getAttrInt32(props, dpiMsgProps_getDelay);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getDeliveryMode()
//   Get the value of the delivery mode property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getDeliveryMode(cxoMsgProps *props, void *unused)
{
    dpiMessageDeliveryMode value;

    if (dpiMsgProps_getDeliveryMode(props->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getEnqTime()
//   Get the value of the enqueue time property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getEnqTime(cxoMsgProps *props, void *unused)
{
    dpiDataBuffer buffer;

    if (dpiMsgProps_getEnqTime(props->handle, &buffer.asTimestamp) < 0)
        return cxoError_raiseAndReturnNull();
    return cxoTransform_toPython(CXO_TRANSFORM_DATETIME, NULL, NULL, &buffer,
            NULL);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getExceptionQ()
//   Get the value of the exception queue property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getExceptionQ(cxoMsgProps *props, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiMsgProps_getExceptionQ(props->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyUnicode_Decode(value, valueLength, props->encoding, NULL);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getExpiration()
//   Get the value of the expiration property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getExpiration(cxoMsgProps *props, void *unused)
{
    return cxoMsgProps_getAttrInt32(props, dpiMsgProps_getExpiration);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getMsgId()
//   Get the value of the msgid property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getMsgId(cxoMsgProps *props, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiMsgProps_getMsgId(props->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyBytes_FromStringAndSize(value, valueLength);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getPriority()
//   Get the value of the priority property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getPriority(cxoMsgProps *props, void *unused)
{
    return cxoMsgProps_getAttrInt32(props, dpiMsgProps_getPriority);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_getState()
//   Get the value of the state property.
//-----------------------------------------------------------------------------
static PyObject *cxoMsgProps_getState(cxoMsgProps *props, void *unused)
{
    dpiMessageState value;

    if (dpiMsgProps_getState(props->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_setCorrelation()
//   Set the value of the correlation property.
//-----------------------------------------------------------------------------
static int cxoMsgProps_setCorrelation(cxoMsgProps *props, PyObject *valueObj,
        void *unused)
{
    cxoBuffer buffer;
    int status;

    if (cxoBuffer_fromObject(&buffer, valueObj, props->encoding))
        return -1;
    status = dpiMsgProps_setCorrelation(props->handle, buffer.ptr,
            buffer.size);
    cxoBuffer_clear(&buffer);
    if (status < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoMsgProps_setDelay()
//   Set the value of the delay property.
//-----------------------------------------------------------------------------
static int cxoMsgProps_setDelay(cxoMsgProps *props, PyObject *valueObj,
        void *unused)
{
    return cxoMsgProps_setAttrInt32(props, valueObj, dpiMsgProps_setDelay);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_setExceptionQ()
//   Set the value of the exception queue property.
//-----------------------------------------------------------------------------
static int cxoMsgProps_setExceptionQ(cxoMsgProps *props, PyObject *valueObj,
        void *unused)
{
    cxoBuffer buffer;
    int status;

    if (cxoBuffer_fromObject(&buffer, valueObj, props->encoding))
        return -1;
    status = dpiMsgProps_setExceptionQ(props->handle, buffer.ptr, buffer.size);
    cxoBuffer_clear(&buffer);
    if (status < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoMsgProps_setExpiration()
//   Set the value of the expiration property.
//-----------------------------------------------------------------------------
static int cxoMsgProps_setExpiration(cxoMsgProps *props, PyObject *valueObj,
        void *unused)
{
    return cxoMsgProps_setAttrInt32(props, valueObj, dpiMsgProps_setExpiration);
}


//-----------------------------------------------------------------------------
// cxoMsgProps_setPriority()
//   Set the value of the expiration property.
//-----------------------------------------------------------------------------
static int cxoMsgProps_setPriority(cxoMsgProps *props, PyObject *valueObj,
        void *unused)
{
    return cxoMsgProps_setAttrInt32(props, valueObj, dpiMsgProps_setPriority);
}


//-----------------------------------------------------------------------------
// declaration of members
//-----------------------------------------------------------------------------
static PyMemberDef cxoMembers[] = {
    { "payload", T_OBJECT, offsetof(cxoMsgProps, payload), 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members
//-----------------------------------------------------------------------------
static PyGetSetDef cxoCalcMembers[] = {
    { "attempts", (getter) cxoMsgProps_getNumAttempts, 0, 0, 0 },
    { "correlation", (getter) cxoMsgProps_getCorrelation,
            (setter) cxoMsgProps_setCorrelation, 0, 0 },
    { "delay", (getter) cxoMsgProps_getDelay, (setter) cxoMsgProps_setDelay, 0,
            0 },
    { "deliverymode", (getter) cxoMsgProps_getDeliveryMode, 0, 0, 0 },
    { "enqtime", (getter) cxoMsgProps_getEnqTime, 0, 0, 0 },
    { "exceptionq", (getter) cxoMsgProps_getExceptionQ,
            (setter) cxoMsgProps_setExceptionQ, 0, 0 },
    { "expiration", (getter) cxoMsgProps_getExpiration,
            (setter) cxoMsgProps_setExpiration, 0, 0 },
    { "msgid", (getter) cxoMsgProps_getMsgId, 0, 0, 0 },
    { "priority", (getter) cxoMsgProps_getPriority,
            (setter) cxoMsgProps_setPriority, 0, 0 },
    { "state", (getter) cxoMsgProps_getState, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeMsgProps = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.MessageProperties",
    .tp_basicsize = sizeof(cxoMsgProps),
    .tp_dealloc = (destructor) cxoMsgProps_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMembers,
    .tp_getset = cxoCalcMembers
};
