//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LOB.c
//   Defines the routines for handling LOB values.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LOB type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    udt_Connection *connection;
    dpiOracleTypeNum oracleTypeNum;
    dpiLob *handle;
} udt_LOB;


//-----------------------------------------------------------------------------
// Declaration of external LOB functions.
//-----------------------------------------------------------------------------
static void LOB_Free(udt_LOB*);
static PyObject *LOB_Str(udt_LOB*);
static PyObject *LOB_Size(udt_LOB*, PyObject*);
static PyObject *LOB_Open(udt_LOB*, PyObject*);
static PyObject *LOB_Close(udt_LOB*, PyObject*);
static PyObject *LOB_Read(udt_LOB*, PyObject*, PyObject*);
static PyObject *LOB_Write(udt_LOB*, PyObject*, PyObject*);
static PyObject *LOB_Trim(udt_LOB*, PyObject*, PyObject*);
static PyObject *LOB_GetChunkSize(udt_LOB*, PyObject*);
static PyObject *LOB_IsOpen(udt_LOB*, PyObject*);
static PyObject *LOB_GetFileName(udt_LOB*, PyObject*);
static PyObject *LOB_SetFileName(udt_LOB*, PyObject*);
static PyObject *LOB_FileExists(udt_LOB*, PyObject*);
static PyObject *LOB_Reduce(udt_LOB*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "LOB"
//-----------------------------------------------------------------------------
static PyMethodDef g_LOBMethods[] = {
    { "size", (PyCFunction) LOB_Size, METH_NOARGS },
    { "open", (PyCFunction) LOB_Open, METH_NOARGS },
    { "close", (PyCFunction) LOB_Close, METH_NOARGS },
    { "read", (PyCFunction) LOB_Read, METH_VARARGS | METH_KEYWORDS },
    { "write", (PyCFunction) LOB_Write, METH_VARARGS | METH_KEYWORDS },
    { "trim", (PyCFunction) LOB_Trim, METH_VARARGS | METH_KEYWORDS },
    { "getchunksize", (PyCFunction) LOB_GetChunkSize, METH_NOARGS },
    { "isopen", (PyCFunction) LOB_IsOpen, METH_NOARGS },
    { "getfilename", (PyCFunction) LOB_GetFileName, METH_NOARGS },
    { "setfilename", (PyCFunction) LOB_SetFileName, METH_VARARGS },
    { "fileexists", (PyCFunction) LOB_FileExists, METH_NOARGS },
    { "__reduce__", (PyCFunction) LOB_Reduce, METH_NOARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
static PyTypeObject g_LOBType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.LOB",                    // tp_name
    sizeof(udt_LOB),                    // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) LOB_Free,              // tp_dealloc
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
    (reprfunc) LOB_Str,                 // tp_str
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
    g_LOBMethods,                       // tp_methods
    0,                                  // tp_members
    0,                                  // tp_getset
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
// LOB_New()
//   Create a new LOB.
//-----------------------------------------------------------------------------
PyObject *LOB_New(udt_Connection *connection, dpiOracleTypeNum oracleTypeNum,
        dpiLob *handle)
{
    udt_LOB *self;

    self = (udt_LOB*) g_LOBType.tp_alloc(&g_LOBType, 0);
    if (!self)
        return NULL;
    if (dpiLob_addRef(handle) < 0) {
        Py_DECREF(self);
        return NULL;
    }
    self->handle = handle;
    self->oracleTypeNum = oracleTypeNum;
    Py_INCREF(connection);
    self->connection = connection;
    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// LOB_Free()
//   Free a LOB.
//-----------------------------------------------------------------------------
static void LOB_Free(udt_LOB *self)
{
    if (self->handle) {
        dpiLob_release(self->handle);
        self->handle = NULL;
    }
    Py_CLEAR(self->connection);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// LOB_InternalRead()
//   Return a portion (or all) of the data in the LOB.
//-----------------------------------------------------------------------------
static PyObject *LOB_InternalRead(udt_LOB *self, uint64_t offset,
        uint64_t amount)
{
    uint64_t bufferSize;
    PyObject *result;
    char *buffer;
    int status;

    // modify the arguments
    if (amount == (uint64_t)(-1)) {
        if (dpiLob_getSize(self->handle, &amount) < 0)
            return Error_RaiseAndReturnNull();
        if (amount >= offset)
            amount = amount - offset + 1;
        else amount = 1;
    }

    // create a buffer of the correct size
    if (dpiLob_getBufferSize(self->handle, amount, &bufferSize) < 0)
        return Error_RaiseAndReturnNull();
    buffer = (char*) PyMem_Malloc(bufferSize);
    if (!buffer)
        return PyErr_NoMemory();

    // read the LOB
    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_readBytes(self->handle, offset, amount, buffer,
            &bufferSize);
    Py_END_ALLOW_THREADS
    if (status < 0) {
        PyMem_Free(buffer);
        return Error_RaiseAndReturnNull();
    }

    // return the result
    if (self->oracleTypeNum == DPI_ORACLE_TYPE_CLOB)
        result = PyUnicode_Decode(buffer, bufferSize,
                self->connection->encodingInfo.encoding, NULL);
    else if (self->oracleTypeNum == DPI_ORACLE_TYPE_NCLOB)
        result = cxString_FromEncodedString(buffer, bufferSize,
                self->connection->encodingInfo.nencoding);
    else result = PyBytes_FromStringAndSize(buffer, bufferSize);
    PyMem_Free(buffer);
    return result;
}


//-----------------------------------------------------------------------------
// LOB_InternalWrite()
//   Write the data in the Python object to the LOB.
//-----------------------------------------------------------------------------
static int LOB_InternalWrite(udt_LOB *self, PyObject *dataObj, uint64_t offset)
{
    const char *encoding;
    udt_Buffer buffer;
    int status;

    if (self->oracleTypeNum == DPI_ORACLE_TYPE_NCLOB)
        encoding = self->connection->encodingInfo.nencoding;
    else encoding = self->connection->encodingInfo.encoding;
    if (cxBuffer_FromObject(&buffer, dataObj, encoding) < 0)
        return -1;
    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_writeBytes(self->handle, offset,
            (char*) buffer.ptr, buffer.size);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// LOB_Size()
//   Return the size of the data in the LOB.
//-----------------------------------------------------------------------------
static PyObject *LOB_Size(udt_LOB *self, PyObject *args)
{
    uint64_t length;

    if (dpiLob_getSize(self->handle, &length) < 0)
        return Error_RaiseAndReturnNull();
    return PyLong_FromUnsignedLongLong(length);
}


//-----------------------------------------------------------------------------
// LOB_Open()
//   Open the LOB to speed further accesses.
//-----------------------------------------------------------------------------
static PyObject *LOB_Open(udt_LOB *self, PyObject *args)
{
    int status;

    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_openResource(self->handle);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// LOB_Close()
//   Close the LOB.
//-----------------------------------------------------------------------------
static PyObject *LOB_Close(udt_LOB *self, PyObject *args)
{
    int status;

    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_closeResource(self->handle);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// LOB_Read()
//   Return a portion (or all) of the data in the LOB.
//-----------------------------------------------------------------------------
static PyObject *LOB_Read(udt_LOB *self, PyObject *args, PyObject *keywordArgs)
{
    static char *keywordList[] = { "offset", "amount", NULL };
    unsigned PY_LONG_LONG offset, amount;

    // offset and amount are expected, both optional
    offset = 1;
    amount = (unsigned PY_LONG_LONG)(-1);
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|KK", keywordList,
            &offset, &amount))
        return NULL;
    return LOB_InternalRead(self, (uint64_t) offset, (uint64_t) amount);
}


//-----------------------------------------------------------------------------
// LOB_Str()
//   Return all of the data in the LOB.
//-----------------------------------------------------------------------------
static PyObject *LOB_Str(udt_LOB *self)
{
    return LOB_InternalRead(self, 1, (uint64_t)(-1));
}


//-----------------------------------------------------------------------------
// LOB_Write()
//   Write a value to the LOB.
//-----------------------------------------------------------------------------
static PyObject *LOB_Write(udt_LOB *self, PyObject *args,
        PyObject *keywordArgs)
{
    static char *keywordList[] = { "data", "offset", NULL };
    unsigned PY_LONG_LONG offset;
    PyObject *dataObj;

    offset = 1;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "O|K", keywordList,
            &dataObj, &offset))
        return NULL;
    if (LOB_InternalWrite(self, dataObj, (uint64_t) offset) < 0)
        return NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// LOB_Trim()
//   Trim the LOB to the specified length.
//-----------------------------------------------------------------------------
static PyObject *LOB_Trim(udt_LOB *self, PyObject *args, PyObject *keywordArgs)
{
    static char *keywordList[] = { "newSize", NULL };
    unsigned PY_LONG_LONG newSize;
    int status;

    newSize = 0;
    if (!PyArg_ParseTupleAndKeywords(args, keywordArgs, "|K", keywordList,
            &newSize))
        return NULL;
    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_trim(self->handle, (uint64_t) newSize);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// LOB_Reduce()
//   Method provided for pickling/unpickling of LOBs.
//-----------------------------------------------------------------------------
static PyObject *LOB_Reduce(udt_LOB *self)
{
    PyObject *result, *value;

    value = LOB_Str(self);
    if (!value)
        return NULL;
    result = Py_BuildValue("(O(O))", Py_TYPE(value), value);
    Py_DECREF(value);
    return result;
}


//-----------------------------------------------------------------------------
// LOB_GetChunkSize()
//   Return the chunk size that should be used when reading/writing the LOB in
// chunks.
//-----------------------------------------------------------------------------
static PyObject *LOB_GetChunkSize(udt_LOB *self, PyObject *args)
{
    uint32_t size;

    if (dpiLob_getChunkSize(self->handle, &size) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(size);
}


//-----------------------------------------------------------------------------
// LOB_IsOpen()
//   Return a boolean indicating if the lob is open or not.
//-----------------------------------------------------------------------------
static PyObject *LOB_IsOpen(udt_LOB *self, PyObject *args)
{
    int isOpen, status;

    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_getIsResourceOpen(self->handle, &isOpen);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();
    return PyBool_FromLong(isOpen);
}


//-----------------------------------------------------------------------------
// LOB_GetFileName()
//   Return the directory alias and file name for the BFILE lob.
//-----------------------------------------------------------------------------
static PyObject *LOB_GetFileName(udt_LOB *self, PyObject *args)
{
    uint32_t directoryAliasLength, fileNameLength;
    const char *directoryAlias, *fileName;
    PyObject *result, *temp;
    int status;

    // get the information from the LOB
    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_getDirectoryAndFileName(self->handle, &directoryAlias,
            &directoryAliasLength, &fileName, &fileNameLength);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();

    // create the two-tuple for returning
    result = PyTuple_New(2);
    if (!result)
        return NULL;
    temp = cxString_FromEncodedString(directoryAlias, directoryAliasLength,
            self->connection->encodingInfo.encoding);
    if (!temp) {
        Py_DECREF(result);
        return NULL;
    }
    PyTuple_SET_ITEM(result, 0, temp);
    temp = cxString_FromEncodedString(fileName, fileNameLength,
            self->connection->encodingInfo.encoding);
    if (!temp) {
        Py_DECREF(result);
        return NULL;
    }
    PyTuple_SET_ITEM(result, 1, temp);

    return result;
}


//-----------------------------------------------------------------------------
// LOB_SetFileName()
//   Set the directory alias and file name for the BFILE lob.
//-----------------------------------------------------------------------------
static PyObject *LOB_SetFileName(udt_LOB *self, PyObject *args)
{
    udt_Buffer directoryAliasBuffer, fileNameBuffer;
    PyObject *directoryAliasObj, *fileNameObj;
    int status;

    // get the directory alias and file name
    if (!PyArg_ParseTuple(args, "OO", &directoryAliasObj, &fileNameObj))
        return NULL;
    if (cxBuffer_FromObject(&directoryAliasBuffer, directoryAliasObj,
            self->connection->encodingInfo.encoding) < 0)
        return NULL;
    if (cxBuffer_FromObject(&fileNameBuffer, fileNameObj,
            self->connection->encodingInfo.encoding) < 0) {
        cxBuffer_Clear(&directoryAliasBuffer);
        return NULL;
    }

    // perform the work
    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_setDirectoryAndFileName(self->handle,
            (char*) directoryAliasBuffer.ptr, directoryAliasBuffer.size,
            (char*) fileNameBuffer.ptr, fileNameBuffer.size);
    Py_END_ALLOW_THREADS
    cxBuffer_Clear(&directoryAliasBuffer);
    cxBuffer_Clear(&fileNameBuffer);
    if (status < 0)
        return Error_RaiseAndReturnNull();

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// LOB_FileExists()
//   Return a boolean indicating if the BFIILE lob exists.
//-----------------------------------------------------------------------------
static PyObject *LOB_FileExists(udt_LOB *self, PyObject *args)
{
    int status, exists;

    Py_BEGIN_ALLOW_THREADS
    status = dpiLob_getFileExists(self->handle, &exists);
    Py_END_ALLOW_THREADS
    if (status < 0)
        return Error_RaiseAndReturnNull();
    if (exists)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

