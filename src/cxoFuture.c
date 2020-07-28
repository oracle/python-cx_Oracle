//-----------------------------------------------------------------------------
// Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoFuture.c
//   Defines the object used for managing behavior changes. This object permits
// setting any attribute to any value but only tracks certain values.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoFuture_free()
//   Free the future object and reset global.
//-----------------------------------------------------------------------------
static void cxoFuture_free(cxoFuture *obj)
{
    Py_TYPE(obj)->tp_free((PyObject*) obj);
    cxoFutureObj = NULL;
}


//-----------------------------------------------------------------------------
// cxoFuture_getAttr()
//   Retrieve an attribute on an object.
//-----------------------------------------------------------------------------
static PyObject *cxoFuture_getAttr(cxoFuture *obj, PyObject *nameObject)
{
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoFuture_setAttr()
//   Set an attribute on an object.
//-----------------------------------------------------------------------------
static int cxoFuture_setAttr(cxoFuture *obj, PyObject *nameObject,
        PyObject *value)
{
    return 0;
}


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeFuture = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.__future__",
    .tp_basicsize = sizeof(cxoFuture),
    .tp_dealloc = (destructor) cxoFuture_free,
    .tp_getattro = (getattrofunc) cxoFuture_getAttr,
    .tp_setattro = (setattrofunc) cxoFuture_setAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT
};
