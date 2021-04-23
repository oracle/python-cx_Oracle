//-----------------------------------------------------------------------------
// Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoSodaDocCursor.c
//   Defines the routines for handling SODA document cursors. These cursors
// permit iterating over the documents that match the criteria that was
// specified by the user.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoSodaDocCursor_new()
//   Create a new SODA document cursor.
//-----------------------------------------------------------------------------
cxoSodaDocCursor *cxoSodaDocCursor_new(cxoSodaDatabase *db,
        dpiSodaDocCursor *handle)
{
    cxoSodaDocCursor *cursor;

    cursor = (cxoSodaDocCursor*)
            cxoPyTypeSodaDocCursor.tp_alloc(&cxoPyTypeSodaDocCursor, 0);
    if (!cursor) {
        dpiSodaDocCursor_release(handle);
        return NULL;
    }
    Py_INCREF(db);
    cursor->db = db;
    cursor->handle = handle;
    return cursor;
}


//-----------------------------------------------------------------------------
// cxoSodaDocCursor_free()
//   Free the memory associated with a SODA document cursor.
//-----------------------------------------------------------------------------
static void cxoSodaDocCursor_free(cxoSodaDocCursor *cursor)
{
    if (cursor->handle) {
        dpiSodaDocCursor_release(cursor->handle);
        cursor->handle = NULL;
    }
    Py_CLEAR(cursor->db);
    Py_TYPE(cursor)->tp_free((PyObject*) cursor);
}


//-----------------------------------------------------------------------------
// cxoSodaDocCursor_repr()
//   Return a string representation of a SODA document cursor.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDocCursor_repr(cxoSodaDocCursor *cursor)
{
    PyObject *module, *name, *result;

    if (cxoUtils_getModuleAndName(Py_TYPE(cursor), &module, &name) < 0)
        return NULL;
    result = cxoUtils_formatString("<%s.%s>", PyTuple_Pack(2, module, name));
    Py_DECREF(module);
    Py_DECREF(name);
    return result;
}


//-----------------------------------------------------------------------------
// cxoSodaDocCursor_close()
//   Create a SODA collection and return it.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDocCursor_close(cxoSodaDocCursor *cursor,
        PyObject *args)
{
    if (dpiSodaDocCursor_close(cursor->handle) < 0)
        return cxoError_raiseAndReturnNull();
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDocCursor_getIter()
//   Return a reference to the cursor which supports the iterator protocol.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDocCursor_getIter(cxoSodaDocCursor *cursor)
{
    Py_INCREF(cursor);
    return (PyObject*) cursor;
}


//-----------------------------------------------------------------------------
// cxoSodaDocCursor_getNext()
//   Return the next document from the cursor.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDocCursor_getNext(cxoSodaDocCursor *cursor)
{
    dpiSodaDoc *handle;
    cxoSodaDoc *doc;
    int status;

    Py_BEGIN_ALLOW_THREADS
    status = dpiSodaDocCursor_getNext(cursor->handle, DPI_SODA_FLAGS_DEFAULT,
            &handle);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return cxoError_raiseAndReturnNull();
    if (!handle)
        return NULL;
    doc = cxoSodaDoc_new(cursor->db, handle);
    if (!doc)
        return NULL;
    return (PyObject*) doc;
}


//-----------------------------------------------------------------------------
// declaration of methods
//-----------------------------------------------------------------------------
static PyMethodDef cxoMethods[] = {
    { "close", (PyCFunction) cxoSodaDocCursor_close, METH_NOARGS },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declarations
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeSodaDocCursor = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.SodaDocCursor",
    .tp_basicsize = sizeof(cxoSodaDocCursor),
    .tp_dealloc = (destructor) cxoSodaDocCursor_free,
    .tp_repr = (reprfunc) cxoSodaDocCursor_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = (getiterfunc) cxoSodaDocCursor_getIter,
    .tp_iternext = (iternextfunc) cxoSodaDocCursor_getNext,
    .tp_methods = cxoMethods
};
