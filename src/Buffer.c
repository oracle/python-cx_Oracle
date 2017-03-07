//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Buffer.c
//   Defines buffer structure and routines for populating it. These are used
// to translate Python objects into the buffers needed for Oracle, including
// Unicode or buffer objects.
//-----------------------------------------------------------------------------

// define structure for abstracting string buffers
typedef struct {
    const char *ptr;
    Py_ssize_t numCharacters;
    Py_ssize_t size;
    PyObject *obj;
} udt_Buffer;


//-----------------------------------------------------------------------------
// cxBuffer_Init()
//   Initialize the buffer with an empty string. Returns 0 as a convenience to
// the caller.
//-----------------------------------------------------------------------------
static int cxBuffer_Init(udt_Buffer *buf)
{
    buf->ptr = NULL;
    buf->size = 0;
    buf->numCharacters = 0;
    buf->obj = NULL;
    return 0;
}


//-----------------------------------------------------------------------------
// cxBuffer_FromObject()
//   Populate the string buffer from a unicode object.
//-----------------------------------------------------------------------------
static int cxBuffer_FromObject(udt_Buffer *buf, PyObject *obj,
        const char *encoding)
{
    if (!obj)
        return cxBuffer_Init(buf);
    if (PyUnicode_Check(obj)) {
        buf->obj = PyUnicode_AsEncodedString(obj, encoding, NULL);
        if (!buf->obj)
            return -1;
        buf->ptr = PyBytes_AS_STRING(buf->obj);
        buf->size = PyBytes_GET_SIZE(buf->obj);
#if PY_MAJOR_VERSION < 3
        buf->numCharacters = PyUnicode_GET_SIZE(obj);
#else
        buf->numCharacters = PyUnicode_GET_LENGTH(obj);
#endif
    } else if (PyBytes_Check(obj)) {
        Py_INCREF(obj);
        buf->obj = obj;
        buf->ptr = PyBytes_AS_STRING(buf->obj);
        buf->size = buf->numCharacters = PyBytes_GET_SIZE(buf->obj);
#if PY_MAJOR_VERSION < 3
    } else if (PyBuffer_Check(obj)) {
        if (PyObject_AsReadBuffer(obj, (void*) &buf->ptr, &buf->size) < 0)
            return -1;
        Py_INCREF(obj);
        buf->obj = obj;
        buf->numCharacters = buf->size;
#endif
    } else {
        PyErr_SetString(PyExc_TypeError, CXORA_TYPE_ERROR);
        return -1;
    }
    return 0;
}

#define cxBuffer_Clear(buf)             Py_CLEAR((buf)->obj)

