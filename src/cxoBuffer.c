//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoBuffer.c
//   Defines buffer structure and routines for populating it. These are used
// to translate Python objects into the buffers needed for Oracle, including
// Unicode or buffer objects.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

//-----------------------------------------------------------------------------
// cxoBuffer_fromObject()
//   Populate the string buffer from a unicode object.
//-----------------------------------------------------------------------------
int cxoBuffer_fromObject(cxoBuffer *buf, PyObject *obj, const char *encoding)
{
    cxoBuffer_init(buf);
    if (!obj || obj == Py_None)
        return 0;
    if (PyUnicode_Check(obj)) {
        buf->obj = PyUnicode_AsEncodedString(obj, encoding, NULL);
        if (!buf->obj)
            return -1;
        buf->ptr = PyBytes_AS_STRING(buf->obj);
        buf->size = (uint32_t) PyBytes_GET_SIZE(buf->obj);
        buf->numCharacters = (uint32_t) PyUnicode_GET_LENGTH(obj);
    } else if (PyBytes_Check(obj)) {
        Py_INCREF(obj);
        buf->obj = obj;
        buf->ptr = PyBytes_AS_STRING(buf->obj);
        buf->size = buf->numCharacters = (uint32_t) PyBytes_GET_SIZE(buf->obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "expecting string or bytes object");
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// cxoBuffer_init()
//   Initialize the buffer with an empty string. Returns 0 as a convenience to
// the caller.
//-----------------------------------------------------------------------------
int cxoBuffer_init(cxoBuffer *buf)
{
    buf->ptr = NULL;
    buf->size = 0;
    buf->numCharacters = 0;
    buf->obj = NULL;
    return 0;
}
