//-----------------------------------------------------------------------------
// Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// cxoSodaDoc.c
//   Defines the routines for handling SODA documents.
//-----------------------------------------------------------------------------

#include "cxoModule.h"

// forward declarations
static PyObject *cxoSodaDoc_getContentAsString(cxoSodaDoc *doc,
        PyObject *args);


//-----------------------------------------------------------------------------
// cxoSodaDoc_new()
//   Create a new SODA document.
//-----------------------------------------------------------------------------
cxoSodaDoc *cxoSodaDoc_new(cxoSodaDatabase *db, dpiSodaDoc *handle)
{
    cxoSodaDoc *doc;

    doc = (cxoSodaDoc*) cxoPyTypeSodaDoc.tp_alloc(&cxoPyTypeSodaDoc, 0);
    if (!doc) {
        dpiSodaDoc_release(handle);
        return NULL;
    }
    Py_INCREF(db);
    doc->db = db;
    doc->handle = handle;
    return doc;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_free()
//   Free the memory associated with a SODA document.
//-----------------------------------------------------------------------------
static void cxoSodaDoc_free(cxoSodaDoc *doc)
{
    if (doc->handle) {
        dpiSodaDoc_release(doc->handle);
        doc->handle = NULL;
    }
    Py_CLEAR(doc->db);
    Py_TYPE(doc)->tp_free((PyObject*) doc);
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_repr()
//   Return a string representation of a SODA document.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_repr(cxoSodaDoc *doc)
{
    PyObject *module, *name, *result, *keyObj;
    uint32_t keyLength;
    const char *key;

    if (dpiSodaDoc_getKey(doc->handle, &key, &keyLength) < 0)
        return cxoError_raiseAndReturnNull();
    keyObj = PyUnicode_Decode(key, keyLength,
            doc->db->connection->encodingInfo.encoding, NULL);
    if (!keyObj)
        return NULL;
    if (cxoUtils_getModuleAndName(Py_TYPE(doc), &module, &name) < 0) {
        Py_DECREF(keyObj);
        return NULL;
    }
    result = cxoUtils_formatString("<%s.%s with key %s>",
            PyTuple_Pack(3, module, name, keyObj));
    Py_DECREF(module);
    Py_DECREF(name);
    return result;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getCreatedOn()
//   Retrieve the time the SODA document was created, as a string in ISO 8601
// format.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getCreatedOn(cxoSodaDoc *doc, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiSodaDoc_getCreatedOn(doc->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (valueLength > 0)
        return PyUnicode_Decode(value, valueLength,
                doc->db->connection->encodingInfo.encoding, NULL);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getKey()
//   Retrieve the key for the SODA document.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getKey(cxoSodaDoc *doc, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiSodaDoc_getKey(doc->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (valueLength > 0)
        return PyUnicode_Decode(value, valueLength,
                doc->db->connection->encodingInfo.encoding, NULL);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getLastModified()
//   Retrieve the time the SODA document was last modified, as a string in ISO
// 8601 format.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getLastModified(cxoSodaDoc *doc, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiSodaDoc_getLastModified(doc->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (valueLength > 0)
        return PyUnicode_Decode(value, valueLength,
                doc->db->connection->encodingInfo.encoding, NULL);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getMediaType()
//   Retrieve the media type of the SODA document.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getMediaType(cxoSodaDoc *doc, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiSodaDoc_getMediaType(doc->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (valueLength > 0)
        return PyUnicode_Decode(value, valueLength,
                doc->db->connection->encodingInfo.encoding, NULL);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getVersion()
//   Retrieve the version for the SODA document.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getVersion(cxoSodaDoc *doc, void *unused)
{
    uint32_t valueLength;
    const char *value;

    if (dpiSodaDoc_getVersion(doc->handle, &value, &valueLength) < 0)
        return cxoError_raiseAndReturnNull();
    if (valueLength > 0)
        return PyUnicode_Decode(value, valueLength,
                doc->db->connection->encodingInfo.encoding, NULL);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getContent()
//   Get the content from the document and return a Python object.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getContent(cxoSodaDoc *doc, PyObject *args)
{
    PyObject *str, *result;

    str = cxoSodaDoc_getContentAsString(doc, args);
    if (!str)
        return NULL;
    if (str == Py_None)
        return str;
    result = PyObject_CallFunctionObjArgs(cxoJsonLoadFunction, str, NULL);
    Py_DECREF(str);
    return result;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getContentAsBytes()
//   Get the content from the document and return a bytes object.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getContentAsBytes(cxoSodaDoc *doc, PyObject *args)
{
    const char *content, *encoding;
    uint32_t contentLength;

    if (dpiSodaDoc_getContent(doc->handle, &content, &contentLength,
            &encoding) < 0)
        return cxoError_raiseAndReturnNull();
    if (contentLength > 0)
        return PyBytes_FromStringAndSize(content, contentLength);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// cxoSodaDoc_getContentAsString()
//   Get the content from the document and return a string.
//-----------------------------------------------------------------------------
static PyObject *cxoSodaDoc_getContentAsString(cxoSodaDoc *doc, PyObject *args)
{
    const char *content, *encoding;
    uint32_t contentLength;

    if (dpiSodaDoc_getContent(doc->handle, &content, &contentLength,
            &encoding) < 0)
        return cxoError_raiseAndReturnNull();
    if (contentLength > 0)
        return PyUnicode_Decode(content, contentLength, encoding, NULL);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// declaration of methods
//-----------------------------------------------------------------------------
static PyMethodDef cxoMethods[] = {
    { "getContent", (PyCFunction) cxoSodaDoc_getContent, METH_NOARGS },
    { "getContentAsBytes", (PyCFunction) cxoSodaDoc_getContentAsBytes,
            METH_NOARGS },
    { "getContentAsString", (PyCFunction) cxoSodaDoc_getContentAsString,
            METH_NOARGS },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of calculated members
//-----------------------------------------------------------------------------
static PyGetSetDef cxoCalcMembers[] = {
    { "createdOn", (getter) cxoSodaDoc_getCreatedOn, 0, 0, 0 },
    { "key", (getter) cxoSodaDoc_getKey, 0, 0, 0 },
    { "lastModified", (getter) cxoSodaDoc_getLastModified, 0, 0, 0 },
    { "mediaType", (getter) cxoSodaDoc_getMediaType, 0, 0, 0 },
    { "version", (getter) cxoSodaDoc_getVersion, 0, 0, 0 },
    { NULL }
};


//-----------------------------------------------------------------------------
// declaration of Python type
//-----------------------------------------------------------------------------
PyTypeObject cxoPyTypeSodaDoc = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cx_Oracle.SodaDoc",
    .tp_basicsize = sizeof(cxoSodaDoc),
    .tp_dealloc = (destructor) cxoSodaDoc_free,
    .tp_repr = (reprfunc) cxoSodaDoc_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = cxoMethods,
    .tp_getset = cxoCalcMembers
};
