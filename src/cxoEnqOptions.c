//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoEnqOptions.c
//   Implements the enqueue options objects used in Advanced Queuing.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoEnqOptions_new()
//   Create a new enqueue options object.
//-----------------------------------------------------------------------------
cxoEnqOptions *cxoEnqOptions_new(cxoConnection *connection,
        dpiEnqOptions *handle)
{
    cxoEnqOptions *options;
    int status;

    options = (cxoEnqOptions*)
            cxoPyTypeEnqOptions.tp_alloc(&cxoPyTypeEnqOptions, 0);
    if (!options)
        return NULL;
    if (handle) {
        status = dpiEnqOptions_addRef(handle);
    } else {
        status = dpiConn_newEnqOptions(connection->handle, &handle);
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
// cxoEnqOptions_free()
//   Free the memory associated with the enqueue options object.
//-----------------------------------------------------------------------------
static void cxoEnqOptions_free(cxoEnqOptions *self)
{
    if (self->handle) {
        dpiEnqOptions_release(self->handle);
        self->handle = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// cxoEnqOptions_getTransformation()
//   Get the value of the transformation option.
//-----------------------------------------------------------------------------
static PyObject *cxoEnqOptions_getTransformation(cxoEnqOptions *self,
        void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiEnqOptions_getTransformation(self->handle, &value,
            &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (!value)
        Py_RETURN_NONE;
    return PyUnicode_Decode(value, valueLength, self->encoding, NULL);
}


//-----------------------------------------------------------------------------
// cxoEnqOptions_getVisibility()
//   Get the value of the visibility option.
//-----------------------------------------------------------------------------
static PyObject *cxoEnqOptions_getVisibility(cxoEnqOptions *self, void *unused)
{
    dpiVisibility value;

    if (dpiEnqOptions_getVisibility(self->handle, &value) < 0)
        return cxoError_raiseAndReturnNull();
    return PyLong_FromLong(value);
}


//-----------------------------------------------------------------------------
// cxoEnqOptions_setDeliveryMode()
//   Set the value of the delivery mode option.
//-----------------------------------------------------------------------------
static int cxoEnqOptions_setDeliveryMode(cxoEnqOptions *self, PyObject *valueObj,
        void *unused)
{
    dpiMessageDeliveryMode value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiEnqOptions_setDeliveryMode(self->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoEnqOptions_setTransformation()
//   Set the value of the transformation option.
//-----------------------------------------------------------------------------
static int cxoEnqOptions_setTransformation(cxoEnqOptions *self,
        PyObject *valueObj, void *unused)
{
    cxoBuffer buffer;
    int status;

    if (cxoBuffer_fromObject(&buffer, valueObj, self->encoding) < 0)
        return -1;
    status = dpiEnqOptions_setTransformation(self->handle, buffer.ptr,
            buffer.size);
    cxoBuffer_clear(&buffer);
    if (status < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// cxoEnqOptions_setVisibility()
//   Set the value of the visibility option.
//-----------------------------------------------------------------------------
static int cxoEnqOptions_setVisibility(cxoEnqOptions *self,
        PyObject *valueObj, void *unused)
{
    dpiVisibility value;

    value = PyLong_AsLong(valueObj);
    if (PyErr_Occurred())
        return -1;
    if (dpiEnqOptions_setVisibility(self->handle, value) < 0)
        return cxoError_raiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// declaration of calculated members for Python type
//-----------------------------------------------------------------------------
static PyGetSetDef cxoEnqOptionsCalcMembers[] = {
    { "deliverymode", 0, (setter) cxoEnqOptions_setDeliveryMode, 0, 0 },
    { "transformation", (getter) cxoEnqOptions_getTransformation,
            (setter) cxoEnqOptions_setTransformation, 0, 0 },
    { "visibility", (getter) cxoEnqOptions_getVisibility,
            (setter) cxoEnqOptions_setVisibility, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeEnqOptions = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.EnqOptions",
    .tp_basicsize = sizeof(cxoEnqOptions),
    .tp_dealloc = (destructor) cxoEnqOptions_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_getset = cxoEnqOptionsCalcMembers
};
