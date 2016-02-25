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
    dvoid *instance;
    dvoid *indicator;
    int isIndependent;
} udt_Object;


//-----------------------------------------------------------------------------
// functions for the Python type "Object"
//-----------------------------------------------------------------------------
static void Object_Free(udt_Object*);
static PyObject *Object_GetAttr(udt_Object*, PyObject*);
static int Object_SetAttr(udt_Object*, PyObject*, PyObject*);
static PyObject *Object_ConvertToPython(udt_Environment*, OCITypeCode, dvoid*,
        dvoid*, udt_ObjectType*);
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
    0,                                  // tp_repr
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
// Declaration of attribute data union
//-----------------------------------------------------------------------------
typedef union {
    boolean booleanValue;
    int integerValue;
    OCINumber numberValue;
    OCIDate dateValue;
    OCIDateTime *timestampValue;
    OCIString *stringValue;
} udt_AttributeData;


//-----------------------------------------------------------------------------
// AttributeData_Initialize()
//   Initialize any memory required for the convert from Python calls. All
// fields checked in the free routine should be initialized.
//-----------------------------------------------------------------------------
static void AttributeData_Initialize(
    udt_AttributeData *data,            // data structure to initialize
    OCITypeCode typeCode)               // type of Oracle data
{
    switch (typeCode) {
        case OCI_TYPECODE_CHAR:
        case OCI_TYPECODE_VARCHAR:
        case OCI_TYPECODE_VARCHAR2:
            data->stringValue = NULL;
            break;
        case OCI_TYPECODE_TIMESTAMP:
            data->timestampValue = NULL;
    };
}


//-----------------------------------------------------------------------------
// AttributeData_Free()
//   Free any memory that was allocated by the convert from Python calls.
//-----------------------------------------------------------------------------
static void AttributeData_Free(
    udt_Environment *environment,       // environment object
    udt_AttributeData *data,            // data structure to initialize
    OCITypeCode typeCode)               // type of Oracle data
{
    switch (typeCode) {
        case OCI_TYPECODE_CHAR:
        case OCI_TYPECODE_VARCHAR:
        case OCI_TYPECODE_VARCHAR2:
            if (data->stringValue)
                OCIStringResize(environment->handle, environment->errorHandle,
                        0, &data->stringValue);
            break;
        case OCI_TYPECODE_TIMESTAMP:
            if (data->timestampValue)
                OCIDescriptorFree(data->timestampValue, OCI_DTYPE_TIMESTAMP);
    };
}


//-----------------------------------------------------------------------------
// Object_New()
//   Create a new object.
//-----------------------------------------------------------------------------
PyObject *Object_New(
    udt_ObjectType *objectType,         // type of object
    dvoid *instance,                    // object instance data
    dvoid *indicator,                   // indicator structure
    int isIndependent)                  // is object independent?
{
    udt_Object *self;

    self = (udt_Object*) g_ObjectType.tp_alloc(&g_ObjectType, 0);
    if (!self)
        return NULL;
    Py_INCREF(objectType);
    self->objectType = objectType;
    self->instance = instance;
    self->indicator = indicator;
    self->isIndependent = isIndependent;
    return (PyObject*) self;
}


//-----------------------------------------------------------------------------
// Object_Create()
//   Create a new object in the OCI.
//-----------------------------------------------------------------------------
static udt_Object *Object_Create(
    udt_ObjectType *self)               // type of object to create
{
    dvoid *instance;
    udt_Object *obj;
    sword status;

    // create the object instance
    status = OCIObjectNew(self->connection->environment->handle,
            self->connection->environment->errorHandle,
            self->connection->handle, self->typeCode, self->tdo, NULL,
            OCI_DURATION_SESSION, TRUE, &instance);
    if (Environment_CheckForError(self->connection->environment, status,
            "Object_Create(): create object instance") < 0)
        return NULL;

    // create the object
    obj = (udt_Object*) Object_New(self, instance, NULL, 1);
    if (!obj) {
        OCIObjectFree(self->connection->environment->handle,
                self->connection->environment->errorHandle, instance,
                OCI_DEFAULT);
        return NULL;
    }

    // get the null indicator structure
    status = OCIObjectGetInd(self->connection->environment->handle,
            self->connection->environment->errorHandle, instance,
            &obj->indicator);
    if (Environment_CheckForError(self->connection->environment, status,
            "Object_Create(): get indicator structure") < 0) {
        Py_DECREF(obj);
        return NULL;
    }

    return obj;
}


