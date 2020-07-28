//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoDeqOptions.c
//   Implements the dequeue options objects used in Advanced Queuing.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoDeqOptions_new()
//   Create a new dequeue options object.
//-----------------------------------------------------------------------------
cxoDeqOptions *cxoDeqOptions_new(cxoConnection *connection,
        dpiDeqOptions *handle)
{
    cxoDeqOptions *options;
    int status;

    options = (cxoDeqOptions*)
            cxoPyTypeDeqOptions.tp_alloc(&cxoPyTypeDeqOptions, 0);
    if (!options)
        return NULL;
    if (handle) {
        status = dpiDeqOptions_addRef(handle);
    } else {
        status = dpiConn_newDeqOptions(connection->handle, &handle);
    }
    if (status < 0) {
        cxoError_raiseAndReturnNull();
        Py_DECREF(options);
        return NULL;
    }
    options->handle = handle;
    options->encoding = connection->encodingInfo.encoding;

    return options;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_free()
//   Free the memory associated with the dequeue options object.
//-----------------------------------------------------------------------------
static void cxoDeqOptions_free(cxoDeqOptions *options)
{
    if (options->handle) {
        dpiDeqOptions_release(options->handle);
        options->handle = NULL;
    }
    Py_TYPE(options)->tp_free((PyObject*) options);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getAttrText()
//   Get the value of the attribute as text.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getAttrText(cxoDeqOptions *options,
        int (*func)(dpiDeqOptions*, const char**, uint32_t*))
{
    uint32_t valueLength;
    const char *value;

    if ((*func)(options->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyUnicode_Decode(value, valueLength, options->encoding, NULL);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setAttrText()
//   Set the value of the attribute as text.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setAttrText(cxoDeqOptions *options, PyObject *value,
        int (*func)(dpiDeqOptions*, const char*, uint32_t))
{
    cxoBuffer buffer;
    int status;

    if (cxoBuffer_fromObject(&buffer, value, options->encoding))
        return -1;
    status = (*func)(options->handle, buffer.ptr, buffer.size);
    cxoBuffer_clear(&buffer);
    if (status < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getCondition()
//   Get the value of the condition option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getCondition(cxoDeqOptions *options,
        void *unused)
{
    return cxoDeqOptions_getAttrText(options, dpiDeqOptions_getCondition);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getConsumerName()
//   Get the value of the consumer name option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getConsumerName(cxoDeqOptions *options,
        void *unused)
{
    return cxoDeqOptions_getAttrText(options, dpiDeqOptions_getConsumerName);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getCorrelation()
//   Get the value of the correlation option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getCorrelation(cxoDeqOptions *options,
        void *unused)
{
    return cxoDeqOptions_getAttrText(options, dpiDeqOptions_getCorrelation);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getMode()
//   Get the value of the mode option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getMode(cxoDeqOptions *options, void *unused)
{
    dpiDeqMode value;

    if (dpiDeqOptions_getMode(options->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getMsgId()
//   Get the value of the message id option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getMsgId(cxoDeqOptions *options, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiDeqOptions_getMsgId(options->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyBytes_FromStringAndSize(value, valueLength);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getNavigation()
//   Get the value of the navigation option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getNavigation(cxoDeqOptions *options,
        void *unused)
{
    dpiDeqNavigation value;

    if (dpiDeqOptions_getNavigation(options->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getTransformation()
//   Get the value of the transformation option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getTransformation(cxoDeqOptions *options,
        void *unused)
{
    return cxoDeqOptions_getAttrText(options, dpiDeqOptions_getTransformation);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getVisibility()
//   Get the value of the visibility option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getVisibility(cxoDeqOptions *options,
        void *unused)
{
    dpiVisibility value;

    if (dpiDeqOptions_getVisibility(options->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_getWait()
//   Get the value of the wait option.
//-----------------------------------------------------------------------------
static PyObject *cxoDeqOptions_getWait(cxoDeqOptions *options, void *unused)
{
    uint32_t value;

    if (dpiDeqOptions_getWait(options->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setCondition()
//   Set the value of the condition option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setCondition(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    return cxoDeqOptions_setAttrText(options, valueObj,
            dpiDeqOptions_setCondition);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setConsumerName()
//   Set the value of the consumer name option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setConsumerName(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    return cxoDeqOptions_setAttrText(options, valueObj,
            dpiDeqOptions_setConsumerName);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setCorrelation()
//   Set the value of the correlation option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setCorrelation(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    return cxoDeqOptions_setAttrText(options, valueObj,
            dpiDeqOptions_setCorrelation);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setDeliveryMode()
//   Set the value of the delivery mode option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setDeliveryMode(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    dpiMessageDeliveryMode value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setDeliveryMode(options->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setMode()
//   Set the value of the mode option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setMode(cxoDeqOptions *options, PyObject *valueObj,
        void *unused)
{
    dpiDeqMode value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setMode(options->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setMsgId()
//   Set the value of the message id option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setMsgId(cxoDeqOptions *options, PyObject *valueObj,
        void *unused)
{
    Py_ssize_t valueLength;
    char *value;

    if (PyBytes_AsStringAndSize(valueObj, &value, &valueLength) < 0)
        return -1;
    if (dpiDeqOptions_setMsgId(options->handle, value,
            (uint32_t) valueLength) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setNavigation()
//   Set the value of the navigation option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setNavigation(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    dpiDeqNavigation value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setNavigation(options->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setTransformation()
//   Set the value of the correlation option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setTransformation(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    return cxoDeqOptions_setAttrText(options, valueObj,
            dpiDeqOptions_setTransformation);
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setVisibility()
//   Set the value of the visibility option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setVisibility(cxoDeqOptions *options,
        PyObject *valueObj, void *unused)
{
    dpiVisibility value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setVisibility(options->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoDeqOptions_setWait()
//   Set the value of the wait option.
//-----------------------------------------------------------------------------
static int cxoDeqOptions_setWait(cxoDeqOptions *options, PyObject *valueObj,
        void *unused)
{
    uint32_t value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiDeqOptions_setWait(options->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type
//-----------------------------------------------------------------------------
static PyGetSetDef cxoDeqOptionsCalcMembers[] = {
    { "condition", (getter) cxoDeqOptions_getCondition,
            (setter) cxoDeqOptions_setCondition, 0, 0 },
    { "consumername", (getter) cxoDeqOptions_getConsumerName,
            (setter) cxoDeqOptions_setConsumerName, 0, 0 },
    { "correlation", (getter) cxoDeqOptions_getCorrelation,
            (setter) cxoDeqOptions_setCorrelation, 0, 0 },
    { "deliverymode", 0, (setter) cxoDeqOptions_setDeliveryMode, 0, 0 },
    { "mode", (getter) cxoDeqOptions_getMode, (setter) cxoDeqOptions_setMode,
            0, 0 },
    { "msgid", (getter) cxoDeqOptions_getMsgId,
            (setter) cxoDeqOptions_setMsgId, 0, 0 },
    { "navigation", (getter) cxoDeqOptions_getNavigation,
            (setter) cxoDeqOptions_setNavigation, 0, 0 },
    { "transformation", (getter) cxoDeqOptions_getTransformation,
            (setter) cxoDeqOptions_setTransformation, 0, 0 },
    { "visibility", (getter) cxoDeqOptions_getVisibility,
            (setter) cxoDeqOptions_setVisibility, 0, 0 },
    { "wait", (getter) cxoDeqOptions_getWait, (setter) cxoDeqOptions_setWait,
            0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeDeqOptions = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.DeqOptions",
    .tp_basicsize = sizeof(cxoDeqOptions),
    .tp_dealloc = (destructor) cxoDeqOptions_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_getset = cxoDeqOptionsCalcMembers
};
