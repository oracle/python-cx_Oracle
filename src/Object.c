//-----------------------------------------------------------------------------
// Copyright 2016, 2017, Oracle and/or its affiliates. All rights reserved.
//
// Portions Copyright 2007-2015, Anthony Tuininga. All rights reserved.
//
// Portions Copyright 2001-2007, Computronix (Canada) Ltd., Edmonton, Alberta,
// Canada. All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Object.c
//   Defines the routines for handling objects in Oracle.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// object type
//-----------------------------------------------------------------------------
typedef struct {
    PyObject_HEAD
    udt_ObjectType *objectType;
    dpiObject *handle;
} udt_Object;


//-----------------------------------------------------------------------------
// functions for the Python type "Object"
//-----------------------------------------------------------------------------
static void Object_Free(udt_Object*);
static PyObject *Object_GetAttr(udt_Object*, PyObject*);
static PyObject *Object_Repr(udt_Object*);
static int Object_SetAttr(udt_Object*, PyObject*, PyObject*);
static PyObject *Object_Append(udt_Object*, PyObject*);
static PyObject *Object_AsList(udt_Object*, PyObject*);
static PyObject *Object_Copy(udt_Object*, PyObject*);
static PyObject *Object_Delete(udt_Object*, PyObject*);
static PyObject *Object_Exists(udt_Object*, PyObject*);
static PyObject *Object_Extend(udt_Object*, PyObject*);
static PyObject *Object_GetElement(udt_Object*, PyObject*);
static PyObject *Object_GetFirstIndex(udt_Object*, PyObject*);
static PyObject *Object_GetLastIndex(udt_Object*, PyObject*);
static PyObject *Object_GetNextIndex(udt_Object*, PyObject*);
static PyObject *Object_GetPrevIndex(udt_Object*, PyObject*);
static PyObject *Object_GetSize(udt_Object*, PyObject*);
static PyObject *Object_SetElement(udt_Object*, PyObject*);
static PyObject *Object_Trim(udt_Object*, PyObject*);


//-----------------------------------------------------------------------------
// declaration of methods for Python type "Object"
//-----------------------------------------------------------------------------
static PyMethodDef g_ObjectMethods[] = {
    { "append", (PyCFunction) Object_Append, METH_VARARGS },
    { "aslist", (PyCFunction) Object_AsList, METH_NOARGS },
    { "copy", (PyCFunction) Object_Copy, METH_NOARGS },
    { "delete", (PyCFunction) Object_Delete, METH_VARARGS },
    { "exists", (PyCFunction) Object_Exists, METH_VARARGS },
    { "extend", (PyCFunction) Object_Extend, METH_VARARGS },
    { "first", (PyCFunction) Object_GetFirstIndex, METH_NOARGS },
    { "getelement", (PyCFunction) Object_GetElement, METH_VARARGS },
    { "last", (PyCFunction) Object_GetLastIndex, METH_NOARGS },
    { "next", (PyCFunction) Object_GetNextIndex, METH_VARARGS },
    { "prev", (PyCFunction) Object_GetPrevIndex, METH_VARARGS },
    { "setelement", (PyCFunction) Object_SetElement, METH_VARARGS },
    { "size", (PyCFunction) Object_GetSize, METH_NOARGS },
    { "trim", (PyCFunction) Object_Trim, METH_VARARGS },
    { NULL, NULL }
};


//-----------------------------------------------------------------------------
// Declaration of members for Python type "Object".
//-----------------------------------------------------------------------------
static PyMemberDef g_ObjectMembers[] = {
    { "type", T_OBJECT, offsetof(udt_Object, objectType), READONLY },
    { NULL }
};