//-----------------------------------------------------------------------------
// Object_Free()
//   Free an object.
//-----------------------------------------------------------------------------
static void Object_Free(
    udt_Object *self)                   // variable to free
{
    if (self->isIndependent)
        OCIObjectFree(self->objectType->connection->environment->handle,
                self->objectType->connection->environment->errorHandle,
                self->instance, OCI_DEFAULT);
    Py_CLEAR(self->objectType);
    Py_TYPE(self)->tp_free((PyObject*) self);
}


//-----------------------------------------------------------------------------
// Object_ConvertFromPython()
//   Convert a Python value to an Oracle value.
//-----------------------------------------------------------------------------
static int Object_ConvertFromPython(
    udt_Environment *environment,       // environment to use
    PyObject *pythonValue,              // Python value to convert
    OCITypeCode typeCode,               // type of Oracle data
    udt_AttributeData *oracleValue,     // Oracle value
    dvoid **ociValue,                   // OCI value
    OCIInd *ociValueIndicator,          // OCI value indicator
    dvoid **ociObjectIndicator,         // OCI object indicator
    udt_ObjectType *subType)            // sub type (for sub objects)
{
    udt_Object *objectValue;
    udt_Buffer buffer;
    sword status;

    // None is treated as null
    if (pythonValue == Py_None) {
        *ociValueIndicator = OCI_IND_NULL;

    // all other values need to be converted
    } else {

        *ociValueIndicator = OCI_IND_NOTNULL;
        switch (typeCode) {
            case OCI_TYPECODE_CHAR:
            case OCI_TYPECODE_VARCHAR:
            case OCI_TYPECODE_VARCHAR2:
                if (cxBuffer_FromObject(&buffer, pythonValue,
                        environment->encoding) < 0)
                    return -1;
                status = OCIStringAssignText(environment->handle,
                        environment->errorHandle, buffer.ptr,
                        buffer.size, &oracleValue->stringValue);
                cxBuffer_Clear(&buffer);
                if (Environment_CheckForError(environment, status,
                        "Object_ConvertFromPython(): assigning string") < 0)
                    return -1;
                *ociValue = oracleValue->stringValue;
                break;
            case OCI_TYPECODE_INTEGER:
                oracleValue->integerValue = PyInt_AsLong(pythonValue);
                if (PyErr_Occurred())
                    return -1;
                *ociValue = &oracleValue->integerValue;
                break;
            case OCI_TYPECODE_NUMBER:
                if (PythonNumberToOracleNumber(environment,
                        pythonValue, &oracleValue->numberValue) < 0)
                    return -1;
                *ociValue = &oracleValue->numberValue;
                break;
            case OCI_TYPECODE_DATE:
                if (PythonDateToOracleDate(pythonValue,
                        &oracleValue->dateValue) < 0)
                    return -1;
                *ociValue = &oracleValue->dateValue;
                break;
            case OCI_TYPECODE_TIMESTAMP:
                status = OCIDescriptorAlloc(environment->handle,
                        (dvoid**) &oracleValue->timestampValue,
                        OCI_DTYPE_TIMESTAMP, 0, 0);
                if (Environment_CheckForError(environment, status,
                        "Object_ConvertFromPython(): "
                        "create timestamp descriptor") < 0)
                    return -1;
                if (PythonDateToOracleTimestamp(environment,
                        pythonValue, oracleValue->timestampValue) < 0)
                    return -1;
                *ociValue = oracleValue->timestampValue;
                break;
            case OCI_TYPECODE_NAMEDCOLLECTION:
            case OCI_TYPECODE_OBJECT:
                if (Py_TYPE(pythonValue) != &g_ObjectType) {
                    PyErr_SetString(PyExc_TypeError,
                            "expecting cx_Oracle.Object");
                    return -1;
                }
                objectValue = (udt_Object*) pythonValue;
                if (objectValue->objectType->tdo != subType->tdo) {
                    PyErr_SetString(PyExc_TypeError,
                            "expecting an object of the correct type");
                    return -1;
                }
                *ociValue = objectValue->instance;
                *ociObjectIndicator = objectValue->indicator;
                break;
#if ORACLE_VERSION_HEX >= ORACLE_VERSION(12, 1)
            case OCI_TYPECODE_BOOLEAN:
                if (PythonBooleanToOracleBoolean(pythonValue,
                        &oracleValue->booleanValue) < 0)
                    return -1;
                *ociValue = &oracleValue->booleanValue;
                break;
#endif
            default:
                PyErr_Format(g_NotSupportedErrorException,
                        "Object_ConvertFromPython(): unhandled data type %d",
                        typeCode);
                return -1;
        };

    }

    return 0;
}


