//-----------------------------------------------------------------------------
// Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoApiType.c
//   Defines the objects used for identifying types defined by the Python
// Database API.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoApiType_free()
//   Free the API type object.
//-----------------------------------------------------------------------------
static void cxoApiType_free(cxoApiType *apiType)
{
    Py_CLEAR(apiType->dbTypes);
    Py_TYPE(apiType)->tp_free((PyObject*) apiType);
}


//-----------------------------------------------------------------------------
// cxoApiType_repr()
//   Return a string representation of a queue.
//-----------------------------------------------------------------------------
static PyObject *cxoApiType_repr(cxoApiType *apiType)
{
    PyObject *module, *name, *apiTypeName, *result;

    apiTypeName = PyUnicode_DecodeASCII(apiType->name, strlen(apiType->name),
            NULL);
    if (!apiTypeName)
        return NULL;
    if (cxoUtils_getModuleAndName(Py_TYPE(apiType), &module, &name) < 0) {
        Py_DECREF(apiTypeName);
        return NULL;
    }
    result = cxoUtils_formatString("<%s.%s %s>",
            PyTuple_Pack(3, module, name, apiTypeName));
    Py_DECREF(module);
    Py_DECREF(name);
    Py_DECREF(apiTypeName);
    return result;
}


//-----------------------------------------------------------------------------
// cxoApiType_reduce()
//   Method provided for pickling/unpickling of API types.
//-----------------------------------------------------------------------------
static PyObject *cxoApiType_reduce(cxoApiType *apiType)
{
    return PyUnicode_DecodeASCII(apiType->name, strlen(apiType->name), NULL);
}


//-----------------------------------------------------------------------------
// declaration of methods
//-----------------------------------------------------------------------------
static PyMethodDef cxoMethods[] = {
    { "__reduce__", (PyCFunction) cxoApiType_reduce, METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// declaration of members
//-----------------------------------------------------------------------------
static PyMemberDef cxoMembers[] = {
    { "name", T_STRING, offsetof(cxoApiType, name), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeApiType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.ApiType",
    .tp_basicsize = sizeof(cxoApiType),
    .tp_dealloc = (destructor) cxoApiType_free,
    .tp_repr = (reprfunc) cxoApiType_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_members = cxoMembers,
    .tp_methods = cxoMethods
};
