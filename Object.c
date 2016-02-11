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
// Declaration of external object variable functions.
//-----------------------------------------------------------------------------
static void Object_Free(udt_Object*);
static PyObject *Object_GetAttr(udt_Object*, PyObject*);
static int Object_SetAttr(udt_Object*, PyObject*, PyObject*);
static PyObject *Object_ConvertToPython(udt_Environment*, OCITypeCode, dvoid*,
        dvoid*, udt_ObjectType*);


//-----------------------------------------------------------------------------
// Declaration of external object variable members.
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
    0,                                  // tp_methods
    g_ObjectMembers                     // tp_members
};


//-----------------------------------------------------------------------------
// Declaration of attribute data union
//-----------------------------------------------------------------------------
typedef union {
    OCINumber numberValue;
    OCIDate dateValue;
    OCIDateTime *timestampValue;
    OCIString *stringValue;
} udt_AttributeData;


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
// Object_ConvertCollectionElements()
//   Convert the collection elements to Python values.
//-----------------------------------------------------------------------------
static int Object_ConvertCollectionElements(
    udt_Environment *environment,       // environment to use
    OCIIter *iter,                      // iterator
    PyObject *list,                     // list result
    udt_ObjectType *objectType)         // collection type information
{
    dvoid *elementValue, *elementIndicator;
    PyObject *elementObject;
    boolean endOfCollection;
    sword status;

    while (list) {
        status = OCIIterNext(environment->handle, environment->errorHandle,
                iter, &elementValue, &elementIndicator, &endOfCollection);
        if (Environment_CheckForError(environment, status,
                "Object_ConvertCollection(): get next") < 0)
            return -1;
        if (endOfCollection)
            break;
        elementObject = Object_ConvertToPython(environment,
                objectType->elementTypeCode, elementValue, elementIndicator,
                (udt_ObjectType*) objectType->elementType);
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
// Object_ConvertCollection()
//   Convert a collection to a Python list.
//-----------------------------------------------------------------------------
static PyObject *Object_ConvertCollection(
    udt_Environment *environment,       // environment to use
    OCIColl *collectionValue,           // collection value
    udt_ObjectType *objectType)         // collection type information
{
    PyObject *list;
    OCIIter *iter;
    sword status;
    int result;

    // create the iterator
    status = OCIIterCreate(environment->handle, environment->errorHandle,
            collectionValue, &iter);
    if (Environment_CheckForError(environment, status,
            "Object_ConvertCollection(): creating iterator") < 0)
        return NULL;

    // create the result list
    list = PyList_New(0);
    if (list) {
        result = Object_ConvertCollectionElements(environment, iter,
                list, objectType);
        if (result < 0) {
            Py_DECREF(list);
            list = NULL;
        }
    }
    OCIIterDelete(environment->handle, environment->errorHandle, &iter);

    return list;
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
                oracleValue->stringValue = NULL;
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
                oracleValue->timestampValue = NULL;
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
            return Object_ConvertCollection(environment,
                    * (OCIColl**) value, subType);
    };

    return PyErr_Format(g_NotSupportedErrorException,
            "Object_GetAttributeValue(): unhandled data type %d",
            typeCode);
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