//-----------------------------------------------------------------------------
// Object_ConvertToPython()
//   Convert an Oracle value to a Python value.
//-----------------------------------------------------------------------------
static PyObject *Object_ConvertToPython(
    udt_Environment *environment,       // environment to use
    OCITypeCode typeCode,               // type of Oracle data
    dvoid *value,                       // Oracle value
    dvoid *indicator,                   // null indicator
    udt_ObjectType *subType)            // sub type (for sub objects)
{
    text *stringValue;
    ub4 stringSize;

    // null values returned as None
    if (* (OCIInd*) indicator == OCI_IND_NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    switch (typeCode) {
        case OCI_TYPECODE_CHAR:
        case OCI_TYPECODE_VARCHAR:
        case OCI_TYPECODE_VARCHAR2:
            stringValue = OCIStringPtr(environment->handle,
                    * (OCIString**) value);
            stringSize = OCIStringSize(environment->handle,
                    * (OCIString**) value);
            return cxString_FromEncodedString( (char*) stringValue,
                    stringSize, environment->encoding);
        case OCI_TYPECODE_INTEGER:
            return PyInt_FromLong(* (int*) value);
        case OCI_TYPECODE_NUMBER:
            return OracleNumberToPythonFloat(environment, (OCINumber*) value);
        case OCI_TYPECODE_DATE:
            return OracleDateToPythonDate(&vt_DateTime, (OCIDate*) value);
        case OCI_TYPECODE_TIMESTAMP:
            return OracleTimestampToPythonDate(environment,
                    * (OCIDateTime**) value);
        case OCI_TYPECODE_OBJECT:
            return Object_New(subType, value, indicator, 0);
        case OCI_TYPECODE_NAMEDCOLLECTION:
            return Object_New(subType, * (OCIColl**) value, indicator, 0);
#if ORACLE_VERSION_HEX >= ORACLE_VERSION(12, 1)
        case OCI_TYPECODE_BOOLEAN:
            return OracleBooleanToPythonBoolean((boolean*) value);
#endif
    };

    return PyErr_Format(g_NotSupportedErrorException,
            "Object_ConvertToPython(): unhandled data type %d", typeCode);
}


//-----------------------------------------------------------------------------
// Object_GetAttributeValue()
//   Retrieve an attribute on the object.
//-----------------------------------------------------------------------------
static PyObject *Object_GetAttributeValue(
    udt_Object *self,                   // object
    udt_ObjectAttribute *attribute)     // attribute to get
{
    dvoid *valueIndicator, *value;
    OCIInd scalarValueIndicator;
    udt_Connection *connection;
    udt_Buffer buffer;
    sword status;
    OCIType *tdo;

    // get the value for the attribute
    connection = self->objectType->connection;
    if (cxBuffer_FromObject(&buffer, attribute->name,
            connection->environment->encoding) < 0)
        return NULL;
    status = OCIObjectGetAttr(connection->environment->handle,
            connection->environment->errorHandle, self->instance,
            self->indicator, self->objectType->tdo,
            (const OraText**) &buffer.ptr, (ub4*) &buffer.size, 1, 0, 0,
            &scalarValueIndicator, &valueIndicator, &value, &tdo);
    cxBuffer_Clear(&buffer);
    if (Environment_CheckForError(connection->environment, status,
            "Object_GetAttributeValue(): getting value") < 0)
        return NULL;

    // determine the proper null indicator
    if (!valueIndicator)
        valueIndicator = &scalarValueIndicator;

    return Object_ConvertToPython(connection->environment,
            attribute->typeCode, value, valueIndicator, attribute->subType);
}


//-----------------------------------------------------------------------------
// Object_SetAttributeValue()
//   Set an attribute on the object.
//-----------------------------------------------------------------------------
static int Object_SetAttributeValue(
    udt_Object *self,                   // object
    udt_ObjectAttribute *attribute,     // attribute to set
    PyObject *value)                    // value to set
{
    dvoid *ociObjectIndicator, *ociValue;
    udt_AttributeData attributeData;
    OCIInd ociValueIndicator;
    udt_Connection *connection;
    udt_Buffer buffer;
    sword status;

    // convert from Python
    ociValue = ociObjectIndicator = NULL;
    connection = self->objectType->connection;
    AttributeData_Initialize(&attributeData, attribute->typeCode);
    if (Object_ConvertFromPython(connection->environment, value,
            attribute->typeCode, &attributeData, &ociValue, &ociValueIndicator,
            &ociObjectIndicator, attribute->subType) < 0) {
        AttributeData_Free(connection->environment, &attributeData,
                attribute->typeCode);
        return -1;
    }

    // set the value for the attribute
    if (cxBuffer_FromObject(&buffer, attribute->name,
            connection->environment->encoding) < 0) {
        AttributeData_Free(connection->environment, &attributeData,
                attribute->typeCode);
        return -1;
    }
    status = OCIObjectSetAttr(connection->environment->handle,
            connection->environment->errorHandle, self->instance,
            self->indicator, self->objectType->tdo,
            (const OraText**) &buffer.ptr, (ub4*) &buffer.size, 1, 0, 0,
            ociValueIndicator, ociObjectIndicator, ociValue);
    AttributeData_Free(connection->environment, &attributeData,
            attribute->typeCode);
    cxBuffer_Clear(&buffer);
    if (Environment_CheckForError(connection->environment, status,
            "Object_SetAttributeValue(): setting value") < 0)
        return -1;

    return 0;
}


//-----------------------------------------------------------------------------
// Object_GetAttr()
//   Retrieve an attribute on an object.
//-----------------------------------------------------------------------------
static PyObject *Object_GetAttr(
    udt_Object *self,                   // object
    PyObject *nameObject)               // name of attribute
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
static int Object_SetAttr(
    udt_Object *self,                   // object
    PyObject *nameObject,               // name of attribute
    PyObject *value)                    // value to set
{
    udt_ObjectAttribute *attribute;

    attribute = (udt_ObjectAttribute*)
            PyDict_GetItem(self->objectType->attributesByName, nameObject);
    if (attribute)
        return Object_SetAttributeValue(self, attribute, value);

    return PyObject_GenericSetAttr( (PyObject*) self, nameObject, value);
}


//-----------------------------------------------------------------------------
// Object_CheckIsCollection()
//   Check if the object is a collection, and if not, raise an exception. This
// is used by the collection methods below.
//-----------------------------------------------------------------------------
static int Object_CheckIsCollection(
    udt_Object *self)                   // object
{
    if (!self->objectType->isCollection) {
        PyErr_SetString(PyExc_TypeError, "object is not a collection");
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Object_PopulateList()
//   Convert the collection elements to Python values.
//-----------------------------------------------------------------------------
static int Object_PopulateList(
    udt_Object *self,                   // collection iterating
    OCIIter *iter,                      // iterator
    PyObject *list)                     // list result
{
    dvoid *elementValue, *elementIndicator;
    udt_Environment *environment;
    PyObject *elementObject;
    boolean endOfCollection;
    sword status;

    environment = self->objectType->connection->environment;
    while (list) {
        status = OCIIterNext(environment->handle, environment->errorHandle,
                iter, &elementValue, &elementIndicator, &endOfCollection);
        if (Environment_CheckForError(environment, status,
                "Object_PopulateList(): get next") < 0)
            return -1;
        if (endOfCollection)
            break;
        elementObject = Object_ConvertToPython(environment,
                self->objectType->elementTypeCode, elementValue,
                elementIndicator,
                (udt_ObjectType*) self->objectType->elementType);
        if (!elementObject)
            return -1;
        if (PyList_Append(list, elementObject) < 0) {
            Py_DECREF(elementObject);
            return -1;
        }
        Py_DECREF(elementObject);
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Object_InternalAppend()
//   Append an item to the collection.
//-----------------------------------------------------------------------------
static int Object_InternalAppend(
    udt_Object *self,                   // object
    PyObject *value)                    // value to append
{
    void *elementValue, *elementIndicator;
    udt_AttributeData attributeData;
    udt_Environment *environment;
    OCIInd tempIndicator;
    sword status;

    // convert Python value to OCI value
    elementValue = elementIndicator = NULL;
    environment = self->objectType->connection->environment;
    AttributeData_Initialize(&attributeData,
            self->objectType->elementTypeCode);
    if (Object_ConvertFromPython(environment, value,
            self->objectType->elementTypeCode, &attributeData, &elementValue,
            &tempIndicator, &elementIndicator,
            (udt_ObjectType*) self->objectType->elementType) < 0) {
        AttributeData_Free(environment, &attributeData,
                self->objectType->elementTypeCode);
        return -1;
    }
    if (!elementIndicator)
        elementIndicator = &tempIndicator;

    // append converted value to collection
    status = OCICollAppend(environment->handle, environment->errorHandle,
            elementValue, elementIndicator, (OCIColl*) self->instance);
    if (Environment_CheckForError(environment, status,
            "Object_Append()") < 0) {
        AttributeData_Free(environment, &attributeData,
                self->objectType->elementTypeCode);
        return -1;
    }
    AttributeData_Free(environment, &attributeData,
            self->objectType->elementTypeCode);

    return 0;
}


//-----------------------------------------------------------------------------
// Object_InternalExtend()
//   Extend the collection by appending each of the items in the sequence.
//-----------------------------------------------------------------------------
static int Object_InternalExtend(
    udt_Object *self,                   // object
    PyObject *sequence)                 // sequence to extend collection with
{
    PyObject *fastSequence, *element;
    Py_ssize_t size, i;

    // make sure we are dealing with a collection
    if (Object_CheckIsCollection(self) < 0)
        return -1;

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
static PyObject *Object_Append(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    PyObject *value;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
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
static PyObject *Object_AsList(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments (none)
{
    udt_Environment *environment;
    PyObject *list;
    OCIIter *iter;
    sword status;

    // make sure this is a collection
    if (Object_CheckIsCollection(self) < 0)
        return NULL;

    // create the iterator
    environment = self->objectType->connection->environment;
    status = OCIIterCreate(environment->handle, environment->errorHandle,
            self->instance, &iter);
    if (Environment_CheckForError(environment, status,
            "Object_AsList(): creating iterator") < 0)
        return NULL;

    // create the result list
    list = PyList_New(0);
    if (list) {
        if (Object_PopulateList(self, iter, list) < 0) {
            Py_DECREF(list);
            list = NULL;
        }
    }
    OCIIterDelete(environment->handle, environment->errorHandle, &iter);

    return list;
}


//-----------------------------------------------------------------------------
// Object_Copy()
//   Return a copy of the object.
//-----------------------------------------------------------------------------
static PyObject *Object_Copy(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments (none)
{
    udt_Environment *environment;
    udt_Object *copiedObject;
    sword status;

    copiedObject = Object_Create(self->objectType);
    if (!copiedObject)
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCIObjectCopy(environment->handle, environment->errorHandle,
            self->objectType->connection->handle, self->instance,
            self->indicator, copiedObject->instance, copiedObject->indicator,
            self->objectType->tdo, OCI_DURATION_SESSION, OCI_DEFAULT);
    if (Environment_CheckForError(environment, status, "Object_Copy()") < 0) {
        Py_DECREF(copiedObject);
        return NULL;
    }

    return (PyObject*) copiedObject;
}


//-----------------------------------------------------------------------------
// Object_Delete()
//   Delete the element at the specified index in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_Delete(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    udt_Environment *environment;
    sword status;
    sb4 index;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCITableDelete(environment->handle, environment->errorHandle,
            index, self->instance);
    if (Environment_CheckForError(environment, status, "Object_Delete()") < 0)
        return NULL;
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_Exists()
//   Return true or false indicating if an element exists in the collection at
// the specified index.
//-----------------------------------------------------------------------------
static PyObject *Object_Exists(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    udt_Environment *environment;
    boolean exists;
    sword status;
    sb4 index;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCITableExists(environment->handle, environment->errorHandle,
            self->instance, index, &exists);
    if (Environment_CheckForError(environment, status, "Object_Exists()") < 0)
        return NULL;
    if (exists)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}


//-----------------------------------------------------------------------------
// Object_Extend()
//   Extend the collection by appending each of the items in the sequence.
//-----------------------------------------------------------------------------
static PyObject *Object_Extend(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
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
static PyObject *Object_GetElement(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    void *elementValue, *elementIndicator;
    udt_Environment *environment;
    boolean exists;
    sb4 position;
    sword status;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &position))
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCICollGetElem(environment->handle, environment->errorHandle,
            (OCIColl*) self->instance, (sb4) position, &exists,
            &elementValue, &elementIndicator);
    if (Environment_CheckForError(environment, status,
            "Object_GetItem(): get element") < 0)
        return NULL;
    if (!exists) {
        PyErr_SetString(PyExc_IndexError, "element does not exist");
        return NULL;
    }
    return Object_ConvertToPython(environment,
            self->objectType->elementTypeCode, elementValue, elementIndicator,
            (udt_ObjectType*) self->objectType->elementType);
}


//-----------------------------------------------------------------------------
// Object_GetFirstIndex()
//   Return the index of the first entry in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_GetFirstIndex(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments (none)
{
    udt_Environment *environment;
    sword status;
    sb4 index;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCITableFirst(environment->handle, environment->errorHandle,
            self->instance, &index);
    if (Environment_CheckForError(environment, status,
            "Object_GetFirstIndex()") < 0)
        return NULL;
    return PyInt_FromLong(index);
}


//-----------------------------------------------------------------------------
// Object_GetLastIndex()
//   Return the index of the last entry in the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_GetLastIndex(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments (none)
{
    udt_Environment *environment;
    sword status;
    sb4 index;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCITableLast(environment->handle, environment->errorHandle,
            self->instance, &index);
    if (Environment_CheckForError(environment, status,
            "Object_GetLastIndex()") < 0)
        return NULL;
    return PyInt_FromLong(index);
}


//-----------------------------------------------------------------------------
// Object_GetNextIndex()
//   Return the index of the next entry in the collection following the index
// specified. If there is no next entry, None is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_GetNextIndex(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    udt_Environment *environment;
    sb4 index, nextIndex;
    boolean exists;
    sword status;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCITableNext(environment->handle, environment->errorHandle,
            index, self->instance, &nextIndex, &exists);
    if (Environment_CheckForError(environment, status,
            "Object_GetNextIndex()") < 0)
        return NULL;
    if (exists)
        return PyInt_FromLong(nextIndex);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetPrevIndex()
//   Return the index of the previous entry in the collection preceding the
// index specified. If there is no previous entry, None is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_GetPrevIndex(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    udt_Environment *environment;
    sb4 index, prevIndex;
    boolean exists;
    sword status;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCITablePrev(environment->handle, environment->errorHandle,
            index, self->instance, &prevIndex, &exists);
    if (Environment_CheckForError(environment, status,
            "Object_GetPrevIndex()") < 0)
        return NULL;
    if (exists)
        return PyInt_FromLong(prevIndex);
    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_GetSize()
//   Return the size of a collection. If the object is not a collection, an
// error is returned.
//-----------------------------------------------------------------------------
static PyObject *Object_GetSize(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments (none)
{
    udt_Environment *environment;
    sword status;
    sb4 size;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCICollSize(environment->handle, environment->errorHandle,
            (const OCIColl*) self->instance, &size);
    if (Environment_CheckForError(environment, status, "Object_Size()") < 0)
        return NULL;
    return PyInt_FromLong(size);
}


//-----------------------------------------------------------------------------
// Object_SetElement()
//   Set the element at the specified location to the given value.
//-----------------------------------------------------------------------------
static PyObject *Object_SetElement(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    void *elementValue, *elementIndicator;
    udt_AttributeData attributeData;
    udt_Environment *environment;
    OCIInd tempIndicator;
    PyObject *value;
    sb4 position;
    sword status;

    // make sure we are dealing with a collection
    if (Object_CheckIsCollection(self) < 0)
        return NULL;

    // parse arguments
    if (!PyArg_ParseTuple(args, "iO", &position, &value))
        return NULL;

    // convert to OCI value
    elementValue = elementIndicator = NULL;
    environment = self->objectType->connection->environment;
    AttributeData_Initialize(&attributeData,
            self->objectType->elementTypeCode);
    if (Object_ConvertFromPython(environment, value,
            self->objectType->elementTypeCode, &attributeData, &elementValue,
            &tempIndicator, &elementIndicator,
            (udt_ObjectType*) self->objectType->elementType) < 0) {
        AttributeData_Free(environment, &attributeData,
                self->objectType->elementTypeCode);
        return NULL;
    }
    if (!elementIndicator)
        elementIndicator = &tempIndicator;
    status = OCICollAssignElem(environment->handle, environment->errorHandle,
            position, elementValue, elementIndicator,
            (OCIColl*) self->instance);
    if (Environment_CheckForError(environment, status,
            "Object_SetItem(): assign element") < 0) {
        AttributeData_Free(environment, &attributeData,
                self->objectType->elementTypeCode);
        return NULL;
    }
    AttributeData_Free(environment, &attributeData,
            self->objectType->elementTypeCode);

    Py_RETURN_NONE;
}


//-----------------------------------------------------------------------------
// Object_Trim()
//   Trim a number of elements from the end of the collection.
//-----------------------------------------------------------------------------
static PyObject *Object_Trim(
    udt_Object *self,                   // object
    PyObject *args)                     // arguments
{
    udt_Environment *environment;
    sb4 numToTrim;
    sword status;

    if (Object_CheckIsCollection(self) < 0)
        return NULL;
    if (!PyArg_ParseTuple(args, "i", &numToTrim))
        return NULL;
    environment = self->objectType->connection->environment;
    status = OCICollTrim(environment->handle, environment->errorHandle,
            numToTrim, self->instance);
    if (Environment_CheckForError(environment, status, "Object_Trim()") < 0)
        return NULL;
    Py_RETURN_NONE;
}