//-----------------------------------------------------------------------------
// Python type declaration
//-----------------------------------------------------------------------------
static PyTypeObject g_ObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cx_Oracle.Object",                 // tp_name
    sizeof(udt_Object),                 // tp_basicsize
    0,                                  // tp_itemsize
    (destructor) Object_Free,           // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    (reprfunc) Object_Repr,             // tp_repr
    0,                                  // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    0,                                  // tp_str
    (getattrofunc) Object_GetAttr,      // tp_getattro
    (setattrofunc) Object_SetAttr,      // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    0,                                  // tp_doc
    0,                                  // tp_traverse
    0,                                  // tp_clear
    0,                                  // tp_richcompare
    0,                                  // tp_weaklistoffset
    0,                                  // tp_iter
    0,                                  // tp_iternext
    g_ObjectMethods,                    // tp_methods
    g_ObjectMembers                     // tp_members
};


//-----------------------------------------------------------------------------
// Object_New()
//   Create a new object.
//-----------------------------------------------------------------------------
PyObject *Object_New(udt_ObjectType *objectType, dpiObject *handle,
        int addReference)
{
    udt_Object *self;

    if (addReference && dpiObject_addRef(handle) < 0)
        return Error_RaiseAndReturnNull();
    self = (udt_Object*) g_ObjectType.tp_alloc(&g_ObjectType, 0);
    if (!self) {
        dpiObject_release(handle);
        return NULL;
    }
    Py_INCREF(objectType);
    self->objectType = objectType;
    self->handle = handle;
    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Object_Free()
//   Free an object.
//-----------------------------------------------------------------------------
static void Object_Free(udt_Object *self)
{
    if (self->handle) {
        dpiObject_release(self->handle);
        self->handle = NULL;
    }
    Py_CLEAR(self->objectType);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Object_Repr()
//   Return a string representation of the object.
//-----------------------------------------------------------------------------
static PyObject *Object_Repr(udt_Object *self)
{
    PyObject *module, *name, *result, *format, *formatArgs;

    if (GetModuleAndName(Py_TYPE(self), &module, &name) < 0)
        return NULL;
    format = cxString_FromAscii("<%s.%s %s.%s at %#x>");
    if (!format) {
        Py_DECREF(module);
        Py_DECREF(name);
        return NULL;
    }
    formatArgs = Py_BuildValue("(OOOOl)", module, name,
            self->objectType->schema, self->objectType->name, self);
    Py_DECREF(module);
    Py_DECREF(name);
    if (!formatArgs) {
        Py_DECREF(format);
        return NULL;
    }
    result = cxString_Format(format, formatArgs);
    Py_DECREF(format);
    Py_DECREF(formatArgs);
    return result;
}


//-----------------------------------------------------------------------------
// Object_ConvertFromPython()
//   Convert a Python value to an Oracle value.
//-----------------------------------------------------------------------------
static int Object_ConvertFromPython(udt_Object *obj, PyObject *value,
        dpiNativeTypeNum *nativeTypeNum, dpiData *data, udt_Buffer *buffer)
{
    dpiTimestamp *timestamp;
    udt_Object *otherObj;
    dpiBytes *bytes;
    udt_LOB *lob;

    // None is treated as null
    if (value == Py_None) {
        data->isNull = 1;
        return 0;
    }

    // convert the different Python types
    data->isNull = 0;
    if (PyUnicode_Check(value) || PyBytes_Check(value)) {
        if (cxBuffer_FromObject(buffer, value,
                obj->objectType->connection->encodingInfo.encoding) < 0)
            return -1;
        *nativeTypeNum = DPI_NATIVE_TYPE_BYTES;
        bytes = &data->value.asBytes;
        bytes->ptr = (char*) buffer->ptr;
        bytes->length = buffer->size;
    } else if (PyBool_Check(value)) {
        *nativeTypeNum = DPI_NATIVE_TYPE_BOOLEAN;
        data->value.asBoolean = (value == Py_True);
#if PY_MAJOR_VERSION < 3

    } else if (PyInt_Check(value)) {
        *nativeTypeNum = DPI_NATIVE_TYPE_INT64;
        data->value.asInt64 = PyInt_AS_LONG(value);
#endif
    } else if (PyLong_Check(value)) {
        *nativeTypeNum = DPI_NATIVE_TYPE_INT64;
        data->value.asInt64 = PyLong_AsLong(value);
        if (PyErr_Occurred())
            return -1;
    } else if (PyFloat_Check(value)) {
        *nativeTypeNum = DPI_NATIVE_TYPE_DOUBLE;
        data->value.asDouble = PyFloat_AS_DOUBLE(value);
    } else if (PyDateTime_Check(value) || PyDate_Check(value)) {
        *nativeTypeNum = DPI_NATIVE_TYPE_TIMESTAMP;
        timestamp = &data->value.asTimestamp;
        timestamp->year = PyDateTime_GET_YEAR(value);
        timestamp->month = PyDateTime_GET_MONTH(value);
        timestamp->day = PyDateTime_GET_DAY(value);
        if (PyDateTime_Check(value)) {
            timestamp->hour = PyDateTime_DATE_GET_HOUR(value);
            timestamp->minute = PyDateTime_DATE_GET_MINUTE(value);
            timestamp->second = PyDateTime_DATE_GET_SECOND(value);
            timestamp->fsecond = PyDateTime_DATE_GET_MICROSECOND(value) * 1000;
        } else {
            timestamp->hour = 0;
            timestamp->minute = 0;
            timestamp->second = 0;
            timestamp->fsecond = 0;
        }
    } else if (Py_TYPE(value) == &g_ObjectType) {
        *nativeTypeNum = DPI_NATIVE_TYPE_OBJECT;
        otherObj = (udt_Object*) value;
        data->value.asObject = otherObj->handle;
    } else if (Py_TYPE(value) == &g_LOBType) {
        *nativeTypeNum = DPI_NATIVE_TYPE_LOB;
        lob = (udt_LOB*) value;
        data->value.asLOB = lob->handle;
    } else {
        PyErr_Format(g_NotSupportedErrorException,
                "Object_ConvertFromPython(): unhandled value type");
        return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Object_ConvertToPython()
//   Convert an Oracle value to a Python value.
//-----------------------------------------------------------------------------
static PyObject *Object_ConvertToPython(udt_Object *obj,
        dpiOracleTypeNum oracleTypeNum, dpiNativeTypeNum nativeTypeNum,
        dpiData *data, udt_ObjectType *objType)
{
    dpiIntervalDS *intervalDS;
    dpiTimestamp *timestamp;
    dpiBytes *bytes;
    int32_t seconds;

    // null values returned as None
    if (data->isNull)
        Py_RETURN_NONE;

    // convert other values as required
    switch (nativeTypeNum) {
        case DPI_NATIVE_TYPE_INT64:
            return PyInt_FromLong((long) data->value.asInt64);
#if PY_MAJOR_VERSION >= 3
        case DPI_NATIVE_TYPE_UINT64:
            return PyInt_FromUnsignedLong(
                    (unsigned long) data->value.asUint64);
#endif
        case DPI_NATIVE_TYPE_FLOAT:
            return PyFloat_FromDouble(data->value.asFloat);
        case DPI_NATIVE_TYPE_DOUBLE:
            return PyFloat_FromDouble(data->value.asDouble);
        case DPI_NATIVE_TYPE_BYTES:
            bytes = &data->value.asBytes;
            return cxString_FromEncodedString(bytes->ptr, bytes->length,
                    bytes->encoding);
        case DPI_NATIVE_TYPE_TIMESTAMP:
            timestamp = &data->value.asTimestamp;
            return PyDateTime_FromDateAndTime(timestamp->year,
                    timestamp->month, timestamp->day, timestamp->hour,
                    timestamp->minute, timestamp->second,
                    timestamp->fsecond / 1000);
        case DPI_NATIVE_TYPE_INTERVAL_DS:
            intervalDS = &data->value.asIntervalDS;
            seconds = intervalDS->hours * 60 * 60 + intervalDS->minutes * 60 +
                    intervalDS->seconds;
            return PyDelta_FromDSU(intervalDS->days, seconds,
                    intervalDS->fseconds / 1000);
        case DPI_NATIVE_TYPE_OBJECT:
            return Object_New(objType, data->value.asObject, 0);
        case DPI_NATIVE_TYPE_BOOLEAN:
            if (data->value.asBoolean)
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        case DPI_NATIVE_TYPE_LOB:
            return LOB_New(obj->objectType->connection, oracleTypeNum,
                    data->value.asLOB);
        default:
            break;
    }

    return PyErr_Format(g_NotSupportedErrorException,
            "Object_ConvertToPython(): unhandled data type");
}


//-----------------------------------------------------------------------------
// Object_GetAttributeValue()
//   Retrieve an attribute on the object.
//-----------------------------------------------------------------------------
static PyObject *Object_GetAttributeValue(udt_Object *self,
        udt_ObjectAttribute *attribute)
{
    dpiData data;

    if (dpiObject_getAttributeValue(self->handle, attribute->handle,
            attribute->nativeTypeNum, &data) < 0)
        return Error_RaiseAndReturnNull();
    return Object_ConvertToPython(self, attribute->oracleTypeNum,
            attribute->nativeTypeNum, &data, attribute->type);
}


//-----------------------------------------------------------------------------
// Object_SetAttributeValue()
//   Set an attribute on the object.
//-----------------------------------------------------------------------------
static int Object_SetAttributeValue(udt_Object *self,
        udt_ObjectAttribute *attribute, PyObject *value)
{
    dpiNativeTypeNum nativeTypeNum = 0;
    udt_Buffer buffer;
    dpiData data;
    int status;

    cxBuffer_Init(&buffer);
    if (Object_ConvertFromPython(self, value, &nativeTypeNum, &data,
            &buffer) < 0)
        return -1;
    status = dpiObject_setAttributeValue(self->handle, attribute->handle,
            nativeTypeNum, &data);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// Object_GetAttr()
//   Retrieve an attribute on an object.
//-----------------------------------------------------------------------------
static PyObject *Object_GetAttr(udt_Object *self, PyObject *nameObject)
{
    udt_ObjectAttribute *attribute;

    attribute = (udt_ObjectAttribute*)
            PyDict_GetItem(self->objectType->attributesByName, nameObject);
    if (attribute)
        return Object_GetAttributeValue(self, attribute);

    return PyObject_GenericGetAttr( (PyObject*) self, nameObject);
}


//-----------------------------------------------------------------------------
// Object_SetAttr()
//   Set an attribute on an object.
//-----------------------------------------------------------------------------
static int Object_SetAttr(udt_Object *self, PyObject *nameObject,
        PyObject *value)
{
    udt_ObjectAttribute *attribute;

    attribute = (udt_ObjectAttribute*)
            PyDict_GetItem(self->objectType->attributesByName, nameObject);
    if (attribute)
        return Object_SetAttributeValue(self, attribute, value);

    return PyObject_GenericSetAttr( (PyObject*) self, nameObject, value);
}


//-----------------------------------------------------------------------------
// Object_InternalAppend()
//   Append an item to the collection.
//-----------------------------------------------------------------------------
static int Object_InternalAppend(udt_Object *self, PyObject *value)
{
    dpiNativeTypeNum nativeTypeNum = 0;
    udt_Buffer buffer;
    dpiData data;
    int status;

    cxBuffer_Init(&buffer);
    if (Object_ConvertFromPython(self, value, &nativeTypeNum, &data,
            &buffer) < 0)
        return -1;
    status = dpiObject_appendElement(self->handle, nativeTypeNum, &data);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnInt();
    return 0;
}


//-----------------------------------------------------------------------------
// Object_InternalExtend()
//   Extend the collection by appending each of the items in the sequence.
//-----------------------------------------------------------------------------
static int Object_InternalExtend(udt_Object *self, PyObject *sequence)
{
    PyObject *fastSequence, *element;
    Py_ssize_t size, i;

    // append each of the items in the sequence to the collection
    fastSequence = PySequence_Fast(sequence, "expecting sequence");
    if (!fastSequence)
        return -1;
    size = PySequence_Fast_GET_SIZE(fastSequence);
    for (i = 0; i < size; i++) {
        element = PySequence_Fast_GET_ITEM(fastSequence, i);
        if (Object_InternalAppend(self, element) < 0)
            return -1;
    }
    
    return 0;
}


//-----------------------------------------------------------------------------
// Object_Append()
//   Append an item to the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_Append(udt_Object *self, PyObject *args)
{
    PyObject *value;

    if (!PyArg_ParseTuple(args, "O", &value))
        return NULL;
    if (Object_InternalAppend(self, value) < 0)
        return NULL;

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_AsList()
//   Returns a collection as a list of elements. If the object is not a
// collection, an error is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_AsList(udt_Object *self, PyObject *args)
{
    PyObject *list, *elementValue;
    int32_t index, nextIndex;
    dpiData data;
    int exists;

    // create the result list
    list = PyList_New(0);
    if (!list)
        return NULL;

    // populate it with each of the elements in the list
    if (dpiObject_getFirstIndex(self->handle, &index, &exists) < 0) {
        Py_DECREF(list);
        return Error_RaiseAndReturnNull();
    }
    while (exists) {
        if (dpiObject_getElementValueByIndex(self->handle, index,
                self->objectType->elementNativeTypeNum, &data) < 0) {
            Py_DECREF(list);
            return Error_RaiseAndReturnNull();
        }
        elementValue = Object_ConvertToPython(self,
                self->objectType->elementOracleTypeNum,
                self->objectType->elementNativeTypeNum, &data,
                (udt_ObjectType*) self->objectType->elementType);
        if (!elementValue) {
            Py_DECREF(list);
            return NULL;
        }
        if (PyList_Append(list, elementValue) < 0) {
            Py_DECREF(elementValue);
            Py_DECREF(list);
            return NULL;
        }
        Py_DECREF(elementValue);
        if (dpiObject_getNextIndex(self->handle, index, &nextIndex,
                &exists) < 0) {
            Py_DECREF(list);
            return Error_RaiseAndReturnNull();
        }
        index = nextIndex;
    }

    return list;
}


//-----------------------------------------------------------------------------
// Object_Copy()
//   Return a copy of the object.
//-----------------------------------------------------------------------------
static PyObject *Object_Copy(udt_Object *self, PyObject *args)
{
    dpiObject *handle;

    if (dpiObject_copy(self->handle, &handle) < 0)
        return Error_RaiseAndReturnNull();
    return (PyObject*) Object_New(self->objectType, handle, 0);
}


//-----------------------------------------------------------------------------
// Object_Delete()
//   Delete the element at the specified index in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_Delete(udt_Object *self, PyObject *args)
{
    int32_t index;

    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    if (dpiObject_deleteElementByIndex(self->handle, index) < 0)
        return Error_RaiseAndReturnNull();
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_Exists()
//   Return true or false indicating if an element exists in the collection at
// the specified index.
//-----------------------------------------------------------------------------
static PyObject *Object_Exists(udt_Object *self, PyObject *args)
{
    int32_t index;
    int exists;

    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    if (dpiObject_getElementExistsByIndex(self->handle, index, &exists) < 0)
        return Error_RaiseAndReturnNull();
    if (exists)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}


//-----------------------------------------------------------------------------
// Object_Extend()
//   Extend the collection by appending each of the items in the sequence.
//-----------------------------------------------------------------------------
static PyObject *Object_Extend(udt_Object *self, PyObject *args)
{
    PyObject *sequence;

    if (!PyArg_ParseTuple(args, "O", &sequence))
        return NULL;
    if (Object_InternalExtend(self, sequence) < 0)
        return NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetElement()
//   Return the element at the given position in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_GetElement(udt_Object *self, PyObject *args)
{
    dpiData data;
    int32_t index;

    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    if (dpiObject_getElementValueByIndex(self->handle, index,
            self->objectType->elementNativeTypeNum, &data) < 0)
        return Error_RaiseAndReturnNull();
    return Object_ConvertToPython(self, self->objectType->elementOracleTypeNum,
            self->objectType->elementNativeTypeNum, &data,
            (udt_ObjectType*) self->objectType->elementType);
}


//-----------------------------------------------------------------------------
// Object_GetFirstIndex()
//   Return the index of the first entry in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_GetFirstIndex(udt_Object *self, PyObject *args)
{
    int32_t index;
    int exists;

    if (dpiObject_getFirstIndex(self->handle, &index, &exists) < 0)
        return Error_RaiseAndReturnNull();
    if (exists)
        return PyInt_FromLong(index);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetLastIndex()
//   Return the index of the last entry in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_GetLastIndex(udt_Object *self, PyObject *args)
{
    int32_t index;
    int exists;

    if (dpiObject_getLastIndex(self->handle, &index, &exists) < 0)
        return Error_RaiseAndReturnNull();
    if (exists)
        return PyInt_FromLong(index);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetNextIndex()
//   Return the index of the next entry in the collection following the index
// specified. If there is no next entry, None is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_GetNextIndex(udt_Object *self, PyObject *args)
{
    int32_t index, nextIndex;
    int exists;

    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    if (dpiObject_getNextIndex(self->handle, index, &nextIndex, &exists) < 0)
        return Error_RaiseAndReturnNull();
    if (exists)
        return PyInt_FromLong(nextIndex);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetPrevIndex()
//   Return the index of the previous entry in the collection preceding the
// index specified. If there is no previous entry, None is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_GetPrevIndex(udt_Object *self, PyObject *args)
{
    int32_t index, prevIndex;
    int exists;

    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    if (dpiObject_getPrevIndex(self->handle, index, &prevIndex, &exists) < 0)
        return Error_RaiseAndReturnNull();
    if (exists)
        return PyInt_FromLong(prevIndex);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetSize()
//   Return the size of a collection. If the object is not a collection, an
// error is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_GetSize(udt_Object *self, PyObject *args)
{
    int32_t size;

    if (dpiObject_getSize(self->handle, &size) < 0)
        return Error_RaiseAndReturnNull();
    return PyInt_FromLong(size);
}


//-----------------------------------------------------------------------------
// Object_SetElement()
//   Set the element at the specified location to the given value.
//-----------------------------------------------------------------------------
static PyObject *Object_SetElement(udt_Object *self, PyObject *args)
{
    dpiNativeTypeNum nativeTypeNum = 0;
    udt_Buffer buffer;
    PyObject *value;
    int32_t index;
    dpiData data;
    int status;

    if (!PyArg_ParseTuple(args, "iO", &index, &value))
        return NULL;
    cxBuffer_Init(&buffer);
    if (Object_ConvertFromPython(self, value, &nativeTypeNum, &data,
            &buffer) < 0)
        return NULL;
    status = dpiObject_setElementValueByIndex(self->handle, index,
            nativeTypeNum, &data);
    cxBuffer_Clear(&buffer);
    if (status < 0)
        return Error_RaiseAndReturnNull();
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_Trim()
//   Trim a number of elements from the end of the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_Trim(udt_Object *self, PyObject *args)
{
    int32_t numToTrim;

    if (!PyArg_ParseTuple(args, "i", &numToTrim))
        return NULL;
    if (dpiObject_trim(self->handle, numToTrim) < 0)
        return Error_RaiseAndReturnNull();
    Py_RETURN_NONE;
}

